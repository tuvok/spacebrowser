#include <PasswordManager.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <iostream>

PasswordManager::PasswordManager()
{
    QFile file(":/js/formExtractor.js");
    file.open(QIODevice::ReadOnly);
    formExtractor = file.readAll();

    QFile file2(":/qtwebchannel/qwebchannel.js");
    file2.open(QIODevice::ReadOnly);
    qWebChannel= file2.readAll();

    QFile file3(":/js/formFiller.js");
    file3.open(QIODevice::ReadOnly);
    formFiller = file3.readAll();
}

PasswordManager::~PasswordManager()
{

}

void PasswordManager::fillPassword(QVariant view)
{
    QObject* v = qvariant_cast<QObject *>(view);

    bool s = QMetaObject::invokeMethod(v, "runJavaScript",
            Q_ARG(QString, formFiller));

}

void PasswordManager::keySelected(QString id)
{
    QStringList parts = id.split(' ');

    if (parts.size() < 1)
        std::cout << "PasswordManager::keySelected(id='" << id.toStdString() <<
            "'): invalid id format";

    keys.addKey(parts[0].toStdString(), true);
}

void PasswordManager::loadSucceeded(QVariant view)
{
    QObject* v = qvariant_cast<QObject *>(view);

    QVariantMap objects;
    objects.insert("pwManager", QVariant(QMetaType::QObjectStar, this));


    QObject* wc = qvariant_cast<QObject *>(v->property("webChannel"));

    QMetaObject::invokeMethod(wc, "registerObject",
            Q_ARG(QString, "pwManager"), Q_ARG(QObject*, this));

    bool s = QMetaObject::invokeMethod(v, "runJavaScript",
            Q_ARG(QString, formExtractor));

    QUrl url = v->property("url").toUrl();
    QString host = url.host();
    QString path = url.path();

    if (int passCount = pwds.countSavedCredentials(host, path) > 0)
    {
        QMetaObject::invokeMethod(v, "passAvailable",
                Q_ARG(QVariant, QVariant(passCount)));
    }

    s = QMetaObject::invokeMethod(v, "runJavaScript",
            Q_ARG(QString, qWebChannel));
}

void PasswordManager::saveAccepted(QString url, bool accepted)
{
    if (!accepted)
    {
        if (tempStore.count(url))
            tempStore.erase(url);
        return;
    }

    if (tempStore.count(url) == 0)
    {
        std::cout << "ERROR: password not found in tempStore\n";
        return;
    }

    const auto& entry = tempStore.at(url);

    switch (pwds.isSaved(entry))
    {
    case db::Passwords::SaveState::Outdated:
    case db::Passwords::SaveState::Absent:
    {
        pwds.saveOrUpdate(entry);
    } break;
    default:
    {
        // nothing to be done
    }
    }
}

bool PasswordManager::savePassword(QVariant fields_qv)
{
    QJsonArray f = fields_qv.toJsonArray();

    db::Passwords::entry_t fields;

    for (const auto& field: f)
    {
        if (!field.isObject())
            continue;

        const auto& obj = field.toObject();
        if (obj["type"] == "text" || obj["type"] == "email")
            fields.login = obj["value"].toString();
        else if (obj["type"] == "password" && !obj["value"].toString().isEmpty())
        {
            fields.password = encrypt(obj["value"].toString());
            fields.fingerprint = keys.getDefaultKey().c_str();
        } else if (obj["type"] == "host")
            fields.host = obj["value"].toString();
        else if (obj["type"] == "path")
            fields.path = obj["value"].toString();
    }

    if (fields.login.isEmpty() || fields.password.isEmpty() || fields.host.isEmpty())
    {
        std::cout << "PasswordManager::savePassword(): one of the fields was empty\n";
        std::cout << "PasswordManager::savePassword(): login: " << fields.login.toStdString() << std::endl;
        std::cout << "PasswordManager::savePassword(): pass_len: " << fields.password.length() << std::endl;
        std::cout << "PasswordManager::savePassword(): url: " << fields.host.toStdString()
                << fields.path.toStdString() << std::endl;

        return false;
    }

    tempStore[fields.host + fields.path] = fields;

    switch (pwds.isSaved(fields))
    {
    case db::Passwords::SaveState::Outdated:
    {
        emit shouldBeUpdated(fields.host + fields.path, fields.login);
    } break;
    case db::Passwords::SaveState::Absent:
    {
        emit shouldBeSaved(fields.host + fields.path, fields.login);
    } break;
    default:
    {
        // nothing to be done
    }
    }

    return true;
}

QString PasswordManager::encrypt(QString text)
{
    try
    {
        gnupgpp::GpgContext ctx = gpg.createContext();
        ctx.setArmor(true);

        std::string keyFp = keys.getDefaultKey();
        if (keyFp.empty())
            return QString();

        std::shared_ptr<gnupgpp::GpgKey> key = ctx.getKey(keyFp);

        return QString(ctx.encrypt(text.toStdString(), key).c_str());

    }
    catch (std::exception& e)
    {
        std::cout << "PasswordManager::encrypt(): caught exception: " << e.what()
                  << std::endl;
    }

    return QString();
}

bool PasswordManager::isEncryptionReady()
{
    std::string keyFp = keys.getDefaultKey();
    if (keyFp.empty())
        return false;

    gnupgpp::GpgContext ctx = gpg.createContext();
    std::shared_ptr<gnupgpp::GpgKey> key = ctx.getKey(keyFp);
    if (!key)
        return false;

    std::shared_ptr<gnupgpp::GpgKey> sKey = ctx.getKey(keyFp, true);
    if (!sKey)
        return false;

    return true;
}

QStringList PasswordManager::keysList() const
{
    QStringList result;

    auto ctx = gpg.createContext();

    auto keys = ctx.listSecretKeys();

    for (auto& key: keys)
    {
        if (key.isRevoked()  || key.isExpired() ||
            key.isDisabled() || key.isInvalid())
            continue;

        QString entry;
        entry += QString(key.getFingerprintOfPK().c_str());

        for (auto& uid: key.getUids())
            entry += " " + QString(uid.getUid().c_str());


        for (auto& sk: key.getSubkeys())
        {
            if (sk.isRevoked()  || sk.isExpired() ||
                sk.isDisabled() || sk.isInvalid())
                continue;

            auto cno = sk.getCardNumber();
            if (!cno.empty())
            {
                entry += " Card No:" + QString(cno.c_str());
                break;
            }
        }
        result.push_back(entry);
    }

    return result;
}

QVariant PasswordManager::getCredentials(QVariant host, QVariant path) noexcept
{
    if (!host.isValid())
        return QVariant();

    QString h = host.toString();
    QString p;

    path.isValid() ? p = path.toString() : p = "";

    auto creds = pwds.getCredentials(h, p);

    switch (creds.size())
    {
    case 0:
    {
        std::cout << "PasswordManager::getCredentials(host='" << h.toStdString()
                  << "' path='" << p.toStdString() << "'): no credentials found\n";
        return QJsonObject();
    }
    case 1:
    {
        auto ctx = gpg.createContext();
        try
        {
            QString pass = ctx.decrypt(creds[0].password.toStdString()).c_str();
            QJsonObject c { {"login", creds[0].login}, {"pass", pass} };
            return c;

        }
        catch (std::exception& e)
        {
            std::cout << "PasswordManager::getCredentials(): failed: " <<
                e.what() << std::endl;
        }
    } break;
    default:
    {
        // FIXME: display choice menu
    }
    }

    return QJsonObject();
}
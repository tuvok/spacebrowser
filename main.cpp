#include <ContentFilter.h>
#include <ViewHandler.h>
#include <PrintHandler.h>
#include <PasswordManager.h>
#include <TabView.h>
#include <db/Backend.h>
#include <db/DbGroup.h>
#include <conf/conf.h>

#include <QApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QtWebEngine>
#include <QQuickWebEngineProfile>
#include <QShortcut>
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTextEdit>

#include <memory>

Q_DECLARE_LOGGING_CATEGORY(mainLogs)
Q_LOGGING_CATEGORY(mainLogs, "main")

void writeSettings(std::shared_ptr<QQuickView> view)
{
    QSettings settings;

    settings.setValue(conf::MainWindow::geometry, view->geometry());
}

void readSettings(std::shared_ptr<QQuickView> view)
{
    QSettings settings;

    if (settings.contains(conf::MainWindow::geometry))
        view->setGeometry(settings.value(conf::MainWindow::geometry).toRect());
}

int main(int argc, char *argv[])
{
    /// Register qml types
    qmlRegisterType<TabView>("spacebrowser.TabView", 1,0, "TabView");

    /// Initialize logging
    ///
    QString colorsStart = "%{if-debug}\033[34m%{endif}%{if-warning}\033[33m%{endif}"
        "%{if-critical}\033[31m%{endif}";
    qSetMessagePattern(colorsStart +
                       "[%{category}]\033[0m %{function}[:%{line}]: \033[32m%{message}\033[0m");

    /// Create and setup QApplication
    ///
    QtWebEngine::initialize();

    QApplication app(argc, argv);
    app.setOrganizationName("SpaceFoundation");
    app.setApplicationName("SpaceBrowser");

    //FIXME: test this
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    /// Create and setup QQuickView with MainWindow
    ///
    std::shared_ptr<QQuickView> view(new QQuickView);
    view->setSource(QUrl("qrc:/ui/MainWindow.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->show();
    readSettings(view);

    /// Setup printing handler
    ///
    PrintHandler ph;
    QObject::connect(view->rootObject(), SIGNAL(printRequest(QVariant)),
            &ph, SLOT(printRequested(QVariant)));

    /// Initialize databases
    ///
    QSettings settings;
    unsigned int dbCount = settings.beginReadArray(conf::Databases::dbArray);
    settings.endArray();

    QObject* confDbConnDialog = view->rootObject()->
        findChild<QObject*>("configureDbConnectionDialog");
    if (!confDbConnDialog)
        throw std::runtime_error("No configureDbConnectionDialog object found");

    db::Backend dbBackend;
    dbBackend.setObjectName("dbBackend");
    QObject::connect(confDbConnDialog, SIGNAL(dbConfigured(QVariant)),
                     &dbBackend, SLOT(dbConfigured(QVariant)));

    // FIXME: call connectDatabases async and configure if connect failed
    if (!dbCount)
    {
        qCCritical(mainLogs) <<"no db configured or connectDatabases() failed";

        dbBackend.configureDbConnection(confDbConnDialog, /*passMan.isEncryptionReady()*/ false);
    }
    dbBackend.connectDatabases();

    /// Setup password manager
    ///
    PasswordManager passMan;
    //passMan.setGrp(&dbGrp);

    QObject::connect(view->rootObject(), SIGNAL(loadSucceeded(QVariant)),
                     &passMan, SLOT(loadSucceeded(QVariant)));

    QObject* encKeyConfDialog = view->rootObject()->
        findChild<QObject*>("encryptionKeyConfigDialog");
    if (!encKeyConfDialog)
        throw std::runtime_error("No encryptionKeyConfigDialog object found");

    QObject::connect(encKeyConfDialog, SIGNAL(keySelected(QString, QString)),
                     &passMan, SLOT(keySelected(QString, QString)));

    QQuickItem* passwordManagerButton = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("passwordManagerButton"));
    if (!passwordManagerButton)
        throw std::runtime_error("No passwordManagerButton object found");

    QObject::connect(passwordManagerButton, SIGNAL(passwordFillRequest(QVariant)),
                     &passMan, SLOT(fillPassword(QVariant)));
    QObject::connect(passwordManagerButton, SIGNAL(passwordFillRequestDBN(QVariant, QVariant)),
                     &passMan, SLOT(fillPassword(QVariant, QVariant)));


/* FIXME: this has to be run after db gets connected
    if (!passMan.isEncryptionReady())
    {
        QStringList model = passMan.keysList();
        QMetaObject::invokeMethod(view->rootObject(), "configureEncryption",
                                  Qt::ConnectionType::QueuedConnection,
                                  Q_ARG(QVariant, model));
    }

*/

    QObject::connect(&passMan, SIGNAL(shouldBeSaved(QVariant, QVariant, QVariant)),
                     view->rootObject(), SLOT(shouldBeSaved(QVariant, QVariant, QVariant)));
    QObject::connect(&passMan, SIGNAL(shouldBeUpdated(QVariant, QVariant, QVariant)),
                     view->rootObject(), SLOT(shouldBeUpdated(QVariant, QVariant, QVariant)));
    QObject::connect(view->rootObject(), SIGNAL(savePasswordAccepted(QString, QString, bool)),
                     &passMan, SLOT(saveAccepted(QString, QString, bool)));


    /// Setup View Handler
    ///
    QQuickItem* scriptBlockingView = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("scriptBlockingView"));
    if (!scriptBlockingView)
        throw std::runtime_error("No scriptBlockingView object found");

    ViewHandler vh(view, dbBackend);
    vh.setObjectName("ViewHandler");
    QObject::connect(&dbBackend, SIGNAL(dbReady(QString, QString)),
                     &vh, SLOT(dbReady(QString, QString)), Qt::QueuedConnection);

    QObject* tabSelectorPanel = view->rootObject()->
        findChild<QObject*>("tabSelectorPanel");
    if (!tabSelectorPanel)
        throw std::runtime_error("No tabSelectorPanel object found");

    QObject::connect(view->rootObject(), SIGNAL(showFullscreen(bool)),
                     &vh, SLOT(showFullscreen(bool)));

    QObject::connect(tabSelectorPanel, SIGNAL(openScriptBlockingView(QString, int)),
                     &vh, SLOT(openScriptBlockingView(QString, int)));


    /// Setup signals to update PasswordManager status icon
    QObject::connect(&vh, &ViewHandler::panelSelected,
                     &passMan, &PasswordManager::checkIfEncryptionReady);
    QObject::connect(&passMan, SIGNAL(encryptionReady(QVariant, QVariant)),
                     view->rootObject(), SLOT(encryptionStatus(QVariant, QVariant)));


    /// Load tabs, set-up signals
    ///
    vh.init();

    /// Setup Downloader
    ///

//    setupProfileDownloadHandler(bd, profile);
//    setupDownloaderSignals(bd, view, profile);


//    qCDebug(mainLogs, "------- vh");
//    vh.dumpObjectInfo();
//    qCDebug(mainLogs, "------- dbBackend");
//    dbBackend.dumpObjectInfo();
//    qCDebug(mainLogs, "-------");

    qCDebug(mainLogs) << "Finished init, executing app";
    int status = app.exec();

    writeSettings(view);

    return status;
}

#include <db/ScriptBlock.h>

#include <db/Sql.h>

namespace db
{

namespace sql
{
QString insert("INSERT INTO \"%1\".\"%2\" (%3) VALUES (%4) %5;");
QString del("DELETE FROM \"%1\".\"%2\" %3;");
}

std::string ScriptBlock::globalTableName("globalScriptWhitelist");
std::string ScriptBlock::siteTableName("siteList");
std::string ScriptBlock::localTableName("scriptWhitelist");

ScriptBlock::ScriptBlock()
{
    // FIXME: in one place,
    // FIXME: connections pooled in one distributor
    sql::Helpers::createSchemaIfNotExists(conn, sql::schemaName);

    sql::Helpers::createTableIfNotExists(conn, sql::schemaName, globalTableName.c_str(),
            "url varchar PRIMARY KEY");

    sql::Helpers::createTableIfNotExists(conn, sql::schemaName, siteTableName.c_str(),
            "url varchar PRIMARY KEY");

    sql::Helpers::createTableIfNotExists(conn, sql::schemaName, localTableName.c_str(),
            ("id serial PRIMARY KEY, \"siteUrl\" varchar REFERENCES \"" +
                    sql::schemaName.toStdString() + "\".\"" + siteTableName +
                    "\", url varchar UNIQUE").c_str());

    sql::Helpers::createIndex(conn, sql::schemaName, localTableName.c_str(), "siteUrl");
}

ScriptBlock::~ScriptBlock()
{

}

ScriptBlock::State ScriptBlock::isAllowed(const std::string& site,
        const std::string& url, bool earlyReturn)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::exists.arg(sql::schemaName).arg(globalTableName.c_str())
            .arg(("WHERE url='" + ntx.esc(url) + "'").c_str()).toStdString());

    if (r.size() != 1)
    {
        throw std::runtime_error(
                "ScriptBlock::isAllowed(): PSQL returned " +
                std::to_string(r.size()) + " rows");
    }

    bool allowedGlobally = r[0][0].as<bool>();

    if (earlyReturn && allowedGlobally)
        return State::AllowedGlobally;

    r = ntx.exec(sql::exists.arg(sql::schemaName).arg(localTableName.c_str())
            .arg(("WHERE \"siteUrl\"='" + ntx.esc(site)
                    + "' AND url='" + ntx.esc(url) + "'").c_str()).toStdString());

    if (r.size() != 1)
    {
        throw std::runtime_error(
                "ScriptBlock::isAllowed(): PSQL returned " +
                std::to_string(r.size()) + " rows");
    }

    bool allowedLocally = r[0][0].as<bool>();

    if (allowedGlobally && allowedLocally)
        return State::AllowedBoth;
    else if (allowedGlobally)
        return State::AllowedGlobally;
    else if (allowedLocally)
        return State::Allowed;

    return State::Blocked;
}

void ScriptBlock::whitelistLocal(const std::string& site, const std::string& url)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::insert.arg(sql::schemaName).arg(siteTableName.c_str())
            .arg("url").arg(("'" + ntx.esc(site) + "'").c_str())
            .arg("ON CONFLICT (url) DO NOTHING").toStdString());

    r = ntx.exec(sql::insert.arg(sql::schemaName).arg(localTableName.c_str())
            .arg("\"siteUrl\", url")
            .arg(("'" + ntx.esc(site) + "', '" + ntx.esc(url) + "'").c_str())
            .arg("ON CONFLICT (url) DO NOTHING").toStdString());
}

void ScriptBlock::whitelistGlobal(const std::string& url)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::insert.arg(sql::schemaName).arg(globalTableName.c_str())
            .arg("url").arg(("'" + ntx.esc(url) + "'").c_str())
            .arg("ON CONFLICT (url) DO NOTHING").toStdString());
}

void ScriptBlock::removeLocal(const std::string& site, const std::string& url)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::del.arg(sql::schemaName).arg(localTableName.c_str())
            .arg(("WHERE \"siteUrl\"='" + ntx.esc(site)
                    + "' AND url ='" + ntx.esc(url) +"'").c_str()).toStdString());
}

void ScriptBlock::removeGlobal(const std::string& url)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::del.arg(sql::schemaName).arg(globalTableName.c_str())
            .arg(("WHERE url='" + ntx.esc(url) + "'").c_str()).toStdString());
}

} /* namespace db */



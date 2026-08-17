#include "SqliteBatchWriter.h"

#include <chrono>
#include <stdexcept>
#include <utility>

SqliteBatchWriter::SqliteBatchWriter(std::string databasePath, std::size_t batchSize)
    : databasePath_(std::move(databasePath))
    , batchSize_(batchSize == 0 ? 1 : batchSize)
{
    open();
    initializeSchema();
}

SqliteBatchWriter::~SqliteBatchWriter()
{
    try 
    {
        flush();
    } 
    catch (...) 
    {
    }

    if (database_ != nullptr) {
        sqlite3_close(database_);
    }
}

void SqliteBatchWriter::add(const Message& message)
{
    pendingMessages_.push_back(message);

    if (pendingMessages_.size() >= batchSize_) {
        flush();
    }
}

void SqliteBatchWriter::flush()
{
    if (pendingMessages_.empty()) 
    {
        return;
    }

    execute("BEGIN TRANSACTION;");

    sqlite3_stmt* statement = nullptr;
    const char* insertSql =
        "INSERT OR IGNORE INTO messages (id, header, payload, received_at) VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(database_, insertSql, -1, &statement, nullptr) != SQLITE_OK) 
    {
        execute("ROLLBACK;");
        throw std::runtime_error(sqlite3_errmsg(database_));
    }

    const auto finalizeStatement = [&statement]() 
        {
        if (statement != nullptr) 
        {
            sqlite3_finalize(statement);
            statement = nullptr;
        }
    };

    try 
    {
        for (const Message& message : pendingMessages_) 
        {
            const auto now = std::chrono::system_clock::now();
            const auto receivedAt = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

            sqlite3_bind_text(statement, 1, message.getId().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(statement, 2, message.getHeader().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(statement, 3, message.getPayload().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(statement, 4, static_cast<sqlite3_int64>(receivedAt));

            if (sqlite3_step(statement) != SQLITE_DONE) 
            {
                throw std::runtime_error(sqlite3_errmsg(database_));
            }

            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);
        }

        finalizeStatement();
        execute("COMMIT;");
        pendingMessages_.clear();
    } 
    catch (...) 
    {
        finalizeStatement();
        execute("ROLLBACK;");
        throw;
    }
}

void SqliteBatchWriter::open()
{
    if (sqlite3_open(databasePath_.c_str(), &database_) != SQLITE_OK) 
    {
        const std::string error = database_ != nullptr ? sqlite3_errmsg(database_) : "cannot open SQLite database";
        throw std::runtime_error(error);
    }
}

void SqliteBatchWriter::initializeSchema()
{
    execute(
        "CREATE TABLE IF NOT EXISTS messages ("
        "id TEXT PRIMARY KEY,"
        "header TEXT NOT NULL,"
        "payload TEXT NOT NULL,"
        "received_at INTEGER NOT NULL"
        ");");
}

void SqliteBatchWriter::execute(const char* sql)
{
    char* errorMessage = nullptr;
    if (sqlite3_exec(database_, sql, nullptr, nullptr, &errorMessage) != SQLITE_OK) 
    {
        std::string error = errorMessage != nullptr ? errorMessage : "SQLite execution failed";
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }
}


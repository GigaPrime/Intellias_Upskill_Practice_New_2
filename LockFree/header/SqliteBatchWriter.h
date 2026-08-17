#pragma once

#include "Message.h"

#include <cstddef>
#include <sqlite3.h>
#include <string>
#include <vector>

class SqliteBatchWriter 
{
private:
    void open();
    void initializeSchema();
    void execute(const char* sql);

    std::string databasePath_;
    std::size_t batchSize_{};
    sqlite3* database_{ nullptr };
    std::vector<Message> pendingMessages_;

public:
    explicit SqliteBatchWriter(std::string databasePath, std::size_t batchSize);
    ~SqliteBatchWriter();

    SqliteBatchWriter(const SqliteBatchWriter&) = delete;
    SqliteBatchWriter& operator=(const SqliteBatchWriter&) = delete;

    void add(const Message& message);
    void flush();
};


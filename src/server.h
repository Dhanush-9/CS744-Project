#pragma once
#include "cache.h"
#include "db.h"
#include "httplib.h"
#include <string>
#include <memory>

using namespace std;

class KVServer {
private:
    unique_ptr<httplib::Server> srv;
    LRU cache;
    DB db;

    // Metrics
    atomic<long long> cacheHits{0};
    atomic<long long> cacheMisses{0};
    atomic<long long> dbReads{0};
    atomic<long long> dbWrites{0};

public:
    KVServer(const string &dbHost, const string &dbUser, const string &dbPass, const string &dbName, int cacheSize = 500);
    void start(const string &bind = "0.0.0.0", int port = 8080);
};

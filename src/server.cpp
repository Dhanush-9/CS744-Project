#include "server.h"
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace httplib;

KVServer::KVServer(const string &dbHost,const string &dbUser,const string &dbPass,const string &dbName, int cacheSize)
    : srv(make_unique<Server>()),
      cache(cacheSize),
      db(dbHost, dbUser, dbPass, dbName)
{
    int threads = thread::hardware_concurrency();
    if (threads == 0) threads = 12;
    srv->new_task_queue = [threads] {
        return new ThreadPool(threads);
    };

    srv->Post("/create", [this](const Request &req, Response &res) {
        if (!req.has_param("key") || !req.has_param("value")) {
            res.status = 400;
            res.set_content("Missing key or value\n", "text/plain");
            return;
        }
        int key;
        try{
            key = stoi(req.get_param_value("key"));
        }
        catch(...){
            res.status = 400;
            res.set_content("Invalid key\n", "text/plain");
            return;
        }
        
        const string value = req.get_param_value("value");

        bool cacheStatus = cache.put(key, value);
        bool dbStatus = db.put(key, value);
        if (!cacheStatus || !dbStatus) {
            res.status = 500;
            res.set_content("Failed to store key " + to_string(key) + "\n", "text/plain");
            return;
        }
        dbWrites++;

        res.status = 200;
        res.set_content("Created key " + to_string(key) + "\n", "text/plain");
    });

    srv->Get("/read", [this](const Request &req, Response &res) {
        if (!req.has_param("key")) {
            res.status = 400;
            res.set_content("Missing key\n", "text/plain");
            return;
        }

        int key;
        try{
            key = stoi(req.get_param_value("key"));
        }
        catch(...){
            res.status = 400;
            res.set_content("Invalid key\n", "text/plain");
            return;
        }
        string value;

        if (cache.get(key, value)) {
            cacheHits++;
            res.set_content("Cache HIT: " + value + "\n", "text/plain");
            return;
        }

        if (!db.get(key, value)) {
            res.status = 404;
            res.set_content("Key not found\n", "text/plain");
            return;
        }
        cacheMisses++;
        dbReads++;
        cache.put(key, value);
        res.status = 200;
        res.set_content("Cache MISS -> DB: " + value + "\n", "text/plain");
    });

    srv->Delete("/delete", [this](const Request &req, Response &res) {
        if (!req.has_param("key")) {
            res.status = 400;
            res.set_content("Missing key\n", "text/plain");
            return;
        }

        int key;
        try{
            key = stoi(req.get_param_value("key"));
        }
        catch(...){
            res.status = 400;
            res.set_content("Invalid key\n", "text/plain");
            return;
        }

        bool dbStatus = db.del(key);
        cache.del(key); 
        dbWrites++;

        if (!dbStatus) {
            res.status = 404;
            res.set_content("Key not found\n", "text/plain");
            return;
        }
        res.status = 200;
        res.set_content("Deleted key " + to_string(key) + "\n", "text/plain");
    });

    srv->Get("/metrics", [this](const Request &req, Response &res) {
        string metrics;
        metrics += "cache_hits :" + to_string(cacheHits.load()) + "\n";
        metrics += "cache_misses :" + to_string(cacheMisses.load()) + "\n";
        metrics += "db_reads :" + to_string(dbReads.load()) + "\n";
        metrics += "db_writes :" + to_string(dbWrites.load()) + "\n";

        res.status = 200;
        res.set_content(metrics, "text/plain");
    });
}

void KVServer::start(const string &bind, int port) {
    cout << "KVServer running on http://localhost:"<<port<<endl;
    srv->listen("0.0.0.0", port);
}

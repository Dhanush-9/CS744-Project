#include "db.h"
#include <iostream>
#include <mysql/mysql.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <stdexcept>

using namespace std;

class ConnectionPool {
private:
    queue<MYSQL*> pool;
    mutex mtx;
    condition_variable cv;
    bool initialized = false;

public:
    static ConnectionPool& instance() {
        static ConnectionPool inst;
        return inst;
    }

    void init(const string &host, const string &user, const string &pass, const string &db, int poolSize) {
        lock_guard<mutex> lock(mtx);
        if (initialized) return;

        for (int i = 0; i < poolSize; ++i) {
            MYSQL *conn = mysql_init(nullptr);
            if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), 0, nullptr, 0)) {
                throw runtime_error(string("MySQL connect failed: ") + mysql_error(conn));
                if(conn) mysql_close(conn);
            }
            mysql_autocommit(conn, 1);
            pool.push(conn);
        }
        initialized = true;
    }

    MYSQL* acquire() {
        unique_lock<mutex> lock(mtx);
        while (pool.empty()) {
            cv.wait(lock);
        }
        MYSQL* conn = pool.front();
        pool.pop();
        return conn;
    }

    void release(MYSQL* conn) {
        lock_guard<mutex> lock(mtx);
        pool.push(conn);
        cv.notify_one();
    }

    void shutdown() {
        lock_guard<mutex> lock(mtx);
        while (!pool.empty()) {
            mysql_close(pool.front());
            pool.pop();
        }
        initialized = false;
    }
};


DB::DB(const string &host, const string &user, const string &pass, const string &dbname, int poolSize) {
    ConnectionPool::instance().init(host, user, pass, dbname, poolSize);
}

DB::~DB() {
    ConnectionPool::instance().shutdown();
}

bool DB::get(int key, string &value) {
    MYSQL* conn = ConnectionPool::instance().acquire();

    string q = "SELECT v FROM kvtable WHERE k= " + to_string(key);
    if (mysql_query(conn, q.c_str())) {
        cerr << "MySQL get error: " << mysql_error(conn) << endl;
        ConnectionPool::instance().release(conn);
        return false;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        ConnectionPool::instance().release(conn);
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    bool ok = false;
    if (row && row[0]) {
        value = row[0];
        ok = true;
    }

    mysql_free_result(res);
    ConnectionPool::instance().release(conn);
    return ok;
}

bool DB::put(int key, const string &value) {
    MYSQL* conn = ConnectionPool::instance().acquire();

    string q = "REPLACE INTO kvtable (k,v) VALUES (" + to_string(key) + ", '" + value + "')";
    bool ok = (mysql_query(conn, q.c_str()) == 0);
    if (!ok) cerr << "MySQL put error: " << mysql_error(conn) << endl;

    ConnectionPool::instance().release(conn);
    return ok;
}

bool DB::del(int key) {
    MYSQL* conn = ConnectionPool::instance().acquire();

    string q = "DELETE FROM kvtable WHERE k = " + to_string(key);
    bool ok = (mysql_query(conn, q.c_str()) == 0);
    bool found = false;
    if (ok) {
        if (mysql_affected_rows(conn) > 0) {
            found = true;
        }
    }
    else{
        cerr << "MySQL delete error: " << mysql_error(conn) << endl;
    }
    ConnectionPool::instance().release(conn);
    return found;
}

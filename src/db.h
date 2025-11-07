#pragma once
#include <string>

using namespace std;

class DB {
public:
    DB(const string &host, const string &user, const string &pass, const string &dbname, int poolSize = 10);
    ~DB();

    bool get(int key, string &value);
    bool put(int key, const string &value);
    bool del(int key);
};
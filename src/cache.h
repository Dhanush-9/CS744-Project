#pragma once
#include <unordered_map>
#include <list>
#include <string>
#include <shared_mutex>
#include <mutex>

using namespace std;
class LRU {
private:
    int capacity;
    list<pair<int, string>> cacheList;
    unordered_map<int, list<pair<int, string>>::iterator> cacheMap;
    long long hit = 0;
    long long miss = 0;
    mutable shared_mutex mtx;

public:
    explicit LRU(int cap);
    bool get(int key, string &value);
    bool put(int key, const string &value);
    bool del(int key);
    void display() const;
};

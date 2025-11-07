#include "cache.h"
#include <iostream>

using namespace std;

LRU :: LRU (int cap) : capacity(cap) {}

bool LRU :: get(int key, string &value){
    lock_guard<mutex> lock(mtx);

    auto it = cacheMap.find(key);
    if(it == cacheMap.end()){
        //miss++;
        return false;
    }

    value = it->second->second;
    cacheList.splice(cacheList.begin(), cacheList, it->second);
    //hit++;
    return true;
}

bool LRU :: put(int key, const string &value){
    lock_guard<mutex> lock(mtx);
    try{
        auto it = cacheMap.find(key);

        //updating existing key
        if(it != cacheMap.end()){
            it->second->second = value;
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            return true;
        }

        //evict if needed
        if(cacheList.size() >= capacity){
            auto last = cacheList.back();
            cacheMap.erase(last.first);
            cacheList.pop_back();
        }

        //insert new key
        cacheList.emplace_front(key, value);
        cacheMap[key] = cacheList.begin();
        return true;
    } 
    catch (...) {
        return false;
    }
}

bool LRU :: del(int key){
    lock_guard<mutex> lock(mtx);
    //key not found
    auto it = cacheMap.find(key);
    if(it == cacheMap.end()){
        return false;
    }
    cacheList.erase(it->second);
    cacheMap.erase(key);
    return true;
}


void LRU :: display() const {
    lock_guard<mutex> lock(mtx);
    cout << "Cache contents: ";
    for(auto &p : cacheList){
        cout << "(" << p.first << ":" << p.second << ") ";
    }
    cout << endl;
}


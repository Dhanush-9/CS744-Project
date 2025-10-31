#include <iostream>
#include <unordered_map>
#include <list>

using namespace std;

class LRU{
private:
    int cap;
    list <pair<int, string>> lru_list;
    unordered_map<int, list<pair<int, string>>::iterator> lru_map;
    long long hit = 0;
    long long miss = 0;

public:
    LRU(int cap){
        this->cap = cap;
    }

    long long miss_ratio(){
        long long total = hit + miss;
        if(total == 0){
            return 0;
        }
        else{
            return miss/total;
        }
    }

    long long hit_ratio(){
        long long total = hit + miss;
        if(total == 0){
            return 0;
        }
        else{
            return hit/total;
        }
    }

    string get(int key){
        if(lru_map.find(key) == lru_map.end()){
            cout << "Cache MISS for key: " << key << endl;
            miss++;
            return "";
        }

        hit++;
        auto it = lru_map[key];
        string value = it->second;
        lru_list.erase(it);
        lru_list.push_front({key, value});
        lru_map[key] = lru_list.begin();

        cout << "Cache HIT for key: " << key << endl;
        return value;
    }

    void put(int key, string value){

        //key already exists
        if(lru_map.find(key) != lru_map.end()){
            lru_list.erase(lru_map[key]);
        }

        //cache full, replace using LRU policy
        else if(lru_list.size() >= cap){
            auto last = lru_list.back();
            lru_map.erase(last.first);
            lru_list.pop_back();
        }

        //insert new key-value pair
        lru_list.push_front({key, value});
        lru_map[key] = lru_list.begin();
    }

    bool delete_key(int key){
        //key not found
        if(lru_map.find(key) == lru_map.end()){
            return false;
        }

        auto it = lru_map[key];
        lru_list.erase(it);
        lru_map.erase(key);
        return true;
    }

    void display() {
        cout << "Cache: ";
        for (auto &p : lru_list) {
            cout << "(" << p.first << ":" << p.second << ") ";
        }
        cout << endl;
    }
};

int main(){
    LRU cache(3);

    cache.put(1, "A");
    cache.put(2, "B");
    cache.put(3, "C");
    cache.display();

    cache.get(2);
    cache.display();

    cache.put(4, "D");
    cache.display();

    cache.delete_key(3);
    cache.display();

    cache.put(5, "E");
    cache.display();

    return 0;
}
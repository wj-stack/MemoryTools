//
// Created by wyatt on 2022/2/26.
//

#ifndef MEMORYSTORE_MEMORYMANAGER_H
#define MEMORYSTORE_MEMORYMANAGER_H

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <utility>
#include "FileManager.h"

#include "FileManager.h"

typedef long Address;

class MemoryManager {
public:
    struct Flag {
        Address address;
        uint count; // 扫描次数
        bool operator<(const MemoryManager::Flag &tmp) const {
            if (this->address == tmp.address) {
                return this->count < tmp.count;
            }
            return this->address < tmp.address;
        }
    };

    map<Flag, FileManager::ptr> addr; // <address,起始地址>
    map<Flag, FileManager::ptr> data; // <address,数据>
public:
    void creat(Address address, uint count);

    void del(Address address, uint count);

    template<class T>
    void append(Address start_address, uint count, Address address, T val);

    void append(Address start_address, uint count, char *val, size_t size);

    template<class T>
    std::shared_ptr<T> readAllData(Address start_address, uint count, size_t &size);

    std::shared_ptr<Address> readAllAddress(Address start_address, uint count, size_t &size);

};

template<class T>
void MemoryManager::append(Address start_address, uint count, Address address, T val) {
    addr[{start_address, count}]->append(address);
    data[{start_address, count}]->append(val);
}


template<class T>
shared_ptr<T> MemoryManager::readAllData(Address start_address, uint count, size_t &size) {
    return data[{start_address, count}]->template readAll<T>(size);
}


#endif //MEMORYSTORE_MEMORYMANAGER_H

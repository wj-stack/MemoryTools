//2
// Created by wyatt on 2022/2/26.
//

#include "MemoryManager.h"


void MemoryManager::creat(Address address, uint count) {
    addr[{address, count}] = std::make_shared<FileManager>(
            "./tmp/address_" + to_string(address) + "_" + to_string(count), "wb+");
    data[{address, count}] = std::make_shared<FileManager>("./tmp/data_" + to_string(address) + "_" + to_string(count),
                                                           "wb+");
}

void MemoryManager::del(Address address, uint count) {
    addr.erase({address, count});
    data.erase({address, count});
}


void MemoryManager::append(Address start_address, uint count, char *val, size_t size) {
    addr[{start_address, count}]->append(start_address);
    data[{start_address, count}]->append(val, size);
}


std::shared_ptr<Address> MemoryManager::readAllAddress(Address start_address, uint count, size_t &size) {
    return addr[{start_address, count}]->template readAll<Address>(size);
}

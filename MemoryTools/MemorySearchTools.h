//
// Created by wyatt on 2022/2/24.
//

#ifndef MEMORYSTORE_MEMORYSEARCHTOOLS_H
#define MEMORYSTORE_MEMORYSEARCHTOOLS_H

#include <cstddef>
#include <vector>
#include <memory>
#include <cstring>
#include <iostream>
#include <functional>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <syscall.h>
#include <vector>
#include "MemoryManager.h"
#include "../ThreadPool/ThreadPool.h"

//#ifdef WIN32
//#include <windows.h>
//#include <tlhelp32.h>
//#endif


struct MemPage {
    long start;
    long end;
    char flags[8];
    char name[128];
};

ostream &operator<<(ostream &os, const MemPage &page) {
    os << hex << page.start << " " << page.end << " " << page.flags << " " << page.name << endl;
    return os;
}


class MemoryTools {
private:
    int pid_;
    MemoryManager manager;

    // 这是两个辅助函数
    template<class T>
    void Third(Address start, uint count, std::atomic<size_t> &allSize, const std::function<bool(T &, T &)> &fun,
               std::atomic<size_t> &acc, T *value = nullptr);
    template<class T>
    void Second(Address start, size_t count, std::atomic<size_t> &allsize, const std::function<bool(T &, T &)> &fun,
                std::atomic<size_t> &acc, T *value = nullptr /* 默认value */);


public:
    //查找游戏进程pid
    static int getPID(const char *pack_name);

    static std::vector<pair<std::string,int>> getAllPID();

    explicit MemoryTools(int pid) : pid_(pid) {}

    static size_t pwritev(int pid, long address, void *buffer, size_t size);

    static size_t preadv(int pid, long address, void *buffer, size_t size);

    static int getMemPage(int pid, std::vector<MemPage> &res);

    template<class T>
    size_t FirstSearch(vector<MemPage> &pages, bool isRead = true, bool isWrite = false, bool isExecute = false);



    // 从数据块 到数据小块小块到
    template<class T>
    size_t SecondSearch(const std::function<bool(T &, T &)> &fun, T *value = nullptr);
    template<class T>
    size_t ThirdSearch(const std::function<bool(T &, T &)> &fun, uint lastCount, T *value = nullptr);




    template<class T>
    int dump(uint count, function<void(const Address &, const T &)> showFunc, bool showNewValue = false);


    template<class T>
    static bool isEqual(T &a, T &b) ;

    template<class T>
    static bool isNotEqual(T &a, T &b);

    template<class T>
    static bool isBigger(T &a, T &b);

    template<class T>
    static bool isSmaller(T &a, T &b);

};

template<class T>
size_t MemoryTools::FirstSearch(vector<MemPage> &pages, bool isRead, bool isWrite, bool isExecute) {

    size_t allSize = 0;
    int count = 1; // 设置第一次扫描
    for (auto &page: pages) {
        if (isRead && page.flags[0] != 'r')continue;
        if (isWrite && page.flags[1] != 'w')continue;
        if (isExecute && page.flags[2] != 'x')continue;
        manager.creat(page.start, count); // 创建
        long size = page.end - page.start;
        allSize += size / sizeof(T);
        void *buff = malloc(size);
        int t = preadv(pid_, page.start, buff, size);
        if (t > 0) {
            manager.append(page.start, count, (char *) buff, size); // 把所有数据都写入
        }
        free(buff);
    }
    return allSize;
}

template<class T>
size_t MemoryTools::SecondSearch(const function<bool(T &, T &)> &fun, T *value) {
    std::atomic<size_t> acc{0};
    std::atomic<size_t> allsize{0};
    threadpool threadpool;
//    timespec tim;
//    clock_gettime(CLOCK_REALTIME, &tim);
//    std::cout << "time:" << tim.tv_nsec << std::endl;
    int acc2 = 0;
    for (auto &mp: manager.addr) {
        if (mp.first.count == 1) {
            acc2++;
            threadpool.commit([&]() {
                Second<T>(mp.first.address, mp.first.count, allsize, fun, acc, value);
            });
        }

    }
    while (acc2 != acc);
    cout << allsize << endl;
    return allsize;
}

template<class T>
size_t MemoryTools::ThirdSearch(const function<bool(T &, T &)> &fun, uint lastCount, T *value) {
    std::atomic<size_t> allSize{0};
    threadpool threadpool;
    std::atomic<size_t> acc{0};
    int acc2 = 0;
    for (auto &mp: manager.addr) {
        if (mp.first.count == lastCount) {
            acc2++;
            threadpool.commit([&]() {
                Third<T>(mp.first.address, mp.first.count, allSize, fun, acc,value);
            });
        }

    }
    while (acc != acc2);
    return allSize;
}

template<class T>
int MemoryTools::dump(uint count, function<void(const Address &, const T &)> showFunc, bool showNewValue){
    size_t allSize = 0;
    for (auto &mp: manager.addr) {
        if (mp.first.count == count) {
            // 上一次扫描
            Address start = mp.first.address;
            uint count = mp.first.count;
            size_t size;

            auto ptr = manager.readAllData<T>(start, count, size);
            T *oldValue = ptr.get(); // 上次搜索到的值

            auto ptr2 = manager.readAllAddress(start, count, size);
            Address *oldAddress = ptr2.get(); // 上次搜索到的地址

            allSize += size;

            for (int i = 0; i < size; ++i) {
                if (showNewValue)
                    preadv(pid_, oldAddress[i], &oldValue[i], sizeof(T));
                showFunc(oldAddress[i], oldValue[i]);

            }

        }

    }
    return allSize;
}

template<class T>
bool MemoryTools::isEqual(T &a, T &b) {
    return a == b;
}

template<class T>
bool MemoryTools::isNotEqual(T &a, T &b) {
    return a != b;
}

template<class T>
bool MemoryTools::isBigger(T &a, T &b){
    return a <= b;
}

template<class T>
bool MemoryTools::isSmaller(T &a, T &b)  {
    return a >= b;
}

template<class T>
void MemoryTools::Second(Address start, size_t count, atomic<size_t> &allsize, const function<bool(T &, T &)> &fun,
                         atomic<size_t> &acc, T *value) {
    const int typeSize = sizeof(T);
    // 有默认value,就是说 和默认value比较
    size_t size;
    auto ptr = manager.readAllData<T>(start, count, size);
    T *oldValue = ptr.get(); // 第一次搜到的数据
    manager.creat(start, count + 1); // 下一次扫描
    void *buff = malloc(size * sizeof(T)); // 新的数据
    int t = preadv(pid_, start, buff, size * sizeof(T));
    if (t > 0) {
        T *newValue = (T *) buff;
        for (int i = 0; i < size; ++i) {
            if (value == nullptr) {
                if (fun(oldValue[i], newValue[i])) {
                    manager.append(start, count + 1, start + i * typeSize, newValue[i]);
                    allsize++;
                }
            } else {
                if (fun(newValue[i], *value)) {
                    manager.append(start, count + 1, start + i * typeSize, newValue[i]);
                    allsize++;
                }
            }
        }
        free(newValue);
    }
    acc++;
}

template<class T>
void MemoryTools::Third(Address start, uint count, atomic<size_t> &allSize, const function<bool(T &, T &)> &fun,
                        atomic<size_t> &acc, T *value){
    T buff; // 移到最外

    {
        // 上一次扫描
        size_t size;

        auto ptr = manager.readAllData<T>(start, count, size);
        T *oldValue = ptr.get(); // 上次搜索到的值

        auto ptr2 = manager.readAllAddress(start, count, size);
        Address *oldAddress = ptr2.get(); // 上次搜索到的地址

        manager.creat(start, count + 1); // 下一次扫描
        for (int i = 0; i < size; ++i) {
            preadv(pid_, oldAddress[i], &buff, sizeof(T));

            if (value == nullptr) {
                if (fun(oldValue[i], buff)) {
                    manager.append(start, count + 1, oldAddress[i], buff);
                    allSize++;
                }
            } else {
                if (fun(buff, *value)) {
                    manager.append(start, count + 1, oldAddress[i], buff);
                    allSize++;
                }
            }
        }
    }
    acc++;
}



#endif //MEMORYSTORE_MEMORYSEARCHTOOLS_H

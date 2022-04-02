//
// Created by wyatt on 2022/2/26.
//

#ifndef MEMORYSTORE_FILEMANAGER_H
#define MEMORYSTORE_FILEMANAGER_H
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <utility>

using namespace std;

class FileManager {
private:
    string fileName;
    FILE *fd;
public:
    using ptr = std::shared_ptr<FileManager>;
    using wptr = std::weak_ptr<FileManager>;
    friend class MemoryManager;

    FileManager(string fileName1, const char *mode);

    template<class T>
    void append(const T &data);

    void append(const char *data, size_t size);

    template<class T>
    void edit(size_t x, const T &data);
    int getLen() ;

    template<class T>
    void read(size_t x,T& data);

    void read(size_t x,size_t count,char* data);

    template<class T>
    std::shared_ptr<T> readAll(size_t &size);


    ~FileManager();

};

template<class T>
shared_ptr<T> FileManager::readAll(size_t &size)  {
    fseek(fd, 0, SEEK_END);
    size_t len = ftell(fd);
    size = len / sizeof(T); // 使用引用返回个数
    fseek(fd, 0, SEEK_SET);
    auto ptr = std::shared_ptr<T>(new T[size]);
    fread(ptr.get(), sizeof(T), size, fd);
    return ptr;
}

template<class T>
void FileManager::read(size_t x, T &data) {
    fseek(fd, x * sizeof(T), SEEK_SET);
    fread(&data,sizeof(T),1,fd);
}

template<class T>
void FileManager::edit(size_t x, const T &data) {
    fseek(fd, x * sizeof(T), SEEK_SET); // 移动到第x个数据并修改
    fwrite(&data, sizeof(T), 1, fd);
}

template<class T>
void FileManager::append(const T &data)  {
    fseek(fd, 0, SEEK_END); // 移动到末尾添加数据
    fwrite(&data, sizeof(T), 1, fd);
}


#endif //MEMORYSTORE_FILEMANAGER_H

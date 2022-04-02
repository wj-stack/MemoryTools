//
// Created by wyatt on 2022/2/26.
//

#include "FileManager.h"

FileManager::FileManager(string fileName1, const char *mode):fileName(std::move(fileName1)){
    remove(fileName.c_str()); // 移除
    fd = fopen(fileName.c_str(), mode);
}

void FileManager::append(const char *data, size_t size) {
    fseek(fd, 0, SEEK_END);  // 移动到末尾添加数据
    size_t t = 0;
    while (t < size)
    {
        t += fwrite(data + t, 1, size - t, fd);
//            cout << "写入" << t << endl;
    }
//        cout << "预计" << size << " 成功写入:" << t << endl;
}


int FileManager::getLen() {
    fseek(fd, 0, SEEK_END);
    return ftell(fd);
}


void FileManager::read(size_t x, size_t count, char *data){
    fseek(fd, x , SEEK_SET);
    fread(data,1,count,fd);
}

FileManager::~FileManager()  {
    fclose(fd);
    remove(fileName.c_str()); // 移除
}

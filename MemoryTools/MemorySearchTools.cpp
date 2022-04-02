//
// Created by wyatt on 2022/2/24.
//

#include "MemorySearchTools.h"

int MemoryTools::getPID(const char *pack_name) {
    int id = -1, pid = -1;
    DIR *dir = 0;
    FILE *file = 0;
    char filename[32] = {0};
    char cmdline[256] = {0};
    struct dirent *entry = 0;

    if (pack_name == NULL) {
        return -1;
    }

    dir = opendir("/proc");
    if (dir == NULL) {
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id > 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            file = fopen(filename, "r");

            if (file) {
                fgets(cmdline, sizeof(cmdline), file);
                fclose(file);
//                    std::cout << cmdline << std::endl;
                if (strcmp(pack_name, cmdline) == 0) {
                    pid = id;
                    break;
                }
            }
        }
    }
    closedir(dir);
    return pid;
}

vector<pair<std::string,int>> MemoryTools::getAllPID() {
    int id = -1, pid = -1;
    DIR *dir = 0;
    FILE *file = 0;
    char filename[32] = {0};
    char cmdline[256] = {0};
    struct dirent *entry = 0;

    dir = opendir("/proc");
    if (dir == nullptr) {
        return {};
    }
    std::vector<pair<std::string,int>> res;
    while ((entry = readdir(dir)) != nullptr) {
        id = atoi(entry->d_name);
        if (id > 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            file = fopen(filename, "r");

            if (file) {
                fgets(cmdline, sizeof(cmdline), file);
                fclose(file);
                res.emplace_back(cmdline, id);
            }
        }
    }
    closedir(dir);
    return res;
}


size_t MemoryTools::pwritev(int pid, long address, void *buffer, size_t size) {
    struct iovec iov_WriteBuffer, iov_WriteOffset;
    iov_WriteBuffer.iov_base = buffer;
    iov_WriteBuffer.iov_len = size;
    iov_WriteOffset.iov_base = (void *) address;
    iov_WriteOffset.iov_len = size;
    return syscall(SYS_process_vm_writev, pid, &iov_WriteBuffer, 1, &iov_WriteOffset, 1, 0);
}

size_t MemoryTools::preadv(int pid, long address, void *buffer, size_t size) {
    struct iovec iov_ReadBuffer, iov_ReadOffset;
    iov_ReadBuffer.iov_base = buffer;
    iov_ReadBuffer.iov_len = size;
    iov_ReadOffset.iov_base = (void *) address;
    iov_ReadOffset.iov_len = size;
    return syscall(SYS_process_vm_readv, pid, &iov_ReadBuffer, 1, &iov_ReadOffset, 1, 0);
}

int MemoryTools::getMemPage(int pid, vector<MemPage> &res) {
    char mempath[64] = {0};
    sprintf(mempath, "/proc/%d/maps", pid);
    auto fd = fopen(mempath, "r");
    if(fd == 0)
    {
        return 0;
    }
    char tmp[256] = {0};
    long start, end;
    while (fgets(tmp, sizeof(tmp), fd)) {
        MemPage mp;
        sscanf(tmp, "%p-%p %s %*p %*p:%*p %*p   %[^\n]%s", &mp.start, &mp.end,
               mp.flags, mp.name);
        res.push_back(mp);
    }
    return res.size();
}
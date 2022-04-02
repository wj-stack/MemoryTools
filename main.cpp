#include <iostream>
#include "MemoryTools/MemorySearchTools.h"
#include "MemoryTools/FileManager.h"
using namespace std;

int a = 1000;

int pid = getpid();
MemoryTools memoryTools(getpid());


template<class T>
void show(const Address & address,const T& newValue)
{
    cout << address << " " << newValue << endl;
}

int main() {
    vector<MemPage> pages;
    MemoryTools::getMemPage(pid, pages);
    for(auto & page:pages)cout << page << endl;
    size_t size = memoryTools.FirstSearch<int>(pages);
    cout << size << endl;
    size_t size1 = memoryTools.SecondSearch<int>(&MemoryTools::isEqual<int>, &a);
    a = 102913;
    memoryTools.dump<int>(2, &show<int>, true);
    cout << &a << " " << a << endl;


    return 0;
}

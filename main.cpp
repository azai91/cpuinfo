//
// Created by Zai, Alexander on 5/17/18.
//

#include <cpuinfo.h>
#include <iostream>

int main() {
    cpuinfo_initialize();
    uint32_t num = cpuinfo_get_processors_count();
    std::cout << num;
    return 0;

}
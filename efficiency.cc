#include <iostream>
#include <cstdint>
#include <string>
#include <windows.h>
#include <fstream>
#include <random>
#include <ctime>

#include "kvstore.h"

enum TestType {
    PUT, GET, DEL
};


void test(uint64_t size, TestType type, std::ofstream &out);

int main(int argc, char *argv[])
{
    std::ofstream out("throughput.csv");

//    test(1 << 9, PUT, out);
//    test(1 << 11, PUT, out);
//    test((1 << 14) - 1, PUT, out);
//    test(1 << 9, GET, out);
//    test(1 << 11, GET, out);
    test(1024 * 64, PUT, out);
//    test(1 << 9, DEL, out);
//    test(1 << 11, DEL, out);
//    test((1 << 14) - 1, DEL, out);

    out.close();
    return 0;
}

void test(uint64_t size, TestType type, std::ofstream &out)
{
    static int n = 0;
    ++n;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint64_t> dis(0, RAND_MAX);
    LARGE_INTEGER cpuFreq;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    double runTime;
    KVStore store("./data" + std::to_string(n));
    std::vector<uint64_t> ranKeys;

    switch (type) {

        case PUT: {
            double totalTime = 0.0;
            for (uint64_t i = 0; i < size; ++i) {
                uint64_t ranKey = dis(generator);
                QueryPerformanceFrequency(&cpuFreq);
                QueryPerformanceCounter(&startTime);
                store.put(ranKey, std::string(1024 * 16, 'p'));
                QueryPerformanceCounter(&endTime);
                double throughPut = (double)cpuFreq.QuadPart / (double)((endTime.QuadPart - startTime.QuadPart));
                out << totalTime << "," << throughPut << std::endl;
                totalTime += (double)((endTime.QuadPart - startTime.QuadPart)) / (double)cpuFreq.QuadPart;
            }
            break;
        }


        case GET: {
            for (uint64_t i = 0; i < size; ++i) {
                uint64_t ranKey = dis(generator);
                ranKeys.push_back(ranKey);
                store.put(ranKey, std::string(2048, 'g'));
            }
            double total_time = 0.0;
            for (uint64_t i = 0; i < size; ++i) {
                // 有概率是store中不存在的key
                uint64_t ranKey = dis(generator) % 20 < 19 ? ranKeys[dis(generator) % size] : dis(generator);
                QueryPerformanceFrequency(&cpuFreq);
                QueryPerformanceCounter(&startTime);
                store.get(ranKey);
                QueryPerformanceCounter(&endTime);
                runTime = (double) ((endTime.QuadPart - startTime.QuadPart) * 1000000) / (double) cpuFreq.QuadPart;
                total_time += runTime;
            }
            printf("%f\n", total_time / (double) size);
            break;
        }
        case DEL:
            for (uint64_t i = 0; i < size; ++i) {
                uint64_t ranKey = dis(generator);
                ranKeys.push_back(ranKey);
                store.put(ranKey, std::string(2048, 'd'));
            }

            for (uint64_t i = 0; i < size; ++i) {
                // 有概率是store中不存在的key
                uint64_t ranKey = dis(generator) % 5 < 4 ? ranKeys[dis(generator) % size] : dis(generator);
                QueryPerformanceFrequency(&cpuFreq);
                QueryPerformanceCounter(&startTime);
                store.del(ranKey);
                QueryPerformanceCounter(&endTime);
                runTime = (double)((endTime.QuadPart - startTime.QuadPart) * 1000000) / (double)cpuFreq.QuadPart;
                out << runTime << ',';
            }
            out << std::endl;
            break;

    }
}

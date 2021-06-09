#include <iostream>
#include <cstdint>
#include <random>
#include <string>
#include <algorithm>

#include "test.h"

class CorrectnessTest : public Test {
private:
    const uint64_t SIMPLE_TEST_MAX = 512;
    const uint64_t LARGE_TEST_MAX = 1024 * 32;

    void regular_test(uint64_t max)
    {
        uint64_t i;

        // Test a single key
        EXPECT(not_found, store.get(1));
        store.put(1, "SE");
        EXPECT("SE", store.get(1));
        EXPECT(true, store.del(1));
        EXPECT(not_found, store.get(1));
        EXPECT(false, store.del(1));

        phase();

        // Test multiple key-value pairs
        std::vector<int> keys(max);
        for (i = 0; i < max; ++i) {
            keys[i] = i;
        }
        std::shuffle(keys.begin(), keys.end(), std::mt19937(std::random_device()()));

        for (i = 0; i < max; ++i) {
            store.put(keys[i], std::string(keys[i] + 1, 's'));
        }

        // Test after all insertions
        for (i = 0; i < max; ++i) {
            std::string got = store.get(i);
            EXPECT(std::string(i+1, 's'), got);
            if (std::string(i+1, 's') != got) {
                printf("i = %d\nreal size = %d\n", i, got.size());
                return;
            }
        }

        phase();


        // Test deletions
        std::shuffle(keys.begin(), keys.end(), std::mt19937(std::random_device()()));
        for (i = 0; i < max; i+=2)
            EXPECT(true, store.del(i));
        phase();

        for (i = 0; i < max; ++i)
            EXPECT((i & 1) ? std::string(i+1, 's') : not_found,
                   store.get(i));
        phase();

        for (i = 0; i < max; ++i)
            EXPECT(keys[i] & 1, store.del(keys[i]));
        phase();

        for (i = 0; i < max; ++i)
            EXPECT(not_found,
                   store.get(i));

        phase();

        report();
    }

public:
    CorrectnessTest(const std::string &dir, bool v=true) : Test(dir, v)
    {
    }

    void start_test(void *args = NULL) override
    {
        std::cout << "KVStore Correctness Test" << std::endl;

        std::cout << "[Simple Test]" << std::endl;
        regular_test(SIMPLE_TEST_MAX);

        std::cout << "[Large Test]" << std::endl;
        regular_test(LARGE_TEST_MAX);
    }
};

int main(int argc, char *argv[])
{
    bool verbose = (argc == 2 && std::string(argv[1]) == "-v");

    std::cout << "Usage: " << argv[0] << " [-v]" << std::endl;
    std::cout << "  -v: print extra info for failed tests [currently ";
    std::cout << (verbose ? "ON" : "OFF")<< "]" << std::endl;
    std::cout << std::endl;
    std::cout.flush();

    CorrectnessTest test("./data", verbose);

    test.start_test();
    return 0;
}



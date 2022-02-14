#include "kvstore.h"

int main() {
    KVStore store("../cmake-build-release/data");
    std::string got = store.get(1);
    printf("%s is what\n", got.c_str());
    return 0;
}

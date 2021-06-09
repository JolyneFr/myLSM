#include "kvstore.h"

int main() {
    KVStore store("../cmake-build-release/data");
    std::string got = store.get(12882);
    bool flag = std::string(12883, 's') == got;
    printf("%d %d\n", got.size(), flag);
    return 0;
}

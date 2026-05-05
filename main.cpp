#include "include/mini_os.h"

int main() {
    MiniOS os;
    os.boot();
    os.run();
    os.shutdown();
    return 0;
}

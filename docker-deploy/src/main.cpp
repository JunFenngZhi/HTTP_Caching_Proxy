#include "proxy.h"

using namespace std;


int main() {
    const char* port = "12345";
    proxy proxy(port);
    proxy.run();
    return 1;
}
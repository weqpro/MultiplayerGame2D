#include "server/ServerApp.h"
#include <iostream>

int main () {
    server::ServerApp app(1234, 32);
    std::cout << "Started server on port 1234\n";
    app.GameLoop();
}

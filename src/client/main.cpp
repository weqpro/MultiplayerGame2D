#include "client/ClientApp.h"

int main () {
    client::ClientApp client(1234);
    client.GameLoop();
    return 0;
}

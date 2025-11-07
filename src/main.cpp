#include "server.h"
#include <iostream>
#include <cstdlib>

using namespace std;
int main() {
    const char* dbUser = getenv("KV_DB_USER");
    const char* dbPass = getenv("KV_DB_PASS");

    string user = dbUser ? dbUser : "kvuser";
    string pass = dbPass ? dbPass : "kvpass";

    // Initialize and start the KVServer
    KVServer server("localhost", user, pass, "kvdb");

    server.start("0.0.0.0", 8080);
    return 0;
}

#include "server.h"
#include <iostream>
#include <cstdlib>

using namespace std;
int main() {
    const char* dbHost = getenv("KV_DB_HOST");
    const char* dbUser = getenv("KV_DB_USER");
    const char* dbPass = getenv("KV_DB_PASS");

    string host = dbHost ? dbHost : "localhost";
    string user = dbUser ? dbUser : "kvuser";
    string pass = dbPass ? dbPass : "kvpass";

    // Initialize and start the KVServer
    KVServer server(host, user, pass, "kvdb");

    server.start("0.0.0.0", 8080);
    return 0;
}

#include <iostream>
#include <string>
#include <libpq-fe.h> // PostgreSQL C API header

using namespace std;

//connection pointer (session with postgresSQL)
PGconn *conn = nullptr;

bool connectToDB(){
    const char *conninfo = "host=localhost port=5432 dbname=kv_db user=userkv password=userpass";
    conn = PQconnectdb(conninfo);

    if(PQstatus(conn) != CONNECTION_OK) {
        cerr << "Connection to database failed: " << PQerrorMessage(conn) << endl;
        conn = nullptr;
        return false;
    }

    cout<< "Connected to database successfully." << endl;
    return true;
}

void disconnectfromDB(){
    if(conn){
        PQfinish(conn);
        conn = nullptr;
        cout << "Disconnected from database." << endl;
    }
}

bool put(int key, string value){
    if(!conn) {
        cerr << "No active database connection." << endl;
        return false;
    }

    string query = "INSERT INTO kv_table (key, value) VALUES (" + to_string(key) + ", '" + value + "') "
                   "ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;";

    PGresult *res = PQexec(conn, query.c_str());

    if(PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "PUT failed: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

string get(int key){
    if(!conn) {
        cerr << "No active database connection." << endl;
        return "";
    }

    string query = "SELECT value FROM kv_table WHERE key = " + to_string(key) + ";";
    PGresult *res = PQexec(conn, query.c_str());

    if(PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "GET failed: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return "";
    }

    if(PQntuples(res) == 0) {
        PQclear(res);
        return ""; // Key not found
    }

    string value = PQgetvalue(res, 0, 0);
    PQclear(res);
    return value;
}

bool deletekey(int key){
    if(!conn) {
        cerr << "No active database connection." << endl;
        return false;
    }

    string query = "DELETE FROM kv_table WHERE key = " + to_string(key) + ";";
    PGresult *res = PQexec(conn, query.c_str());

    if(PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "DELETE failed: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

int main() {
    if(!connectToDB()) {
        return 1;
    }

    cout << "\n----Testing DB operations----\n" << endl;

    // Test PUT
    put(1, "C++ is nice!");
    put(2, "Rust is good!");

    // Test GET
    cout << "Get 1: " << get(1) << endl;
    cout << "Get 2: " << get(2) << endl;

    //Test UPDATE
    put(1, "C++ is awesome!");
    cout << "GET 1 (UPDATED): " << get(1) << endl;
    
    //Test DELETE
    deletekey(1);
    cout << "Get 1 (after DELETE): " << get(1) << endl;

    disconnectfromDB();
    return 0;
}

# Setup for HTTP KV Server

A multithreaded HTTP key-value server with an LRU cache and MySQL backend.

## Option A - Run with Docker

Requires Docker

``` bash
docker compose up --build
```

This starts MySQL and server together andit listens to port 8080

To stop:

``` bash
docker compose down 
```

## Option B --- Run natively

### Install dependencies

``` bash
sudo apt update
sudo apt install g++ cmake libmysqlclient-dev mysql-server -y
```
- g++ -> c++ compiler
- cmake -> build system
- libmysqlclient-dev -> For mysql headers
- mysql-server -> mysql database

### Configure database and table

``` bash
chmod +x scripts/init_db.sh
./scripts/init_db.sh
```

## Build the project

``` bash
mkdir build && cd build
cmake ..
make
```

## Run the server
``` bash
./kv_server
```

## Test using curl 

``` bash
curl -X POST -d "key=1&value=hello" http://localhost:8080/create
curl http://localhost:8080/read?key=1
curl -X DELETE http://localhost:8080/delete?key=1
curl http://localhost:8080/metrics
```
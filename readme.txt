# Setup for HTTP KV Server

##Install necessary components

sudo apt update
sudo apt install g++ cmake libmysqlclient-dev mysql-server -y

g++ -> c++ compiler
cmake -> build system
libmysqlclient-dev -> For mysql headers
mysql-server -> mysql database

##Run the provided script to configure database and table

chmod +x scripts/init_db.sh
./scripts/init_db.sh

##Build the project

mkdir build
cd build
cmake ..
make

##Run the server
./kv_server

##Test using curl 

curl -X POST -d "key=1&value=hello" http://localhost:8080/create
curl http://localhost:8080/read?key=1
curl -X DELETE http://localhost:8080/delete?key=1
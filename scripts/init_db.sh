#!/bin/bash

set -e

DB_NAME="kvdb"
DB_USER="kvuser"
DB_PASS="kvpass"
TABLE_NAME="kvtable"

echo "Starting MySQL service (if not already running)..."
sudo service mysql start

echo "Creating database '$DB_NAME' and user '$DB_USER'..."

sudo mysql <<SQL
CREATE DATABASE IF NOT EXISTS ${DB_NAME};
CREATE USER IF NOT EXISTS '${DB_USER}'@'localhost' IDENTIFIED BY '${DB_PASS}';
GRANT ALL PRIVILEGES ON ${DB_NAME}.* TO '${DB_USER}'@'localhost';
FLUSH PRIVILEGES;

USE ${DB_NAME};
CREATE TABLE IF NOT EXISTS ${TABLE_NAME} (
  k INT PRIMARY KEY,
  v TEXT NOT NULL
);
SQL

echo
echo "Database and user setup complete!"
echo "Database: ${DB_NAME}"
echo "User: ${DB_USER}"
echo "Password: ${DB_PASS}"
echo
echo "You can test with:"
echo "  mysql -u ${DB_USER} -p${DB_PASS} ${DB_NAME}"

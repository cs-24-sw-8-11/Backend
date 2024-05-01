#pragma once

#include <format>
#include <ranges>
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <memory>

#include "Globals.hpp"

using namespace std;

#define EXCEPTION_HANDLER catch(SQLite::Exception& e) { \
    string msg;                                         \
    switch (e.getErrorCode()) {                         \
        case 19:                                        \
            msg = "Error! A keyword not followed!";     \
            break;                                      \
        default:                                        \
            msg = "Error! Unknown error!";              \
            break;                                      \
    }                                                   \
    cerr                                                \
        << "\033[38;2;255;0;0m"                         \
        << msg                                          \
        << "\033[0m"                                    \
        << endl;                                        \
    cerr                                                \
        << "\t"                                         \
        << e.getErrorCode()                             \
        << ":\t\033[38;2;100;100;100m"                  \
        << e.what() << "\033[0m"                        \
        << endl;                                        \
}

/*
This is a custom data structure, a key value pair of strings that we can use for the database.
*/
class Pair {
 private:
    string _key;
    string _value;
 public:
    Pair(string key, string value) {
        this->_key = key;
        this->_value = value;
    }
    string key() const {
        return this->_key;
    }
    string value() const {
        return this->_value;
    }
    void set(string value){
        this->_value = value;
    }
};

/*
This is the row class, it is an abstraction of the rows in the database.

### Methods
it has 4 public methods along with 3 overloaded operators
- keys()
  returns a vector of keys
- values()
  returns a vector of values
- put(key, value)
  inserts a value at the key position of the row, if it already exists it will update the value
- has(key)
  returns a boolean whether the row contains a key

- operator[], operator!= and operator== is overloaded
*/

/// @brief Data structure for a single row in the database.
class Row {
 private:
    vector<Pair> data;

 public:
    /// @brief Returns all the colomn names of the table the row belongs to.
    vector<string> keys() {
        vector<string> keys;
        for (auto pair : data) {
            keys.push_back(pair.key());
        }
        return keys;
    }
    /// @brief Returns all the values in the row object.
    vector<string> values() {
        vector<string> values;
        for (auto pair : data) {
            values.push_back(pair.value());
        }
        return values;
    }
    void put(string key, string value) {
        if (this->has(key)) {
            for (auto pair : this->data) {
                if (pair.key() == key) {
                    pair.set(value);
                }
            }
        } else {
            this->data.push_back(Pair{key, value});
        }
    }
    string operator[](string key) {
        for (auto pair : data) {
            if (pair.key() == key) {
                return pair.value();
            }
        }
        throw exception();
    }
    inline bool has(string key) {
        for (auto pair : data) {
            if (pair.key() == key) {
                return true;
            }
        }
        return false;
    }
    inline bool operator!=(const Row& rhs) {
        for (auto pair : rhs.data) {
            if (!(this->has(pair.key()))) {
                return true;
            }
            if (this->operator[](pair.key()) != pair.value()) {
                return true;
            }
        }
        return false;
    }
    inline bool operator==(const Row& rhs) {
        return !(*(this) != rhs);
    }
};

/// @brief Data Structure for a table in the database.
class Table {
 private:
    string name;
    vector<string> columns;
    shared_ptr<SQLite::Database> db;

    /// @brief Creates an SQL statement object with a given SQL query, abstracting database and printing
    /// @param sql
    SQLite::Statement make_statement(string sql) {
        if (VERBOSE)
            cout << "\033[38;2;100;100;100mSQL: " << sql << "\033[0m" << endl;
        return SQLite::Statement(*(this->db), sql);
    }

 public:
    Table(string name,
          vector<string> columns,
          shared_ptr<SQLite::Database> db) {
        this->name = name;
        this->columns = columns;
        this->db = db;
        string qs;
        for (auto i = 0; i < columns.size(); i++) {
            qs += ", ";
            qs += columns[i];
        }
        auto sql = format(
            "CREATE TABLE IF NOT EXISTS {} (id INTEGER PRIMARY KEY{})",
            this->name,
            qs);
        SQLite::Statement query = make_statement(sql);
        try {
            query.exec();
        }
        EXCEPTION_HANDLER;
    }
    size_t size() {
        SQLite::Statement query = make_statement(
            format("SELECT id FROM {}",
            this->name));
        auto size = 0;
        while (query.executeStep()) size++;
        return size;
    }
    int add(vector<string> keys, vector<string> values) {
        string qs = "?";
        string keystring = keys[0];
        for (auto i = 1; i < keys.size(); i++) {
            qs += ", ?";
            keystring += ", ";
            keystring += keys[i];
        }
        auto sql = format(
            "INSERT INTO {0} ({1}) VALUES ({2});",
            this->name,
            keystring, qs);
        auto sql2 = format(
            "SELECT id FROM {0} ORDER BY id DESC LIMIT 1",
            this->name);
        SQLite::Statement query = make_statement(sql);
        SQLite::Statement query2 = make_statement(sql2);
        for (auto i = 0; i < values.size(); i++) {
            query.bind(i+1, values[i]);
        }
        int id = -1;
        try {
            query.exec();
            query2.executeStep();
            id = query2.getColumn("id").getInt();
        }
        EXCEPTION_HANDLER;
        return id;
    }
    vector<int> get_where(string key, string value) {
        SQLite::Statement query = make_statement(
            format("SELECT id FROM {} WHERE {} = ?",
            this->name,
            key));
        query.bind(1, value);
        vector<int> res;
        try {
            while (query.executeStep()) {
                res.push_back(query.getColumn("id").getInt());
            }
        }
        EXCEPTION_HANDLER;
        return res;
    }
    vector<int> get_where() {
        SQLite::Statement query = make_statement(
            format("SELECT id FROM {}",
            this->name));
        vector<int> res;
        try {
            while (query.executeStep()) {
                res.push_back(query.getColumn("id").getInt());
            }
        }
        EXCEPTION_HANDLER;
        return res;
    }
    vector<int> get_where_like(string key, string value) {
        SQLite::Statement query = make_statement(
            format("SELECT id FROM {} WHERE {} like '%' || ? || '%'",
            this->name,
            key));
        query.bind(1, value);
        vector<int> res;
        try {
            while (query.executeStep()) {
                res.push_back(query.getColumn("id").getInt());
            }
        }
        EXCEPTION_HANDLER;
        return res;
    }
    Row get(int id) {
        Row row;
        SQLite::Statement query = make_statement(
            format("SELECT * FROM {} WHERE id = ?",
            this->name));
        query.bind(1, id);
        try {
            query.executeStep();
            auto colcnt = query.getColumnCount();
            for (auto i = 0; i < colcnt; i++) {
                row.put(query.getColumnName(i),
                    query.getColumn(query.getColumnName(i)).getString());
            }
        }
        EXCEPTION_HANDLER;
        return row;
    }
    void delete_item(int id) {
        SQLite::Statement query = make_statement(
            format("DELETE FROM {} WHERE id = ?",
            this->name));
        query.bind(1, id);
        try {
            query.exec();
        }
        EXCEPTION_HANDLER;
    }
    void modify(int id,
                vector<string> keys,
                vector<string> values){
        for (auto [key, value] : views::zip(keys, values)) {
            SQLite::Statement query = make_statement(
                format("UPDATE {} SET {} = ? WHERE id = ?",
                this->name,
                key));
            query.bind(1, value);
            query.bind(2, id);
            try {
                query.exec();
            }
            EXCEPTION_HANDLER;
        }
    }
};

class TableFactory {
 private:
    shared_ptr<SQLite::Database> db;
 public:
    explicit TableFactory(string path) {
        this->db = make_shared<SQLite::Database>(path,
            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    }
    shared_ptr<Table> create(string name, vector<string> columns){
        shared_ptr<Table> t = make_shared<Table>(
            name,
            columns,
            this->db);
        if (VERBOSE)
            cout << "Initialized " << name << " table" << endl;
        return t;
    }
};

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

using namespace std;
using namespace std::ranges;

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
class Row {
 private:
    vector<pair<string, string>> data;

 public:
    vector<string> keys() {
        vector<string> keys;
        for (auto [key, value] : data) {
            keys.push_back(key);
        }
        return keys;
    }
    vector<string> values() {
        vector<string> values;
        for (auto [key, value] : data) {
            values.push_back(value);
        }
        return values;
    }
    void put(string target_key, string target_value) {
        if (this->has(target_key)) {
            for (auto pair : this->data) {
                if (pair.first == target_key) {
                    pair.second = target_value;
                }
            }
        } else {
            this->data.push_back(make_pair(target_key, target_value));
        }
    }
    string operator[](string target_key) {
        for (auto [key, value] : data) {
            if (key == target_key) {
                return value;
            }
        }
        throw exception();
    }
    inline bool has(string target_key) {
        for (auto [key, value] : data) {
            if (key == target_key) {
                return true;
            }
        }
        return false;
    }
    inline bool operator!=(const Row& rhs) {
        for (auto [key, value] : rhs.data) {
            if (!(this->has(key))) {
                return true;
            }
            if (this->operator[](key) != value) {
                return true;
            }
        }
        return false;
    }
    inline bool operator==(const Row& rhs) {
        return !(*(this) != rhs);
    }
};

class Table {
 private:
    string name;
    vector<string> columns;
    shared_ptr<SQLite::Database> db;

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
        SQLite::Statement query(*(this->db), sql);
        try {
            query.exec();
        }
        EXCEPTION_HANDLER;
    }
    size_t size() {
        SQLite::Statement query(*(this->db),
            std::format("SELECT id FROM {}",
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
        SQLite::Statement query(*(this->db), sql);
        SQLite::Statement query2(*(this->db), sql2);
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
        SQLite::Statement query(*(this->db),
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
        SQLite::Statement query(*(this->db),
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
    Row get(int id) {
        Row row;
        SQLite::Statement query(*(this->db),
            std::format("SELECT * FROM {} WHERE id = ?",
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
        SQLite::Statement query(*(this->db),
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
        for (auto [key, value] : zip_view(keys, values)) {
            SQLite::Statement query(*(this->db),
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
    shared_ptr<Table> create(string name,
                                  vector<string> columns){
        shared_ptr<Table> t = make_shared<Table>(
            name,
            columns,
            this->db);
        return t;
    }
};

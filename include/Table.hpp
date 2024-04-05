#include <string>
#include <vector>
#include <format>
#include <iostream>
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <SQLiteCpp/SQLiteCpp.h>

#define EXCEPTION_HANDLER catch(SQLite::Exception& e){  \
    std::string msg;                                    \
    switch(e.getErrorCode()){                           \
        case 19:                                        \
            msg = "Error! A keyword not followed!";     \
            break;                                      \
        default:                                        \
            msg = "Error! Unknown error!";              \
            break;                                      \
    }                                                   \
    std::cerr                                           \
        << "\033[38;2;255;0;0m"                         \
        << msg                                          \
        << "\033[0m"                                    \
        << std::endl;                                   \
    std::cerr                                           \
        << "\t"                                         \
        << e.getErrorCode()                             \
        << ":\t\033[38;2;100;100;100m"                  \
        << e.what() << "\033[0m"                        \
        << std::endl;                                   \
}

/*
This is a custom data structure, a key value pair of strings that we can use for the database.
*/
class Pair {
    private:
        std::string _key;
        std::string _value;
    public:
        Pair(std::string key, std::string value){
            this->_key = key;
            this->_value = value;
        }

        std::string key() const {
            return this->_key;
        }
        std::string value() const {
            return this->_value;
        }
        void set(std::string value){
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
class Row {
    private:
        std::vector<Pair> data;

    public:
        std::vector<std::string> keys(){
            std::vector<std::string> keys;
            for(auto pair : data){
                keys.push_back(pair.key());
            }
            return keys;
        }
        std::vector<std::string> values(){
            std::vector<std::string> values;
            for(auto pair : data){
                values.push_back(pair.value());
            }
            return values;
        }

        void put(std::string key, std::string value){
            if(this->has(key)){
                for(auto pair : this->data){
                    if(pair.key() == key){
                        pair.set(value);
                    }
                }
            }
            else{
                this->data.push_back(Pair{key, value});
            }
        }

        std::string operator[](std::string key){
            for(auto pair : data){
                if(pair.key() == key){
                    return pair.value();
                }
            }
            throw std::exception();
        }

        inline bool has(std::string key){
            for(auto pair : data){
                if(pair.key() == key){
                    return true;
                }
            }
            return false;
        } 

        inline bool operator!=(const Row& rhs){
            for(auto pair : rhs.data){
                if(!(this->has(pair.key()))){
                    return true;
                }
                if(this->operator[](pair.key()) != pair.value()){
                    return true;
                }
            }
            return false;
        }
        inline bool operator==(const Row& rhs){
            return !(*(this) != rhs);
        }
};

class Table {
    private:
        std::string name;
        std::vector<std::string> columns;
        std::shared_ptr<SQLite::Database> db;
    public:
        Table(std::string name, std::vector<std::string> columns, std::shared_ptr<SQLite::Database> db){
            this->name = name;
            this->columns = columns;
            this->db = db;
            std::string qs;
            for(auto i = 0; i < columns.size(); i++){
                qs += ", ";
                qs += columns[i];
            }
            auto sql = std::format("CREATE TABLE IF NOT EXISTS {} (id INTEGER PRIMARY KEY{})", this->name, qs);
            SQLite::Statement query(*(this->db), sql);
            try{
                query.exec();
            }
            EXCEPTION_HANDLER;
        }
        size_t size(){
            SQLite::Statement query(*(this->db), std::format("SELECT id FROM {}", this->name));
            auto size = 0;
            while(query.executeStep()) size++;
            return size;

        }
        void add(std::vector<std::string> keys, std::vector<std::string> values){
            std::string qs = "?";
            std::string keystring = keys[0];
            for(auto i = 1; i < keys.size(); i++){
                qs += ", ?";
                keystring += ", ";
                keystring += keys[i];
            }
            auto sql = std::format("INSERT INTO {} ({}) VALUES ({})", this->name, keystring, qs);
            SQLite::Statement query(*(this->db), sql);
            for(auto i = 0; i < values.size(); i++){
                query.bind(i+1, values[i]);
            }
            try{
                query.exec();
            }
            EXCEPTION_HANDLER;
        }

        std::vector<int> get_where(std::string key, std::string value){
            SQLite::Statement query(*(this->db), std::format("SELECT id FROM {} WHERE {} = ?", this->name, key));
            query.bind(1, value);
            std::vector<int> res;
            try{
                while(query.executeStep()){
                    res.push_back(query.getColumn("id").getInt());
                }
            }
            EXCEPTION_HANDLER;
            return res;
        }
        std::vector<int> get_where(){
            SQLite::Statement query(*(this->db), std::format("SELECT id FROM {}", this->name));
            std::vector<int> res;
            try{
                while(query.executeStep()){
                    res.push_back(query.getColumn("id").getInt());
                }
            }
            EXCEPTION_HANDLER;
            return res;
        }

        Row get(int id){
            Row row;
            SQLite::Statement query(*(this->db), std::format("SELECT * FROM {} WHERE id = ?", this->name));
            query.bind(1, id);
            try{
                query.executeStep();
                auto colcnt = query.getColumnCount();
                for(auto i = 0; i < colcnt; i++){
                    row.put(query.getColumnName(i), query.getColumn(query.getColumnName(i)).getString());
                }
            }
            EXCEPTION_HANDLER;
            return row;
        }

        void delete_item(int id){
            SQLite::Statement query(*(this->db), std::format("DELETE FROM {} WHERE id = ?", this->name));
            query.bind(1, id);
            try{
                query.exec();
            }
            EXCEPTION_HANDLER;
        }

        void modify(int id, std::vector<std::string> keys, std::vector<std::string> values){
            for(auto pair : std::views::zip(keys, values)){
                auto key = std::get<0>(pair);
                auto value = std::get<1>(pair);
                SQLite::Statement query(*(this->db), std::format("UPDATE {} SET {} = ? WHERE id = ?", this->name, key));
                query.bind(1, value);
                query.bind(2, id);
                try{
                    query.exec();
                }
                EXCEPTION_HANDLER;
            }
        }
};

class TableFactory {
    private:
        std::shared_ptr<SQLite::Database> db;
    public: 
        TableFactory(std::string path){
            this->db = std::make_shared<SQLite::Database>(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        }
        std::shared_ptr<Table> create(std::string name, std::vector<std::string> columns){
            std::shared_ptr<Table> t = std::make_shared<Table>(name, columns, this->db);
            return t;
        };
};
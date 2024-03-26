#include <string>
#include <vector>
#include <format>
#include <iostream>
#include <SQLiteCpp/SQLiteCpp.h>

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
            std::string qs = columns[0];
            for(auto i = 1; i < columns.size(); i++){
                qs += ", ";
                qs += columns[i];
            }
            auto sql = std::format("CREATE TABLE IF NOT EXISTS {} ({})", this->name, qs);
            SQLite::Statement query(*(this->db), sql);
            query.exec();
        }
        size_t size(){
            SQLite::Statement query(*(this->db), std::format("select * from {}", this->name));
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
            query.exec();
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
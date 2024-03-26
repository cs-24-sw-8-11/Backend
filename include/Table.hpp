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
            std::string qs = "?";
            for(auto i = 1; i < columns.size(); i++){
                qs += ", ?";
            }
            SQLite::Statement query(*(this->db), std::format("CREATE TABLE IF NOT EXISTS {} values ({})", this->name, qs));
            for(auto i = 0; i < columns.size(); i++){
                query.bind(i, columns[i]);
            }
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
            for(auto i = 1; i < columns.size(); i++){
                qs += ", ?";
                keystring += ", ";
                keystring += keys[i];
            }
            SQLite::Statement query(*(this->db), std::format("insert into {} ({}) values ({})", this->name, keystring, qs));
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
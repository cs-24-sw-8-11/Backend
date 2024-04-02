#include <string>
#include <vector>
#include <format>
#include <iostream>
#include <unordered_map>
#include <ranges>
#include <SQLiteCpp/SQLiteCpp.h>

class Table {
    private:
        std::string name;
        std::vector<std::string> columns;
        std::shared_ptr<SQLite::Database> db;
        void print_exception(SQLite::Exception e, std::string msg){
            std::cerr << "\033[38;2;255;0;0m" << msg << "\033[0m" << std::endl;
            std::cerr << "\t" << e.getErrorCode() << ":\t\033[38;2;100;100;100m" << e.what() << "\033[0m" << std::endl;
        }
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
            auto sql = std::format("CREATE TABLE IF NOT EXISTS {} (id integer primary key{})", this->name, qs);
            SQLite::Statement query(*(this->db), sql);
            try{
                query.exec();
            }
            catch(SQLite::Exception& e){
                std::string msg;
                switch(e.getErrorCode()){
                    default:
                        msg = "Unknown error!";
                        break;
                }
                print_exception(e, msg);
            }
        }
        size_t size(){
            SQLite::Statement query(*(this->db), std::format("select id from {}", this->name));
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
            catch(SQLite::Exception& e){
                std::string msg;
                switch(e.getErrorCode()){
                    case 19:
                        msg = "Error! Unique keyword not followed!";
                        break;
                    default:
                        msg = "Unknown error!";
                        break;
                }
                print_exception(e, msg);
            }
        }

        std::vector<int> get_where(std::string key, std::string value){
            SQLite::Statement query(*(this->db), std::format("SELECT id FROM {} where {} = ?", this->name, key));
            query.bind(1, value);
            std::vector<int> res;
            try{
                while(query.executeStep()){
                    res.push_back(query.getColumn("id").getInt());
                }
            }
            catch(SQLite::Exception& e){
                std::string msg;
                switch(e.getErrorCode()){
                    case -1:
                        msg = std::format("Error! no value of '{}' in column: '{}' of table: '{}' exists", value, key, this->name);
                        break;
                    default:
                        msg = "Unknown error!";
                        break;
                }
                print_exception(e, msg);
                res.push_back(-1);
            }
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
            catch(SQLite::Exception& e){
                std::string msg;
                switch(e.getErrorCode()){
                    default:
                        msg = "Unknown error!";
                        break;
                }
                print_exception(e, msg);
                res.push_back(-1);
            }
            return res;
        }

        std::unordered_map<std::string, std::string> get(int id){
            std::unordered_map<std::string, std::string> map;
            SQLite::Statement query(*(this->db), std::format("SELECT * FROM {} where id = ?", this->name));
            query.bind(1, id);
            try{
                query.executeStep();
                auto colcnt = query.getColumnCount();
                for(auto i = 0; i < colcnt; i++){
                    map[query.getColumnName(i)] = query.getColumn(query.getColumnName(i)).getString();
                }
            }
            catch(SQLite::Exception& e){
                std::string msg;
                switch(e.getErrorCode()){
                    case -1:
                        msg = std::format("Error! no row with id: {}", id);
                        break;
                    default:
                        msg = "Unknown error!";
                        break;
                }
                print_exception(e, msg);
            }
            return map;
        }

        void delete_item(int id){
            SQLite::Statement query(*(this->db), std::format("DELETE FROM {} where id = ?", this->name));
            query.bind(1, id);
            try{
                query.exec();
            }
            catch(SQLite::Exception& e){
                std::string msg;
                switch(e.getErrorCode()){
                    default:
                        msg = "Unknown error!";
                        break;
                }
                print_exception(e, msg);
            }
        }

        void modify(int id, std::vector<std::string> keys, std::vector<std::string> values){
            for(auto pair : std::views::zip(keys, values)){
                auto key = std::get<0>(pair);
                auto value = std::get<1>(pair);
                SQLite::Statement query(*(this->db), std::format("update {} set {} = ? where id = ?", this->name, key));
                query.bind(1, value);
                query.bind(2, id);
                try{
                    query.exec();
                }
                catch(SQLite::Exception& e){
                    std::string msg;
                    switch(e.getErrorCode()){
                        default:
                            msg = "Unknown error!";
                            break;
                    }
                    print_exception(e, msg);
                }
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
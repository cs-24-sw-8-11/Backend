#include "Table.hpp"

class Database {
    public:
        std::shared_ptr<Table> users;
        Database(std::string path){
            TableFactory factory{path};
            this->users = factory.create("users", {
                "id INTEGER PRIMARY KEY",
                "username VARCHAR UNIQUE NOT NULL",
                "password VARCHAR NOT NULL",
                "userdataId INTEGER",
                "FOREIGN KEY(userdataId) REFERENCES userdata(id)"
            });
        }
};
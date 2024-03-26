#include "Database.hpp"

#include <cassert>
#include <string>
#include <iostream>

std::shared_ptr<Database> db;

/*void init_db(){
    std::remove("/tmp/db.db3");
    db = std::make_shared<Database>("/tmp/db.db3");
}

void test_add_user(){
    auto num_users = 1000;
    for(auto i = 0; i < num_users; i++){
        char* str;
        sprintf(str, "user%d", i);
        db->users->add({
            "username",
            "password"
        }, {
            "tester",
            "tester"
        });
    }
    assert(db->users->size() == num_users);
}*/

int main(){
    init_db();
    test_add_user();
}
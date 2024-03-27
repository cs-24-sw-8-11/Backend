#include "Database.hpp"

#include <cassert>
#include <string>
#include <iostream>
#include <format>

std::shared_ptr<Database> db;

void init_db(){
    std::remove("/tmp/db.db3");
    db = std::make_shared<Database>("/tmp/db.db3");
}

void test_add_user(){
    auto num_users = 1000;
    for(auto i = 0; i < num_users; i++){
        db->users->add({
            "username",
            "password"
        }, {
            std::format("user{}", i),
            "password"
        });
    }
    assert(db->users->size() == num_users);
}

void test_add_journal(){
    db->users->add({"username", "password"}, {"tester", "tester"});
    auto user_id = db->users->get_id("username", "tester");
    auto num_journals = 1000;
    for(auto i = 0; i < num_journals; i++){
        db->journals->add({"userId"}, {std::format("{}", user_id)});
    }
    assert(db->journals->size() == num_journals);
}

int main(){
    init_db();
    test_add_user();
}
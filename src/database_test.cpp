#include "Database.hpp"

#include <cassert>
#include <string>
#include <iostream>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>

std::shared_ptr<Database> db;
nlohmann::json config;
std::string default_username;
std::string default_password;
std::vector<std::string> default_questions;



void init(){
    std::remove("/tmp/db.db3");
    db = std::make_shared<Database>("/tmp/db.db3");
    std::ifstream file("files/testdata/default.json");
    config = nlohmann::json::parse(file);
    default_username = config["user"]["username"].get<std::string>();
    default_password = config["user"]["password"].get<std::string>();
    default_questions = config["questions"].get<std::vector<std::string>>();
}

void test_add_user(){
    auto num_users = 1000;
    for(auto i = 0; i < num_users; i++){
        db->users->add({
            "username",
            "password"
        }, {
            std::format("user{}", i),
            default_password
        });
    }
    assert(db->users->size() == num_users);
}

/*void test_delete_user(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    assert(db->users->size() == 1);
    db->users->delete(user_id);
    assert(db->users->size() == 0);
}*/

/*void test_modify_user(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    auto current_data = db->users->get(user_id);
    db->users->modify(user_id, {"username"}, {"tester1"});
    auto new_data = db->users->get(user_id);
    assert(current_data != new_data);
}*/

void test_add_journal(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    auto num_journals = 1000;
    for(auto i = 0; i < num_journals; i++){
        db->journals->add({"userId"}, {std::format("{}", user_id)});
    }
    assert(db->journals->size() == num_journals);
}

/*void test_delete_journal(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    db->journals->add({"userId"}, {std::format("{}", user_id)});
    assert(db->journals->size() == 1);
    auto journal_id = db->journals->get_where("userId", std::format("{}", user_id))[0];
    db->journals->delete(journal_id);
    assert(db->journals->size() == 1);
}*/

/*void test_modify_journal(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    db->journals->add({"userId"}, {std::format("{}", user_id)});
    auto journal_id = db->journals->get_where("userId", std::format("{}", user_id))[0];
    auto current_data = db->journals->get(journal_id);
    db->journals->modify(journal_id, {"userId", std::format("{}", user_id+1)});
    auto new_data = db->journals->get(journal_id);
    assert(current_data != new_data);
}*/

int main(){
    init(); // reinit between each test to clear environment
    test_add_user();
    init();
    test_add_journal();
}
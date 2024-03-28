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

auto num_additions = 100;


void init(){
    std::remove("db.db3");
    db = std::make_shared<Database>("db.db3");
    std::ifstream file("files/testdata/default.json");
    config = nlohmann::json::parse(file);
    default_username = config["user"]["username"].get<std::string>();
    default_password = config["user"]["password"].get<std::string>();
    default_questions = config["questions"].get<std::vector<std::string>>();
}

/* -------------------------------------------------
                Addition tests
*/// -----------------------------------------------

void test_add_user(){
    for(auto i = 0; i < num_additions; i++){
        db->users->add({
            "username",
            "password"
        }, {
            std::format("user{}", i),
            default_password
        });
    }
    assert(db->users->size() == num_additions);
}

void test_add_journal(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    for(auto i = 0; i < num_additions; i++){
        db->journals->add({"userId"}, {std::format("{}", user_id)});
    }
    assert(db->journals->size() == num_additions);
}
void test_add_question(){
    for(auto question : default_questions) 
        db->questions->add({"question", "tags"}, {question, "default"});

    assert(db->questions->size() == default_questions.size());
}

void test_add_answer(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    db->journals->add({"userId"}, {std::format("{}", user_id)});
    auto journal_id = db->journals->get_where("userId", std::format("{}", user_id))[0];
    for(auto question : default_questions) 
        db->questions->add({"question", "tags"}, {question, "default"});
    auto question_id = db->questions->get_where("tags", "default")[0];

    for(auto i = 0; i < num_additions; i++){
        db->answers->add({"answer", "journalId", "questionId"}, {std::format("answer{}", i), std::format("{}", journal_id), std::format("{}", question_id)});
    }
    assert(db->answers->size() == num_additions);
}

void test_add_setting(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where("username", default_username)[0];
    for(auto i = 0; i < num_additions; i++){
        db->settings->add({"userId", "key", "value"}, {std::format("{}", user_id), std::format("setting {}", i), std::format("value {}", i)});
    }
    assert(db->settings->size() == num_additions);
}

// testing userdata add is redundant with delete test


/* -------------------------------------------------
                Deletion tests
*/// -----------------------------------------------

/*void test_delete_user(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    assert(db->users->size() == 1);
    db->users->delete(user_id);
    assert(db->users->size() == 0);
}*/

/*void test_delete_journal(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->journals->add({"userId"}, {std::format("{}", user_id)});
    assert(db->journals->size() == 1);
    auto journal_id = db->journals->get_where())[0];
    db->journals->delete(journal_id);
    assert(db->journals->size() == 1);
}*/

/*void test_delete_question(){
    for(auto question : default_questions)
        db->questions->add({"question", "tags"}, {question, "default"});

    assert(db->questions->size() == default_questions.size());
    
    auto ids = db->questions->get_where();
    for(auto id : ids){
        db->questions->delete(id);
    }
    assert(db->questions->size() == 0);
}*/

/*void test_delete_answer(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->journals->add({"userId"}, {std::format("{}", user_id)});
    auto journal_id = db->journals->get_where()[0];
    for(auto question : default_questions)
        db->questions->add({"question", "tags"}, {question, "default"});
    auto question_id = db->questions->get_where()[0];
    db->answers->add({"answer", "journalId", "questionId"}, {"answer", std::format("{}", journal_id), std::format("{}", question_id)});
    assert(db->answers->size() == 1);
    auto answer_id = db->answers->get_where()[0];

    db->answers->delete(answer_id); 
    assert(db->answers->size() == 0);   
}*/

/*void test_delete_setting(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->settings->add({"userId", "key", "value"}, {std::format("{}", user_id), "testkey", "testvalue"});
    assert(db->settings->size() == 1);
    auto setting_id = db->settings->get_where()[0];
    db->settings->delete(setting_id);
    assert(db->settings->size() == 0);
}*/

/*void test_delete_userdata(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->userdata->add({"agegroup", "occupation", "userId"}, {"50-54", "unemployed", std::format("{}", user_id)});
    assert(db->userdata->size() == 1);
    auto userdata_id = db->userdata->get_where()[0];
    db->userdata->delete(userdata_id);
    assert(db->userdata->size() == 0);
}*/

/* -------------------------------------------------
                Modification tests
*/// -----------------------------------------------

/*void test_modify_user(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    auto current_data = db->users->get(user_id);
    db->users->modify(user_id, {"username"}, {"tester1"});
    auto new_data = db->users->get(user_id);
    assert(current_data != new_data);
}*/


/*void test_modify_journal(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->journals->add({"userId"}, {std::format("{}", user_id)});
    auto journal_id = db->journals->get_where()[0];
    auto current_data = db->journals->get(journal_id);
    db->journals->modify(journal_id, {"userId", std::format("{}", user_id+1)});
    auto new_data = db->journals->get(journal_id);
    assert(current_data != new_data);
}*/

/*void test_modify_question(){
    for(auto question : default_questions)
        db->questions->add({"question", "tags"}, {question, "default"});
    auto question_id = db->questions->get_where()[0];
    auto current_data = db->questions->get(question_id);
    db->questions->modify({"tags"}, {"notdefault"});
    auto new_data = db->questions->get(question_id);
    assert(current_data != new_data);
}*/

/*void test_modify_answer(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->journals->add({"userId"}, {std::format("{}", user_id)});
    auto journal_id = db->journals->get_where()[0];
    for(auto question : default_questions)
        db->questions->add({"question", "tags"}, {question, "default"});
    auto question_id = db->questions->get_where()[0];
    db->answers->add({"answer", "journalId", "questionId"}, {"answer", std::format("{}", journal_id), std::format("{}", question_id)});
    auto answer_id = db->answers->get_where()[0];
    auto current_data = db->answers->get(answer_id);
    db->answers->modify({"answer"}, {"betteranswer"});
    auto new_data = db->answers->get(answer_id);
    assert(current_data != new_data);
}*/

/*void test_modify_setting(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->settings->add({"userId", "key", "value"}, {std::format("{}", user_id), "testkey", "testvalue"});
    auto setting_id = db->settings->get_where()[0];
    auto current_data = db->settings->get(setting_id);
    db->settings->modify({"value"}, {"bettervalue"});
    auto new_data = db->settings->get(setting_id);
    assert(current_data != new_data);
}*/

/*void test_modify_userdata(){
    db->users->add({"username", "password"}, {default_username, default_password});
    auto user_id = db->users->get_where()[0];
    db->userdata->add({"agegroup", "occupation", "userId"}, {"50-54", "unemployed", std::format("{}", user_id)});
    auto userdata_id = db->userdata->get_where()[0];
    auto current_data = db->userdata->get(userdata_id);
    db->userdata->modify({"agegroup"}, {"55-59"});
    auto new_data = db->userdata->get(userdata_id);
    assert(current_data != new_data);
}*/

int main(){
    init(); // reinit between each test to clear environment
    test_add_user();
    init();
    test_add_journal();
    init();
    test_add_question();
    init();
    test_add_answer();
    init();
    test_add_setting();
}

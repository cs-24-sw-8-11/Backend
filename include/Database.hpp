#pragma once
#include <memory>
#include <string>

#include "Table.hpp"

using namespace std;


enum QuestionType {
    VALUED,
    BOOLEAN
};

enum UserState {
    TRAINING,
    STANDARD
};
// wrappers to make db access easy while keeping types
string db_int(int e) {
    return format("{}", e);
}

class Database {
 public:
    shared_ptr<Table> users;
    shared_ptr<Table> journals;
    shared_ptr<Table> answers;
    shared_ptr<Table> questions;
    shared_ptr<Table> settings;
    shared_ptr<Table> userdata;
    shared_ptr<Table> predictions;
    explicit Database(string path){
        TableFactory factory{path};
        this->users = factory.create("users", {
            "username VARCHAR UNIQUE NOT NULL",
            "password VARCHAR NOT NULL",
            "userdataId INTEGER",
            "state INTEGER NOT NULL",
            "FOREIGN KEY(userdataId) REFERENCES userdata(id)"
        });
        this->journals = factory.create("journals", {
            "comment varchar",
            "userId INTEGER NOT NULL",
            "FOREIGN KEY(userId) REFERENCES users(id)"
        });
        this->answers = factory.create("answers", {
            "answer VARCHAR NOT NULL",
            "journalId INTEGER NOT NULL",
            "questionId INTEGER NOT NULL",
            "FOREIGN KEY(journalId) REFERENCES journals(id)",
            "FOREIGN KEY(questionId) REFERENCES questions(id)"
        });
        this->questions = factory.create("questions", {
            "tags VARCHAR",
            "type INTEGER NOT NULL",
            "question VARCHAR NOT NULL"
        });
        this->settings = factory.create("settings", {
            "key VARCHAR NOT NULL",
            "value VARCHAR NOT NULL",
            "userId INTEGER NOT NULL",
            "FOREIGN KEY(userId) REFERENCES users(id)"
        });
        this->userdata = factory.create("userdata", {
            "agegroup VARCHAR NOT NULL",
            "occupation VARCHAR NOT NULL",
            "userId INTEGER NOT NULL"
        });
        this->predictions = factory.create("predictions", {
            "userId INTEGER NOT NULL",
            "value INTEGER NOT NULL",
            "FOREIGN KEY(userId) REFERENCES users(id)"
        });
    }
};

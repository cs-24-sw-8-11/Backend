#include "Table.hpp"

enum QuestionType {
    VALUED,
    BOOLEAN
};

class Database {
    public:
        std::shared_ptr<Table> users;
        std::shared_ptr<Table> journals;
        std::shared_ptr<Table> answers;
        std::shared_ptr<Table> questions;
        std::shared_ptr<Table> settings;
        std::shared_ptr<Table> userdata;
        std::shared_ptr<Table> predictions;

        Database(std::string path){
            TableFactory factory{path};
            this->users = factory.create("users", {
                "username VARCHAR UNIQUE NOT NULL",
                "password VARCHAR NOT NULL",
                "userdataId INTEGER",
                "FOREIGN KEY(userdataId) REFERENCES userdata(id)"
            });
            this->journals = factory.create("journals", {
                "comment varchar",
                "userId integer not null",
                "foreign key(userId) references users(id)"
            });
            this->answers = factory.create("answers", {
                "answer varchar not null",
                "journalId integer not null",
                "questionId integer not null",
                "foreign key(journalId) references journals(id)",
                "foreign key(questionId) references questions(id)"
            });
            this->questions = factory.create("questions", {
                "tags varchar",
                "type integer not null",
                "question varchar not null"
            });
            this->settings = factory.create("settings", {
                "key varchar not null",
                "value varchar not null",
                "userId integer not null",
                "foreign key(userId) references users(id)"
            });
            this->userdata = factory.create("userdata", {
                "agegroup varchar not null",
                "occupation varchar not null",
                "userId integer not null"
            });
            this->predictions = factory.create("predictions", {
                "userId integer not null",
                "value integer not null",
                "verifiedValue"
            });
        }
};
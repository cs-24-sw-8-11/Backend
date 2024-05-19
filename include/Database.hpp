#pragma once
#include <memory>
#include <string>

#include "Table.hpp"
#include "Globals.hpp"
#include "Logger.hpp"

using namespace std;
using namespace P8;

enum QuestionType {
    VALUED,
    BOOLEAN
};

enum UserState {
    TRAINING,
    STANDARD
};

enum JournalType {
    TRAINING_JOURNAL,
    PREDICTION_JOURNAL
};

class DbException : public exception {
    string why;
 public:
    explicit DbException(string why) {
        this->why = why;
    }
    string what() {
        return why;
    }
};

/// @brief Database object with points to all relevant tables with methods for retrieving and modification.
class Database {
 public:
    map<string, Table> tables;

    Database() = default;

    /// @brief Constructs the database object using the TableFactory.
    /// @param path
    explicit Database(string path) {
        TableFactory factory{path};
        tables["users"] = (factory.create("users", {
            "username VARCHAR UNIQUE NOT NULL",
            "password VARCHAR NOT NULL",
            "state INTEGER NOT NULL"
        }));
        tables["journals"] = (factory.create("journals", {
            "userId INTEGER NOT NULL",
            "timestamp INTEGER NOT NULL",
            "FOREIGN KEY(userId) REFERENCES users(id)"
        }));
        tables["answers"] = (factory.create("answers", {
            "value INTEGER NOT NULL",
            "rating INTEGER NOT NULL",
            "journalId INTEGER NOT NULL",
            "questionId INTEGER NOT NULL",
            "FOREIGN KEY(journalId) REFERENCES journals(id)",
            "FOREIGN KEY(questionId) REFERENCES questions(id)"
        }));
        tables["questions"] = (factory.create("questions", {
            "tags VARCHAR",
            "type INTEGER NOT NULL",
            "question VARCHAR NOT NULL"
        }));
        tables["settings"] = (factory.create("settings", {
            "key VARCHAR NOT NULL",
            "value VARCHAR NOT NULL",
            "userId INTEGER NOT NULL",
            "FOREIGN KEY(userId) REFERENCES users(id)"
        }));
        tables["userdata"] = (factory.create("userdata", {
            "userId INTEGER NOT NULL",
            "education INTEGER NOT NULL",
            "urban INTEGER NOT NULL",
            "gender INTEGER NOT NULL",
            "religion INTEGER NOT NULL",
            "orientation INTEGER NOT NULL",
            "race INTEGER NOT NULL",
            "married INTEGER NOT NULL",
            "age INTEGER NOT NULL",
            "pets INTEGER NOT NULL",
            "FOREIGN KEY(userId) REFERENCES users(id)"
        }));
        tables["predictions"] = (factory.create("predictions", {
            "userId INTEGER NOT NULL",
            "value INTEGER NOT NULL",
            "timestamp INTEGER NOT NULL",
            "FOREIGN KEY(userId) REFERENCES users(id)"
        }));
        tables["mitigations"] = (factory.create("mitigations", {
            "tags VARCHAR",
            "type INTEGER NOT NULL",
            "title VARCHAR NOT NULL",
            "description VARCHAR NOT NULL"
        }));
        tables["legends"] = (factory.create("legends", {
            "questionId INTEGER NOT NULL",
            "text VARCHAR NOT NULL",
            "legend_index INTEGER NOT NULL",
            "FOREIGN KEY(questionId) REFERENCES questions(id)"
        }));
        tables["ratings"] = (factory.create("ratings", {
            "predictionId INTEGER NOT NULL",
            "rating INTEGER NOT NULL",
            "expected BOOLEAN NOT NULL",
            "FOREIGN KEY(predictionId) REFERENCES predictions(id)"
        }));

        log<DEBUG>("Initialized all tables");
    }

    Table operator[](string key){
        if (!tables.contains(key))
            throw DbException(format("No such table {} exists", key));
        return tables[key];
    }
};

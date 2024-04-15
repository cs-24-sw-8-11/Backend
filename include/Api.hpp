#pragma once
#include <format>
#include <crow.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <utility>

#include <nlohmann/json.hpp>

#include "Database.hpp"
#include "PredictionManager.hpp"
#include "Endpoints/Users.hpp"

class API : public std::enable_shared_from_this<API> {
 public:
    crow::SimpleApp app;
    std::shared_ptr<Database> db;
    std::map<int, std::string> authedUsers;
    PredictionManager manager;
    std::string dbPath;

    std::shared_ptr<Users> users;

    explicit API(std::string path) {
        dbPath = path;
        db = std::make_shared<Database>(path);
    }
    void addChild(std::shared_ptr<Users> child)
    {
        children.push_back(child);

        // like this
        child->setParent(shared_from_this());  // ok
        //               ^^^^^^^^^^^^^^^^^^
    }
    void Run(int port) {
        addChild(std::make_shared<Users>(db));
        CROW_ROUTE(app, "/journals/new")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            nlohmann::json z = nlohmann::json::parse(req.body);
            if (!x) {
                return crow::response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<std::string>();
            auto comment = z["comment"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto data = z.at("data").items();
                auto journalid = db->journals->add({
                    "comment",
                    "userId"}, {
                    comment,
                    db_int(userid)});
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto questionId = i.value().back().get<std::string>();
                    auto answer = i.value().front().get<std::string>();
                    db->answers->add({
                        "answer",
                        "journalId",
                        "questionId"}, {
                        answer,
                        db_int(journalid), questionId});
                }
                return crow::response(200, "Successfully created new journal.");
            } else {
                return crow::response(403,
                    "Token does not match expected value!");
            }
        });
        // journal id
        CROW_ROUTE(app, "/journals/get/<int>")
        ([&](int jid) {
            crow::json::wvalue x({});
            if (jid < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            auto journal = db->journals->get(jid);
            for (auto key : journal.keys()) {
                x[key] = journal[key];
            }
            x["answers"] = db->answers->get_where(
                "journalId",
                db_int(jid));
            return x;
        });
        // user id
        CROW_ROUTE(app, "/journals/ids/<int>")
        ([&](int uid) {
            crow::json::wvalue x({});
            if (uid < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            auto journals = db->journals->get_where(
                "userId",
                db_int(uid));
            // Funky solution to make it only return json array
            x = std::move(journals);
            return std::move(x);
        });
        CROW_ROUTE(app, "/journals/delete/<int>/<int>/<string>")
        .methods("DELETE"_method)
        ([&](int userid, int journalid, std::string token) {
            if (authedUsers[userid] == token) {
                db->journals->delete_item(journalid);
                return crow::response(202, "Successfully deleted journal.");
            } else {
                return crow::response(403,
                    "Token does not match expected value!");
            }
        });
        CROW_ROUTE(app, "/answers/get/<int>/<string>")(
            [&](int answerid, std::string token){
            crow::json::wvalue x({});
            if (answerid < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            auto answer = db->answers->get(answerid);
            auto allowedToGetAnswer = authedUsers[std::stoi(db->journals->get(
                std::stoi(answer["journalId"]))["userId"])] == token;
            if (allowedToGetAnswer) {
                for (auto key : answer.keys()) {
                    x[key] = answer[key];
                }
            } else {
                x["error"] = "Not allowed to access other users' answers!";
            }
            return x;
        });
        CROW_ROUTE(app, "/settings/get/<int>")
        ([&](int userid) {
            std::vector<crow::json::wvalue> vec;
            crow::json::wvalue x({});
            if (userid < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            auto settings = db->settings->get_where(
                "userId",
                db_int(userid));
            for (auto setting : settings) {
                crow::json::wvalue z({});
                auto row = db-> settings->get(setting);
                for (auto key : row.keys()) {
                    z[key] = row[key];
                }
                vec.push_back(z);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/settings/update")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto x = crow::json::load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x) {
                return crow::response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto data = z.at("settings");
                auto userSettings = db->settings->get_where(
                    "userId",
                    db_int(userid));
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto key = i.key();
                    auto value = i.value().front().get<std::string>();
                    if (!SettingExists(userSettings, key)) {
                        db->settings->add({
                            "key",
                            "value",
                            "userId"}, {
                            key,
                            value,
                            db_int(userid)});
                    }
                    for (auto setting : userSettings) {
                        auto settingsRow = db->settings->get(setting);
                        if (settingsRow["key"] == key) {
                            db->settings->modify(setting, {"value"}, {value});
                        }
                    }
                }
                return crow::response(200, "Successfully updated settings.");
            } else{
                return crow::response(403,
                    "Token does not match expected value!");
            }
        });
        CROW_ROUTE(app, "/questions/defaults")
        ([&]() {
            auto questions = db->questions->get_where("tags", "default");
            std::vector<crow::json::wvalue> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                crow::json::wvalue x({});
                for (auto key : question.keys()) {
                    x[key] = question[key];
                }
                vec.push_back(x);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/questions/get/<string>")
        ([&](std::string tag) {
            auto questions = db->questions->get_where("tags", tag);
            std::vector<crow::json::wvalue> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                crow::json::wvalue x({});
                for (auto key : question.keys()) {
                    x[key] = question[key];
                }
                vec.push_back(x);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/predictions/get/<int>/<string>")
        ([&](int uid, std::string token) {
            if (uid < 0) {
                return crow::json::wvalue({{"error", "Invalid id"}});
            }
            std::vector<crow::json::wvalue> vec;
            auto predictions = db->predictions->get_where(
                "userId",
                db_int(uid));
            if (authedUsers[uid] == token) {
                for (auto prediction : predictions) {
                    auto row = db->predictions->get(prediction);
                    crow::json::wvalue x({});
                    for (auto key : row.keys()) {
                        x[key] = row[key];
                    }
                    vec.push_back(x);
                }
            } else {
                return crow::json::wvalue({{"error",
                    "Not allowed to access other users' predictions!"}});
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/predictions/add")
        .methods("POST"_method)([&](const crow::request& req){
            auto x = crow::json::load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x) {
                return crow::response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<std::string>();
            auto qid = z["questionid"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto prediction = manager.create_new_prediction(userid);
                auto answers = db->answers->get_where("questionId", qid);
                auto journals = db->journals->get_where(
                    "userId",
                    db_int(userid));
                for (auto answer : answers) {
                    if (std::count(
                        journals.begin(),
                        journals.end(),
                        std::stoi(db->answers->get(answer)["journalId"])) > 0) {
                        prediction.add_valued_data(qid,
                            std::stod(db->answers->get(answer)["answer"]));
                    }
                }
                auto predictionValue = prediction.build();
                db->predictions->add({
                    "userId",
                    "value"}, {
                    db_int(userid),
                    std::format("{}", predictionValue)});
                return crow::response(200, "Successfully added prediction");
            } else {
                return crow::response(403,
                    "Token does not match expected value!");
            }
        });

        app.port(port).multithreaded().run();
    }
    // Called whenever a user is registered to prevent empty settings
    void DefaultSettings(int userId) {
        db->settings->add({
            "key",
            "value",
            "userId"}, {
            "key",
            "value",
            db_int(userId)});
        db->settings->add({
            "key",
            "value",
            "userId"}, {
            "key1",
            "value2",
            db_int(userId)});
        // add more settings here
    }
    int64_t Hash(std::string username, std::string password) {
        auto hash1 = std::hash<std::string>{}(username);
        auto hash2 = std::hash<std::string>{}(password);
        auto combinedhash = hash1 ^ (hash2 << 1);
        return combinedhash;
    }
    int UserIdFromToken(std::string token) {
        for (auto user : authedUsers) {
            if (user.second == token) {
                return user.first;
            }
        }
        return 0;
    }
    bool SettingExists(std::vector<int> ids, std::string key) {
        for (auto id : ids) {
            auto row = db->settings->get(id);
            if (row["key"] == key) {
                return true;
            }
        }
        return false;
    }

 private:
    std::vector<std::weak_ptr<Users>> children;
};

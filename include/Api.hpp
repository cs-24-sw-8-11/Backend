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

using namespace crow::json;
using namespace crow;
using namespace std;

class API {
 public:
    SimpleApp app;
    shared_ptr<Database> db;
    map<int, std::string> authedUsers;
    PredictionManager manager;
    explicit API(string path) {
        db = std::make_shared<Database>(path);
    }
    void Run(int port) {
        CROW_ROUTE(app, "/user/get/<int>")
        ([&](int id) {
            wvalue x({});
            auto user = db->users->get(id);
            auto userdata = db->userdata->get(db->userdata->get_where(
                "userId",
                db_int(id)).front());
            if (id < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            for (auto key : user.keys()) {
                if (key == "password") {
                    continue;
                }
                x[key] = user[key];
            }
            for (auto key : userdata.keys()) {
                x[key] = userdata[key];
            }
            return x;
        });
        CROW_ROUTE(app, "/user/ids/<int>-<int>")
        ([&](int min, int max) {
            wvalue x;
            if (min < 0 || max < 0 || (max-min < 0)) {
                x["difference"] = max-min;
                x["max"] = max;
                x["min"] = min;
                x["error"] = "Invalid Range";
                return x;
            }
            auto users = db->users->get_where();
            vector<int> filteredUsers;
            for (const auto & user : users) {
                if (user <= max && user >= min) {
                    filteredUsers.push_back(user);
                }
            }
            // Funky solution to make it only return json array
            x = std::move(filteredUsers);
            // This is done due to "inconsistent type" errors appearing
            // if not done
            return std::move(x);
        });
        CROW_ROUTE(app, "/user/ids")
        ([&]() {
            wvalue x;
            auto users = db->users->get_where();
            x = std::move(users);
            return std::move(x);
        });
        CROW_ROUTE(app, "/user/auth")
        .methods("POST"_method)
        ([&](const request& req) {
            auto x = load(req.body);
            if (!x) {
                return response(400, "Unable to load/parse json");
            }
            auto username = x["username"].s();
            auto password = x["password"].s();
            auto dbpassword = db->users->get(db->users->get_where(
                "username",
                username).front())["password"];

            if (dbpassword == password) {
                auto userid = db->users->get_where(
                    "username",
                    username).front();
                auto hash = Hash((string)username, (string)password);
                auto token = format("{}", hash);
                authedUsers[userid] = token;
                return response(200, token);
            } else {
                return response(403, "Invalid Token");
            }
        });
        CROW_ROUTE(app, "/user/register")
        .methods("POST"_method)
        ([&](const request& req) {
            auto x = load(req.body);
            if (!x) {
                return response(400, "Unable to load/parse JSON.");
            }
            auto username = x["username"].s();
            auto password = x["password"].s();
            auto alreadyRegistered = db->users->get_where(
                "username",
                username).size() > 0;
            // we dont want empty usernames and passwords
            if (username.size() > 0 &&
                password.size() > 0 &&
                !alreadyRegistered) {
                auto userid = db->users->add({
                    "username",
                    "password",
                    "state"}, {
                    username,
                    password,
                    db_int(TRAINING)});
                auto userdataid = db->userdata->add({
                    "agegroup",
                    "occupation",
                    "userId"}, {
                    "18-24",
                    "school",
                    db_int(userid)});
                db->users->modify(userid, {"userdataId"}, {db_int(userdataid)});
                DefaultSettings(userid);
                return response(200, "Successfully registered!");
            } else {
                return response(400, "Username already taken!");
            }
        });
        CROW_ROUTE(app, "/user/data/update")
        .methods("POST"_method)
        ([&](const request& req) {
            auto x = load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x) {
                return response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto data = z.at("data");
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto key = i.key();
                    auto value = i.value().front().get<string>();
                    db->userdata->modify(userid, {key}, {value});
                }
                return crow::response(200, "Successfully updated user data.");
            } else {
                return crow::response(403,
                    "Token does not match expected value!");
            }
        });
        CROW_ROUTE(app, "/journals/new")
        .methods("POST"_method)
        ([&](const request& req) {
            auto x = load(req.body);
            nlohmann::json z = nlohmann::json::parse(req.body);
            if (!x) {
                return response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<string>();
            auto comment = z["comment"].get<string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto data = z.at("data").items();
                auto journalid = db->journals->add({
                    "comment",
                    "userId"}, {
                    comment,
                    db_int(userid)});
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto questionId = i.value().back().get<string>();
                    auto answer = i.value().front().get<string>();
                    db->answers->add({
                        "answer",
                        "journalId",
                        "questionId"}, {
                        answer,
                        db_int(journalid), questionId});
                }
                return response(200, "Successfully created new journal.");
            } else {
                return response(403,
                    "Token does not match expected value!");
            }
        });
        // journal id
        CROW_ROUTE(app, "/journals/get/<int>")
        ([&](int jid) {
            wvalue x({});
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
            wvalue x({});
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
        ([&](int userid, int journalid, string token) {
            if (authedUsers[userid] == token) {
                db->journals->delete_item(journalid);
                return response(202, "Successfully deleted journal.");
            } else {
                return response(403,
                    "Token does not match expected value!");
            }
        });
        CROW_ROUTE(app, "/answers/get/<int>/<string>")(
            [&](int answerid, string token){
            wvalue x({});
            if (answerid < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            auto answer = db->answers->get(answerid);
            auto allowedToGetAnswer = authedUsers[stoi(db->journals->get(
                stoi(answer["journalId"]))["userId"])] == token;
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
            vector<wvalue> vec;
            wvalue x({});
            if (userid < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            auto settings = db->settings->get_where(
                "userId",
                db_int(userid));
            for (auto setting : settings) {
                wvalue z({});
                auto row = db-> settings->get(setting);
                for (auto key : row.keys()) {
                    z[key] = row[key];
                }
                vec.push_back(z);
            }
            wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/settings/update")
        .methods("POST"_method)
        ([&](const request& req) {
            auto x = json::load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x) {
                return response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto data = z.at("settings");
                auto userSettings = db->settings->get_where(
                    "userId",
                    db_int(userid));
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto key = i.key();
                    auto value = i.value().front().get<string>();
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
                return response(200, "Successfully updated settings.");
            } else{
                return response(403,
                    "Token does not match expected value!");
            }
        });
        CROW_ROUTE(app, "/questions/defaults")
        ([&]() {
            auto questions = db->questions->get_where("tags", "default");
            vector<wvalue> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                wvalue x({});
                for (auto key : question.keys()) {
                    x[key] = question[key];
                }
                vec.push_back(x);
            }
            wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/questions/get/<string>")
        ([&](string tag) {
            auto questions = db->questions->get_where("tags", tag);
            vector<wvalue> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                wvalue x({});
                for (auto key : question.keys()) {
                    x[key] = question[key];
                }
                vec.push_back(x);
            }
            wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/predictions/get/<int>/<string>")
        ([&](int uid, string token) {
            if (uid < 0) {
                return wvalue({{"error", "Invalid id"}});
            }
            vector<wvalue> vec;
            auto predictions = db->predictions->get_where(
                "userId",
                db_int(uid));
            if (authedUsers[uid] == token) {
                for (auto prediction : predictions) {
                    auto row = db->predictions->get(prediction);
                    wvalue x({});
                    for (auto key : row.keys()) {
                        x[key] = row[key];
                    }
                    vec.push_back(x);
                }
            } else {
                return wvalue({{"error",
                    "Not allowed to access other users' predictions!"}});
            }
            wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/predictions/add")
        .methods("POST"_method)([&](const request& req){
            auto x = load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x) {
                return response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<string>();
            auto qid = z["questionid"].get<string>();
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
                        stoi(db->answers->get(answer)["journalId"])) > 0) {
                        prediction.add_valued_data(qid,
                            stod(db->answers->get(answer)["answer"]));
                    }
                }
                auto predictionValue = prediction.build();
                db->predictions->add({
                    "userId",
                    "value"}, {
                    db_int(userid),
                    format("{}", predictionValue)});
                return response(200, "Successfully added prediction");
            } else {
                return response(403,
                    "Token does not match expected value!");
            }
        });

        app.port(port).multithreaded().run();
    }


 private:
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
    int64_t Hash(string username, string password) {
        auto hash1 = hash<string>{}(username);
        auto hash2 = hash<string>{}(password);
        auto combinedhash = hash1 ^ (hash2 << 1);
        return combinedhash;
    }
    int UserIdFromToken(string token) {
        for (auto user : authedUsers) {
            if (user.second == token) {
                return user.first;
            }
        }
        return 0;
    }
    bool SettingExists(vector<int> ids, string key) {
        for (auto id : ids) {
            auto row = db->settings->get(id);
            if (row["key"] == key) {
                return true;
            }
        }
        return false;
    }
};

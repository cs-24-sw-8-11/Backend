#pragma once

#include <httplib.h>
#include <memory>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#include "Database.hpp"
#include "PredictionManager.hpp"

using namespace std;
using namespace httplib;
using namespace nlohmann;

map<int, string> authedUsers;

class Route {
 protected:
    shared_ptr<Database> db;
    shared_ptr<Server> server;
    PredictionManager manager;

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

    int UserIdFromToken(string token) {
        for (auto [uid, utoken] : authedUsers) {
            if (utoken == token) {
                return uid;
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

    void respond(Response* response, json data, int status = 200) {
        response->status = status;
        response->set_content(to_string(data), "application/json");
    }

    void respond(Response* response, string data, int status = 200) {
        response->status = status;
        response->set_content(data, "text/plain");
    }


 public:
    Route(Database db, shared_ptr<Server> server) {
        this->db = make_shared<Database>(db);
        this->server = server;
    }
    virtual void init() {
        cerr << "Error, called unimplemented run method" << endl;
        throw exception();
    }
};

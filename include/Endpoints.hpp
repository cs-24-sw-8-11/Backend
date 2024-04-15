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

class Endpoint {
    public:
        crow::SimpleApp app;
        std::map<int, std::string>* authedUsers;
        std::shared_ptr<Database> db;
        std::shared_ptr<PredictionManager> manager;

        explicit Endpoint(crow::SimpleApp* App, std::map<int, std::string>* AuthedUsers, std::shared_ptr<Database> DB, std::shared_ptr<PredictionManager> Manager) {
            app = App;
            authedUsers = AuthedUsers;
            db = DB;
            manager = Manager;
        }

    protected:
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
};
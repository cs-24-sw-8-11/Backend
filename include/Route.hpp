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

/// @brief Key/Value pair that maps authorized user ids to their token.
map<int, string> authedUsers;

/// @brief Super class for managing all the endpoints of the API.
class Route {
 protected:
    Database db;
    shared_ptr<Server> server;
    PredictionManager manager;

    /// @brief Populates the settings table with default settings for a given user.
    /// @param userId
    void default_settings(int userId) {
        db["settings"].add({
            "key",
            "value",
            "userId"}, {
            "predictions",
            "true",
            to_string(userId)});
    }

    /// @brief Returns the user id from the authedUsers dictionary from a given token.
    /// @param token
    /// @return User id from token.
    int user_id_from_token(string token) {
        for (auto [uid, utoken] : authedUsers) {
            if (utoken == token) {
                return uid;
            }
        }
        return 0;
    }

    /// @brief Checks whether a setting with a given key exists with a given list of setting ids.
    /// @param ids
    /// @param key
    /// @return Whether the setting exists or not.
    bool setting_exists(vector<int> ids, string key) {
        for (auto id : ids) {
            auto row = db["settings"].get(id);
            if (row["key"] == key) {
                return true;
            }
        }
        return false;
    }

    /// @brief Wrapper around response.status and response.set_content, used to determine content-type via overloading
    /// @param response
    /// @param data
    /// @param status
    void respond(Response* response, json data, int status = 200) {
        response->status = status;
        response->set_content(to_string(data), "application/json");
    }

    /// @brief Wrapper around response.status and response.set_content, used to determine content-type via overloading
    /// @param response
    /// @param data
    /// @param status
    void respond(Response* response, string data, int status = 200) {
        response->status = status;
        response->set_content(data, "text/plain");
    }


 public:
    /// @brief Constructs a route object with references to the http server and the database.
    /// @param db
    /// @param server
    Route(Database db, shared_ptr<Server> server) {
        this->db = db;
        this->server = server;
    }

    /// @brief Template virtual method that is overwritten in each route to add all their endpoints.
    virtual void init() {
        cerr << "Error, called unimplemented run method" << endl;
        throw exception();
    }
};

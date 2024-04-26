#pragma once

#include <string>
#include <vector>

#include "Route.hpp"

#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

/// @brief Hashing function that makes a combined hash of a username and password.
/// @param username
/// @param password
/// @return Hash of username to the power of password bitshifted by 1.
int64_t make_hash(std::string username, std::string password) {
    auto hash1 = std::hash<std::string>{}(username);
    auto hash2 = std::hash<std::string>{}(password);
    auto combinedhash = hash1 ^ (hash2 << 1);
    return combinedhash;
}

/// @brief This class contains all of the endpoints related to users.
class Users : public Route {
 public:
    // Inherit the super class constructor.
    using Route::Route;
    /// @brief Initializes the User endpoints.
    void init() override {
        /// @brief Gets the userdata of a user.
        this->server->Get("/user/get/:token", [&](Request request, Response& response){
            json response_data;
            auto token = request.path_params["token"];
            auto uid = user_id_from_token(token);
            if (uid <= 0) {
                response_data["error"] = "Invalid Token!";
                respond(&response, response_data, 400);
                return;
            }
            auto user = db->users->get(uid);
            auto userdata_ids = db->userdata->get_where("userId", db_int(uid));
            if (userdata_ids.size() == 0) {
                response_data["error"] = "User has no userdata yet!";
                respond(&response, response_data, 400);
                return;
            }
            auto userdata = db->userdata->get(userdata_ids[0]);
            for (auto key : user.keys()) {
                if (key == "password")
                    continue;

                response_data[key] = user[key];
            }
            for (auto key : userdata.keys())
                response_data[key] = userdata[key];

            respond(&response, response_data);
        });
        /// @brief Get a range of user ids with a given min and max value.
        this->server->Get("/user/ids/:min/:max", [&](Request request, Response& response){
            json response_data;
            auto min = stoi(request.path_params["min"]);
            auto max = stoi(request.path_params["max"]);
            if (min < 0 || max < 0 || (max-min < 0)) {
                response_data["difference"] = max-min;
                response_data["max"] = max;
                response_data["min"] = min;
                response_data["error"] = "Invalid Range.";
                return respond(&response, response_data);
            }
            auto users = db->users->get_where();
            vector<int> filteredUsers;
            for (const auto & user : users) {
                if (user <= max && user >= min)
                    filteredUsers.push_back(user);
            }
            response_data = filteredUsers;
            respond(&response, response_data);
        });
        /// @brief Gets all user ids.
        this->server->Get("/user/ids", [&](Request request, Response& response){
            json data = db->users->get_where();
            respond(&response, data);
        });
        /// @brief Authenticate a user.
        this->server->Post("/user/auth", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto username = body["username"].get<string>();
            auto password = body["password"].get<string>();
            auto hash = make_hash(username, password);
            auto token = std::format("{}", hash);
            if (db->users->get_where("username", username).size() == 0) {
                return respond(&response, string("Invalid Credentials!"), 403);
            }
            auto dbpassword = db->users->get(db->users->get_where(
                "username",
                username).front())["password"];

            if (dbpassword == token) {
                auto userid = db->users->get_where(
                    "username",
                    username).front();
                authedUsers[userid] = token;
                respond(&response, token);
            } else {
                respond(&response, string("Invalid Credentials!"), 403);
            }
        });
        /// @brief Register a new user into the system.
        this->server->Post("/user/register", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto username = body["username"].get<string>();
            auto password = body["password"].get<string>();
            auto alreadyRegistered = db->users->get_where(
                "username",
                username).size() > 0;
            // we dont want empty usernames and passwords
            if (username.size() > 0 &&
                password.size() > 0 &&
                !alreadyRegistered) {
                auto hash = make_hash(username, password);
                auto token = std::format("{}", hash);
                auto userid = db->users->add({
                    "username",
                    "password",
                    "state"}, {
                    username,
                    token,
                    db_int(TRAINING)});
                default_settings(userid);
                respond(&response, string("Successfully Registered!"));
            } else {
                respond(&response, string("Username is already taken!"), 400);
            }
        });
        /// @brief Updates or creates a user's userdata with new values.
        this->server->Post("/user/data/update", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<std::string>();
            auto uid = user_id_from_token(token);
            if (authedUsers[uid] == token) {
                auto data = body["data"];
                if (db->userdata->get_where("userId", db_int(uid)).size() == 0) {
                    db->userdata->add({
                        "agegroup",
                        "major",
                        "userId"}, {
                        data["agegroup"].get<string>(),
                        data["major"].get<string>(),
                        db_int(uid)});
                } else {
                    for (auto [key, value] : data.items()) {
                        db->userdata->modify(uid, {key}, {value});
                    }
                }
                respond(&response, string("Successfully updated user data."));
            } else {
                respond(&response, string("Token does not match expected value!"), 403);
            }
        });
    }
};

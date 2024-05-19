#pragma once

#include <string>

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace httplib;
using namespace nlohmann;

class Settings : public Route {
    // Inherits the super class constructor.
    using Route::Route;

 public:
    /// @brief Initializes the Settings endpoints.
    void init() override {
        /// @brief Returns all the settings belonging to a given user.
        Get("/settings/get/:token", [&](Request request, Response& response){
            auto token = request.path_params["token"];
            auto uid = user_id_from_token(token);
            json response_data;
            if (uid < 0) {
                response_data["error"] = "Invalid Token!";
                return respond(&response, response_data, 400);
            }
            auto settings = db["settings"].get_where(
                "userId",
                uid);
            for (auto setting : settings) {
                json data;
                auto row = db["settings"].get(setting);
                data = row;
                response_data.push_back(data);
            }
            respond(&response, response_data);
        });
        /// @brief Updates all the specified settings for a given user.
        Post("/settings/update", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<std::string>();
            auto uid = user_id_from_token(token);
            if (authedUsers[uid] == token) {
                auto data = body["settings"].get<map<string, string>>();
                auto userSettings = db["settings"].get_where(
                    "userId",
                    to_string(uid));
                for (auto [key, value] : data) {
                    if (!setting_exists(userSettings, key)) {
                        db["settings"].add({
                            {"key", key},
                            {"value", value},
                            {"userId", to_string(uid)}
                        });
                    }
                    for (auto setting : userSettings) {
                        auto settingsRow = db["settings"].get(setting);
                        if (settingsRow["key"] == key) {
                            db["settings"].modify(setting, {"value"}, {value});
                        }
                    }
                }
                respond(&response, string("Successfully updated settings"));
            } else {
                respond(&response, string("Token does not match expected value!"), 403);
            }
        }, {"token", "settings"});
    }
};

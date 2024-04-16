#pragma once

#include <string>

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace httplib;
using namespace nlohmann;

class Settings : public Route {
    using Route::Route;

 public:
    void init() override {
        this->server->Get("/settings/get/:uid", [&](Request request, Response& response){
            auto uid = stoi(request.path_params["uid"]);
            json response_data;
            if (uid < 0) {
                response_data["error"] = "Invalid Id.";
                return respond(&response, response_data, 400);
            }
            auto settings = db->settings->get_where(
                "userId",
                db_int(uid));
            for (auto setting : settings) {
                json data;
                auto row = db-> settings->get(setting);
                for (auto key : row.keys()) {
                    data[key] = row[key];
                }
                response_data.push_back(data);
            }
            respond(&response, response_data);
        });
        this->server->Post("/settings/update", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<std::string>();
            auto uid = UserIdFromToken(token);
            if (authedUsers[uid] == token) {
                auto data = body["settings"].get<map<string, string>>();
                auto userSettings = db->settings->get_where(
                    "userId",
                    db_int(uid));
                for (auto [key, value] : data) {
                    if (!SettingExists(userSettings, key)) {
                        db->settings->add({
                            "key",
                            "value",
                            "userId"}, {
                            key,
                            value,
                            db_int(uid)});
                    }
                    for (auto setting : userSettings) {
                        auto settingsRow = db->settings->get(setting);
                        if (settingsRow["key"] == key) {
                            db->settings->modify(setting, {"value"}, {value});
                        }
                    }
                }
                respond(&response, string("Successfully updated settings"));
            } else {
                respond(&response, string("Token does not match expected value!"), 403);
            }
        });
    }
};

#pragma once
#include <crow.h>

#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

#include "Endpoints.hpp"

class Settings : public Endpoint {
    public:
    explicit Settings() {
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
                return crow::response(403, "Token does not match expected value!");
            }
        });
    }
};
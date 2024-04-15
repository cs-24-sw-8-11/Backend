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

class Users : public Endpoint {
    public:
        explicit Users() {
            CROW_ROUTE(app, "/user/get/<int>")([&](int id) {
            crow::json::wvalue x({});
            auto user = db->users->get(id);
            auto userdata = db->userdata->get(db->userdata->get_where("userId", db_int(id)).front());
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
            crow::json::wvalue x;
            if (min < 0 || max < 0 || (max - min < 0)) {
                x["difference"] = max - min;
                x["max"] = max;
                x["min"] = min;
                x["error"] = "Invalid Range";
                return x;
            }
            auto users = db->users->get_where();
            std::vector<int> filteredUsers;
            for (const auto& user : users) {
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
            crow::json::wvalue x;
            auto users = db->users->get_where();
            x = std::move(users);
            return std::move(x);
        });
        CROW_ROUTE(app, "/user/auth")
            .methods("POST"_method)([&](const crow::request& req) {
                auto x = crow::json::load(req.body);
                if (!x) {
                    return crow::response(400, "Unable to load/parse json");
                }
                auto username = x["username"].s();
                auto password = x["password"].s();
                if (db->users->get_where("username", username).size() == 0) {
                    return crow::response(403, "Invalid Credentials!");
                }
                auto dbpassword = db->users->get(db->users->get_where("username",username).front())["password"];

                if (dbpassword == password) {
                    auto userid = db->users->get_where("username", username).front();
                    auto hash = Hash((std::string)username, (std::string)password);
                    auto token = std::format("{}", hash);
                    authedUsers[userid] = token;
                    return crow::response(200, token);
                } else {
                    return crow::response(403, "Invalid Credentials!");
                }
            });
        CROW_ROUTE(app, "/user/register")
            .methods("POST"_method)([&](const crow::request& req) {
                auto x = crow::json::load(req.body);
                if (!x) {
                    return crow::response(400, "Unable to load/parse JSON.");
                }
                auto username = x["username"].s();
                auto password = x["password"].s();
                auto alreadyRegistered = db->users->get_where("username", username).size() > 0;
                // we dont want empty usernames and passwords
                if (username.size() > 0 &&
                    password.size() > 0 &&
                    !alreadyRegistered) {
                    auto userid = db->users->add({"username",
                                                  "password",
                                                  "state"},
                                                 {username,
                                                  password,
                                                  db_int(TRAINING)});
                    auto userdataid = db->userdata->add({"agegroup",
                                                         "occupation",
                                                         "userId"},
                                                        {"18-24",
                                                         "school",
                                                         db_int(userid)});
                    db->users->modify(userid, {"userdataId"}, {db_int(userdataid)});
                    DefaultSettings(userid);
                    return crow::response(200, "Successfully registered!");
                } else {
                    return crow::response(400, "Username is already taken!");
                }
            });
        CROW_ROUTE(app, "/user/data/update")
            .methods("POST"_method)([&](const crow::request& req) {
                auto x = crow::json::load(req.body);
                auto z = nlohmann::json::parse(req.body);
                if (!x) {
                    return crow::response(400, "Unable to load/parse JSON.");
                }
                auto token = z["token"].get<std::string>();
                auto userid = UserIdFromToken(token);
                if (authedUsers[userid] == token) {
                    auto data = z.at("data");
                    for (auto i = data.begin(); i != data.end(); ++i) {
                        auto key = i.key();
                        auto value = i.value().front().get<std::string>();
                        db->userdata->modify(userid, {key}, {value});
                    }
                    return crow::response(200, "Successfully updated user data.");
                } else {
                    return crow::response(403, "Token does not match expected value!");
                }
            });
        }
};
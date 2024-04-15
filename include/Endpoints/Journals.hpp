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

class Journals : public Endpoint {
    public:
    explicit Journals() {
        CROW_ROUTE(app, "/journals/new")
            .methods("POST"_method)([&](const crow::request& req) {
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
                    auto journalid = db->journals->add({"comment",
                                                        "userId"},
                                                       {comment,
                                                        db_int(userid)});
                    for (auto i = data.begin(); i != data.end(); ++i) {
                        auto questionId = i.value().back().get<std::string>();
                        auto answer = i.value().front().get<std::string>();
                        db->answers->add({"answer",
                                          "journalId",
                                          "questionId"},
                                         {answer,
                                          db_int(journalid), questionId});
                    }
                    return crow::response(200, "Successfully created new journal.");
                } else {
                    return crow::response(403, "Token does not match expected value!");
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
            .methods("DELETE"_method)([&](int userid, int journalid, std::string token) {
                if (authedUsers[userid] == token) {
                    db->journals->delete_item(journalid);
                    return crow::response(202, "Successfully deleted journal.");
                } else {
                    return crow::response(403, "Token does not match expected value!");
                }
            });
    }
};
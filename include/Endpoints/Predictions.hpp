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

class Predictions : public Endpoint {
    public: 
    explicit Predictions() {
        CROW_ROUTE(app, "/predictions/get/<int>/<string>")
        ([&](int uid, std::string token) {
            if (uid < 0) {
                return crow::json::wvalue({{"error", "Invalid id"}});
            }
            std::vector<crow::json::wvalue> vec;
            auto predictions = db->predictions->get_where(
                "userId",
                db_int(uid));
            if (authedUsers[uid] == token) {
                for (auto prediction : predictions) {
                    auto row = db->predictions->get(prediction);
                    crow::json::wvalue x({});
                    for (auto key : row.keys()) {
                        x[key] = row[key];
                    }
                    vec.push_back(x);
                }
            } else {
                return crow::json::wvalue({{"error", "Not allowed to access other users' predictions!"}});
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/predictions/add")
        .methods("POST"_method)([&](const crow::request& req){
            auto x = crow::json::load(req.body);
            auto z = nlohmann::json::parse(req.body);
            if (!x) {
                return crow::response(400, "Unable to load/parse JSON.");
            }
            auto token = z["token"].get<std::string>();
            auto qid = z["questionid"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto prediction = manager->create_new_prediction(userid);
                auto answers = db->answers->get_where("questionId", qid);
                auto journals = db->journals->get_where(
                    "userId",
                    db_int(userid));
                for (auto answer : answers) {
                    if (std::count(
                        journals.begin(),
                        journals.end(),
                        std::stoi(db->answers->get(answer)["journalId"])) > 0) {
                        prediction.add_valued_data(qid, std::stod(db->answers->get(answer)["answer"]));
                    }
                }
                auto predictionValue = prediction.build();
                db->predictions->add({
                    "userId",
                    "value"}, {
                    db_int(userid),
                    std::format("{}", predictionValue)});
                return crow::response(200, "Successfully added prediction");
            } else {
                return crow::response(403, "Token does not match expected value!");
            }
        });
    }
};
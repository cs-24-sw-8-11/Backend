#pragma once

#include <string>
#include <vector>
#include <utility>

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

#define DAY 86400
#define WEEK 604800

class Predictions : public Route {
    // Inherits the super class constructor.
    using Route::Route;

 public:
    /// @brief Initializes the Prediction endpoints.
    void init() override {
        /// @brief Returns all predictions from a specific user.
        this->server->Get("/predictions/get/:token", [&](Request request, Response& response){
            auto token = request.path_params["token"];
            auto uid = user_id_from_token(token);
            json response_data;
            vector<json> result;
            auto predictions = db->predictions->get_where(
                "userId",
                uid);
            if (authedUsers[uid] == token) {
                for (auto prediction : predictions) {
                    auto row = db->predictions->get(prediction);
                    json data;
                    for (auto key : row.keys()) {
                        data[key] = row[key];
                    }
                    result.push_back(data);
                }
            } else {
                response_data["error"] = "Invalid Token!";
                return respond(&response, response_data, 403);
            }
            response_data = result;
            respond(&response, response_data);
        });
        /// @brief Submits a prediction to the system.
        this->server->Post("/predictions/add", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<string>();
            auto uid = user_id_from_token(token);
            json response_data;
            if (authedUsers[uid] == token) {
                auto jids = db->journals->get_where("userId", uid);
                map<int, Row> journals;
                auto now = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
                for (auto jid : jids) {
                    auto journal = db->journals->get(jid);
                    auto journal_time = stoi(journal["timestamp"]);
                    auto delta = now - journal_time;
                    if (delta <= WEEK) {
                        // This journal is within the valid range
                        journals[delta/DAY] = journal;
                    } else {
                        cout << "\033[38;2;255;0;0minvalid journal found\033[0m" << endl;
                    }
                }
                if (journals.size() < 3) {
                    response_data["error"] = "Too few journals!";
                    respond(&response, response_data, 403);
                    return;
                }
                PredictionManager manager;
                auto builder = manager.create_new_prediction(uid);
                for (auto [timestamp, journal] : journals) {
                    auto jid = journal["id"];
                    vector<pair<double, double>> prediction_data;
                    for (auto aid : db->answers->get_where("journalId", jid)) {
                        auto answer = db->answers->get(aid);
                        prediction_data.push_back(make_pair(stod(answer["value"]), stod(answer["rating"])));
                    }
                    builder.add_journal(timestamp, prediction_data);
                }
                auto result = builder.build();
                response_data["value"] = result;
                respond(&response, response_data);
            } else {
                response_data["error"] = "Token does not match expected value!";
                respond(&response, response_data, 403);
            }
        });
    }
};

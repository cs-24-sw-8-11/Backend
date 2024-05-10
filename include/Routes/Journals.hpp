#pragma once

#include <chrono>
#include <vector>
#include <string>
#include "Route.hpp"
#include <nlohmann/json.hpp>

#include "Utils.hpp"


using namespace httplib;
using namespace nlohmann;
using namespace std;

class Journals : public Route {
    // Inherit the super class constructor.
    using Route::Route;

    /// @brief Initializes the Journal endpoints.
    void init() override {
        /// @brief Submits a new journal to the system
        this->server->Post("/journals/new", [&](Request request, Response& response){
            auto time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
            auto body = json::parse(request.body);
            auto token = body["token"].get<string>();
            auto userid = user_id_from_token(token);
            if (authedUsers[userid] == token) {
                auto list = body["data"].get<vector<json>>();
                auto jid = db->journals->add({
                    "timestamp",
                    "userId"}, {
                    to_string(time),
                    to_string(userid)});
                for (auto entry : list) {
                    auto qid = entry["qid"].get<string>();
                    auto meta = entry["meta"].get<string>();
                    auto rating = entry["rating"].get<int>();
                    // run sentiment analysis on answer
                    auto result = P8::run_cmd(format("python ./lib/datasets/sentiment_analysis.py \"{}\"", meta))["stdout"];
                    db->answers->add({
                        "value",
                        "rating",
                        "journalId",
                        "questionId"}, {
                        result,
                        to_string(static_cast<double>(rating)/5.0),
                        to_string(jid),
                        qid});
                }
                respond(&response, string("Successfully created new journal."));
            } else {
                respond(&response, string("Token does not match expected value!"), 403);
            }
        });
        /// @brief Returns a journal with a given journal id.
        this->server->Get("/journals/get/:jid", [&](Request request, Response& response){
            auto jid = stoi(request.path_params["jid"]);
            json response_data;
            if (jid <= 0 || db->journals->get_where("id", jid).size() == 0) {
                response_data["error"] = "Invalid Journal Id.";
                return respond(&response, response_data, 400);
            }
            auto journal = db->journals->get(jid);
            for (auto key : journal.keys()) {
                response_data[key] = journal[key];
            }
            response_data["answers"] = db->answers->get_where(
                "journalId",
                jid);
            respond(&response, response_data);
        });
        /// @brief Returns all the journal ids belonging to a specific user.
        this->server->Get("/journals/ids/:token", [&](Request request, Response& response){
            json response_data;
            auto token = request.path_params["token"];
            auto uid = user_id_from_token(token);
            if (uid <= 0) {
                response_data["error"] = "Invalid Token!";
                return respond(&response, response_data, 400);
            }
            auto journals = db->journals->get_where(
                "userId",
                uid);
            response_data = journals;
            respond(&response, response_data);
        });
        /// @brief Deletes a journal from the system with a given journal id.
        this->server->Delete("/journals/delete/:jid/:token", [&](Request request, Response& response){
            auto token = request.path_params["token"];
            auto uid = user_id_from_token(token);
            auto jid = stoi(request.path_params["jid"]);
            if (authedUsers[uid] == token) {
                db->journals->delete_item(jid);
                respond(&response, string("Successfully deleted journal."), 202);
            } else {
                respond(&response, string("Token does not match expected value!"), 403);
            }
        });
    }
};

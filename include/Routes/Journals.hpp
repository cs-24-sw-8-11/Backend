#pragma once

#include <chrono>
#include <vector>
#include <string>
#include "Route.hpp"
#include <nlohmann/json.hpp>
#include <thread>

#include "Utils.hpp"


using namespace httplib;
using namespace nlohmann;
using namespace std;
using namespace P8;

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
                auto jid = db["journals"].add({
                    {"timestamp", to_string(time)},
                    {"userId", to_string(userid)}
                });
                for (auto entry : list) {
                    auto qid = entry["qid"].get<string>();
                    auto question = db["questions"].get(stoi(qid))["question"];
                    auto meta = entry["meta"].get<string>();
                    auto rating = entry["rating"].get<string>();
                    // run sentiment analysis on answer
                    increment_running(userid);
                    sentiment_threads[userid].push_back(thread([meta, this, rating, jid, qid, userid, question]() {
                        auto analysis = run_cmd(format("python ./lib/datasets/sentiment_analysis.py \"{}\" \"{}\"", meta, question));
                        auto sa_stdout = split_string(analysis["stdout"], "\n");
                        if (sa_stdout.size() < 2) {
                            decrement_running(userid);
                            return;
                        }
                        auto meta_value = sa_stdout[0];
                        auto question_value = sa_stdout[1];
                        auto final_value = mean({stod(question_value), stod(meta_value)});
                        db["answers"].add({
                            {"value", to_string(final_value)},
                            {"rating", to_string(stod(rating)/5.0)},
                            {"journalId", to_string(jid)},
                            {"questionId", qid}
                        });
                        decrement_running(userid);
                    }));
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
            if (jid <= 0 || db["journals"].get_where("id", jid).size() == 0) {
                response_data["error"] = "Invalid Journal Id.";
                return respond(&response, response_data, 400);
            }
            auto journal = db["journals"].get(jid);
            response_data = journal;

            response_data["answers"] = db["answers"].get_where(
                "journalId",
                jid);
            respond(&response, response_data);
        });
        /// @brief Returns all the journal ids belonging to a specific user.
        this->server->Get("/journals/ids/:token", [&](Request request, Response& response){
            json response_data;
            auto token = request.path_params["token"];
            auto uid = user_id_from_token(token);

            // check if threads are running for the user:
            if(get_running(uid))
                for(auto& thread : sentiment_threads[uid]){
                    thread.join();
                }

            if (uid <= 0) {
                response_data["error"] = "Invalid Token!";
                return respond(&response, response_data, 400);
            }
            auto journals = db["journals"].get_where(
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
                db["journals"].delete_item(jid);
                respond(&response, string("Successfully deleted journal."), 202);
            } else {
                respond(&response, string("Token does not match expected value!"), 403);
            }
        });
    }
};

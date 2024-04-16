#pragma once

#include <vector>
#include <string>
#include "Route.hpp"
#include <nlohmann/json.hpp>


using namespace httplib;
using namespace nlohmann;
using namespace std;

class Journals : public Route {
    using Route::Route;
    void init() override {
        this->server->Post("/journals/new", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<string>();
            auto comment = body["comment"].get<string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto list = body["data"].get<vector<json>>();
                auto jid = db->journals->add({
                    "comment",
                    "userId"}, {
                    comment,
                    db_int(userid)});
                for (auto entry : list) {
                    auto qid = entry["question"].get<string>();
                    auto answer = entry["answer"].get<string>();
                    db->answers->add({
                        "answer",
                        "journalId",
                        "questionId"}, {
                        answer,
                        db_int(jid), qid});
                }
                respond(&response, string("Successfully created new journal."));
            } else {
                respond(&response, string("Token does not match expected value"), 403);
            }
        });
        this->server->Get("/journals/get/:jid", [&](Request request, Response& response){
            auto jid = stoi(request.path_params["jid"]);
            json response_data;
            if (jid < 0) {
                response_data["error"] = "Invalid id";
                return respond(&response, response_data, 400);
            }
            auto journal = db->journals->get(jid);
            for (auto key : journal.keys()) {
                response_data[key] = journal[key];
            }
            response_data["answers"] = db->answers->get_where(
                "journalId",
                db_int(jid));
            respond(&response, response_data);
        });
        this->server->Get("/journals/ids/:uid", [&](Request request, Response& response){
            json response_data;
            auto uid = stoi(request.path_params["uid"]);
            if (uid < 0) {
                response_data["error"] = "Invalid id";
                return respond(&response, response_data, 400);
            }
            auto journals = db->journals->get_where(
                "userId",
                db_int(uid));
            response_data = journals;
            respond(&response, response_data);
        });
        this->server->Delete("/journals/delete/:uid/:jid/:token", [&](Request request, Response& response){
            auto uid = stoi(request.path_params["uid"]);
            auto jid = stoi(request.path_params["jid"]);
            auto token = request.path_params["token"];
            if (authedUsers[uid] == token) {
                db->journals->delete_item(jid);
                respond(&response, string("Successfully deleted journal."), 202);
            } else {
                respond(&response, string("Token does not match expected value!"), 403);
            }
        });
    }
};

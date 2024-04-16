#pragma once

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace httplib;
using namespace nlohmann;

class Answers : public Route {
    using Route::Route;

 public:
    void init() override {
        this->server->Get("/answers/get/:answerId/:token", [&](Request request, Response& response){
            auto answerId = stoi(request.path_params["answerId"]);
            auto token = request.path_params["token"];
            json response_data;
            if (answerId < 0) {
                response_data["error"] = "Invalid Id.";
                return respond(&response, response_data);
            }
            auto answer = db->answers->get(answerId);
            auto allowedToGetAnswer = authedUsers[stoi(db->journals->get(
                stoi(answer["journalId"]))["userId"])] == token;
            if (allowedToGetAnswer) {
                for (auto key : answer.keys()) {
                    response_data[key] = answer[key];
                }
            } else {
                response_data["error"] = "Not allowed to access other users' answers!";
            }
            return respond(&response, response_data);
        });
    }
};

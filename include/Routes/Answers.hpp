#pragma once

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace httplib;
using namespace nlohmann;

class Answers : public Route {
    // Inherits the super class constructor.
    using Route::Route;

 public:
    /// @brief Initializes the Answers endpoints.
    void init() override {
        /// @brief Returns a specific answer provided that the user has permission to access it.
        Get("/answers/get/:answerId/:token", [&](Request request, Response& response){
            auto answerId = stoi(request.path_params["answerId"]);
            auto token = request.path_params["token"];
            json response_data;
            if (answerId <= 0 || db["answers"].get_where("id", answerId).size() == 0) {
                response_data["error"] = "Invalid Answer Id.";
                return respond(&response, response_data, 400);
            }
            auto answer = db["answers"].get(answerId);
            auto allowedToGetAnswer = authedUsers[stoi(db["journals"].get(
                stoi(answer["journalId"]))["userId"])] == token;
            if (allowedToGetAnswer) {
                response_data = answer;
            } else {
                response_data["error"] = "Not allowed to access other users' answers!";
            }
            return respond(&response, response_data);
        });
    }
};

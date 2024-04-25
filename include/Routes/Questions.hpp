#pragma once

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

class Questions : public Route {
    // Inherits the super class constructor.
    using Route::Route;

 public:
    /// @brief Adds the Questions endpoints.
    void init() override {
        /// @brief Returns the default questions (those with the tag "default").
        this->server->Get("/questions/defaults", [&](Request request, Response& response){
            if (db->questions->get_where("tags", "default").size() == 0) {
                json response_data;
                response_data["error"] = "Somehow there are no default questions???";
                return respond(&response, response_data, 400);
            }
            auto questions = db->questions->get_where("tags", "default");
            auto response_data = json::array();
            for (auto i : questions) {
                auto question = db->questions->get(i);
                json data;
                for (auto key : question.keys()) {
                    data[key] = question[key];
                }
                response_data.push_back(data);
            }
            respond(&response, response_data);
        });
        /// @brief Returns all the questions with a given tag.
        this->server->Get("/questions/get/:tag", [&](Request request, Response& response){
            auto tag = request.path_params["tag"];
            if (db->questions->get_where("tags", tag).size() == 0) {
                json response_data;
                response_data["error"] = "No Questions With that Tag.";
                return respond(&response, response_data, 400);
            }
            auto questions = db->questions->get_where("tags", tag);
            json response_data;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                json data;
                for (auto key : question.keys()) {
                    data[key] = question[key];
                }
                response_data.push_back(data);
            }
            respond(&response, response_data);
        });
    }
};

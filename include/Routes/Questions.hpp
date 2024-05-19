#pragma once

#include <vector>

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;
using namespace P8;

class Questions : public Route {
    // Inherits the super class constructor.
    using Route::Route;

 public:
    /// @brief Initializes the Questions endpoints.
    void init() override {
        /// @brief Returns the default questions (those with the tag "default").
        Get("/questions/defaults", [&](Request request, Response& response){
            if (db["questions"].get_where("tags", "default").size() == 0) {
                json response_data;
                response_data["error"] = "Somehow there are no default questions???";
                return respond(&response, response_data, 400);
            }
            auto questions = db["questions"].get_where("tags", "default");
            auto response_data = json::array();
            for (auto i : questions) {
                auto question = db["questions"].get(i);
                json data;
                data = question;
                response_data.push_back(data);
            }
            respond(&response, response_data);
        });
        /// @brief Returns all the questions with a given tag.
        Get("/questions/get/:tag", [&](Request request, Response& response){
            auto tag = request.path_params["tag"];
            json response_data;
            if (db["questions"].get_where("tags", tag).size() == 0) {
                auto qids = db["questions"].get_where();

                while (response_data.size() < 5) {
                    auto row = db["questions"].get(qids[randint(qids.size())]);
                    if (row["tags"] == "default")
                        continue;
                    response_data.push_back(row);
                }

                return respond(&response, response_data);
            }
            auto questions = db["questions"].get_where("tags", tag);
            for (auto i : questions) {
                auto question = db["questions"].get(i);
                json data;
                data = question;
                response_data.push_back(data);
            }
            respond(&response, response_data);
        });
        Get("/questions/legend/:qid", [&](Request request, Response& response){
            auto qid = request.path_params["qid"];
            auto lids = db["legends"].get_where("questionId", stoi(qid));
            vector<Row> legends;

            for (auto lid : lids)
                legends.push_back(db["legends"].get(lid));

            if (lids.size() == 0) {
                json data;
                data["0"] = "Default legend item";
                respond(&response, data);
                return;
            }

            json data;
            for (auto lid_data : legends) {
                auto index = lid_data["legend_index"];
                auto text = lid_data["text"];
                data[index] = text;
            }
            respond(&response, data);
        });
    };
};

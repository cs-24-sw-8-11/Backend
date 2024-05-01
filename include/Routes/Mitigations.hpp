#pragma once

#include <algorithm>
#include <vector>
#include <string>

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace httplib;
using namespace nlohmann;

class Mitigations : public Route {
    // Inherits the super class constructor.
    using Route::Route;

 public:
    /// @brief Initializes the Mitigations endpoints.
    void init() override {
        /// @brief Returns a specific mitigation with a given id.
        this->server->Get("/mitigations/get/:mid", [&](Request request, Response& response){
            auto mid = stoi(request.path_params["mid"]);
            json response_data;
            if (mid <= 0 || db->mitigations->get_where("id", db_int(mid)).size() == 0) {
                response_data["error"] = "Invalid Mitigation Id.";
                return respond(&response, response_data, 400);
            }
            auto mitigations = db->mitigations->get(mid);
            for (auto key : mitigations.keys()) {
                response_data[key] = mitigations[key];
            }
            respond(&response, response_data);
        });
        /// @brief Returns a specific mitigation with a given id.
        this->server->Get("/mitigations/tags/:tag", [&](Request request, Response& response){
            auto tagparam = request.path_params["tag"];
            auto tags = split(tagparam);
            json response_data;
            vector<int> mitigations;
            for (auto tag : tags) {
                auto mitigation = db->mitigations->get_where_like("tags", tag);

                for (auto m : mitigation) {
                    mitigations.push_back(m);
                }
            }
            if (mitigations.size() > 0) {
                sort(mitigations.begin(), mitigations.end());
                auto distinct = unique(mitigations.begin(), mitigations.end());
                mitigations.resize(distance(mitigations.begin(), distinct));

                for (distinct = mitigations.begin(); distinct != mitigations.end(); ++distinct) {
                    auto mitigation = db->mitigations->get(*distinct);
                    json data;
                    for (auto key : mitigation.keys()) {
                        data[key] = mitigation[key];
                    }
                    response_data.push_back(data);
                }
                respond(&response, response_data);
            } else {
                response_data["error"] = "No mitigations found with the given tag(s).";
                respond(&response, response_data, 400);
            }
        });
    }

 private:
    vector<string> split(string s){
        vector<string> res;
        int pos = 0;
        while (pos < s.size()) {
            pos = s.find(",");
            res.push_back(s.substr(0, pos));
            s.erase(0, pos+1);
        }
        return res;
    }
};

#pragma once

#include <algorithm>
#include <vector>
#include <string>

#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using namespace httplib;
using namespace nlohmann;
using namespace P8;

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
            if (mid <= 0 || db->mitigations->get_where("id", mid).size() == 0) {
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
            auto tags = split_string(tagparam);
            json response_data;
            vector<int> mitigations;
            // Get all mitigations that contain the input tags which might contain duplicates
            for (auto tag : tags) {
                auto mitigation = db->mitigations->get_where_like("tags", tag);

                for (auto m : mitigation) {
                    mitigations.push_back(m);
                }
            }
            if (mitigations.size() > 0) {
                // Sort the vector so std::unique can remove them due to how it internally works
                std::sort(mitigations.begin(), mitigations.end());
                // Remove all repeated sequences ie. 1,1,1,1,2,2 -> 1,2
                auto distinct = std::unique(mitigations.begin(), mitigations.end());
                // Resize the vector because elements have been removed
                mitigations.resize(std::distance(mitigations.begin(), distinct));

                // Now take new filtered vector and iterate through it
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
        this->server->Get("/mitigations/new/:token", [&](Request request, Response& response) {
            json response_data;
            auto token = request.path_params["token"];
            auto uid = user_id_from_token(token);
            if (authedUsers[uid] == token) {
                auto udid = db->userdata->get_where("userId", uid)[0];
                auto userdata = db->userdata->get(udid);
                auto tags = userdata_to_tags(userdata);
                if (tags.size() == 0)
                    tags = {"default"};

                auto tag = tags[randint(tags.size()-1)];

                auto mids = db->mitigations->get_where_like("tags", tag);
                if (mids.size() == 0)
                    mids = db->mitigations->get_where_like("tags", "default");
                auto mid = mids[randint(mids.size()-1)];
                auto mitigation = db->mitigations->get(mid);

                json data;
                for (auto key : mitigation.keys())
                    data[key] = mitigation[key];

                response_data["data"] = data;
                respond(&response, response_data);
            } else {
                response_data["error"] = "Error, invalid token";
                respond(&response, response_data, 403);
            }
        });
    }
};

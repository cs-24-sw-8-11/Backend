#pragma once

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
            cout << "haha";
            respond(&response, response_data);
        });
        /// @brief Returns a specific mitigation with a given id.
        this->server->Get("/mitigations/tags/:tag", [&](Request request, Response& response){
            auto tag = request.path_params["tag"];
            if (db->mitigations->get_where("tags", tag).size() == 0) {
                json response_data;
                response_data["error"] = "No Mitigations With that Tag.";
                return respond(&response, response_data, 400);
            }
            auto mitigations = db->mitigations->get_where("tags", tag);
            json response_data;
            for (auto i : mitigations) {
                auto mitigation = db->mitigations->get(i);
                json data;
                for (auto key : mitigation.keys()) {
                    data[key] = mitigation[key];
                }
                response_data.push_back(data);
            }
            respond(&response, response_data);
        });
    }
};
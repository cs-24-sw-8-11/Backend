#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace std;

class Answers : public Route {
    using Route::Route;
 public:
    virtual void init() override {
        this->server->Get("/answers/get/:answerId/:token", [&](const httplib::Request& request, httplib::Response& response){
            auto answerId = stoi(request.path_params.at("answerId"));
            auto token = request.path_params.at("token");
            nlohmann::json response_data;
            if(answerId < 0){
                response_data["error"] = "Invalid id";
            }
            auto answer = db->answers->get(answerId);
            auto allowedToGetAnswer = authedUsers[std::stoi(db->journals->get(
                std::stoi(answer["journalId"]))["userId"])] == token;
            if (allowedToGetAnswer) {
                for (auto key : answer.keys()) {
                    response_data[key] = answer[key];
                }
            } else {
                response_data["error"] = "Not allowed to access other users' answers!";
            }            
            response.set_content(response_data, "application/json");
        });
    }
};
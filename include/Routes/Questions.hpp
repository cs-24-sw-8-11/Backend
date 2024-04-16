#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

class Questions : public Route {
    using Route::Route;
 public:
    virtual void init() override {
        this->server->Get("/questions/defaults", [&](Request request, Response& response){
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
            respond(response, response_data);
        });
        this->server->Get("/questions/get/:tag", [&](Request request, Response& response){
            auto tag = request.path_params["tag"];
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
            respond(response, response_data);
        });
    }
};
#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

class Questions : public Route {
    using Route::Route;
 public:
    virtual void init() override {
        this->server->Get("/questions/defaults", [&](Request request, Response response){
            auto questions = db->questions->get_where("tags", "default");
            vector<json> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                json data;
                for (auto key : question.keys()) {
                    data[key] = question[key];
                }
                vec.push_back(data);
            }
            json response_data;
            response_data = move(vec);
            response.set_content(move(response_data), "application/json");
        });
        this->server->Get("/questions/get/:tag", [&](Request request, Response response){
            auto tag = request.path_params.at("tag");
            auto questions = db->questions->get_where("tags", tag);
            vector<json> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                json data({});
                for (auto key : question.keys()) {
                    data[key] = question[key];
                }
                vec.push_back(data);
            }
            json response_data;
            response_data = move(vec);
            response.set_content(move(response_data), "application/json");
        });
    }
};
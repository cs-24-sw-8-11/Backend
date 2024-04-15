#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

class Predictions : public Route {
    using Route::Route;
 public:
    virtual void init() override {
        this->server->Get("/predictions/get/:uid/:token", [&](Request request, Response response){
            auto uid = stoi(request.path_params.at("uid"));
            auto token = request.path_params.at("token");
            json response_data;
            vector<json> result;
            auto predictions = db->predictions->get_where(
                "userId",
                db_int(uid)
            );
            if(authedUsers[uid] == token){
                for (auto prediction : predictions) {
                    auto row = db->predictions->get(prediction);
                    json data;
                    for (auto key : row.keys()) {
                        data[key] = row[key];
                    }
                    result.push_back(data);
                }
            }
            else {
                response_data["error"] = "Invalid id";
                response.set_content(response_data, "application/json");
                return;
            }
            response_data = std::move(result);
            response.set_content(std::move(response_data), "application/json");
        });
        this->server->Post("/predictions/add", [&](Request request, Response response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<std::string>();
            auto qid = body["questionid"].get<std::string>();
            auto uid = UserIdFromToken(token);
            if (authedUsers[uid] == token) {
                auto prediction = manager.create_new_prediction(uid);
                auto answers = db->answers->get_where("questionId", qid);
                auto journals = db->journals->get_where(
                    "userId",
                    db_int(uid));
                for (auto answer : answers) {
                    if (std::count(
                        journals.begin(),
                        journals.end(),
                        std::stoi(db->answers->get(answer)["journalId"])) > 0) {
                        prediction.add_valued_data(qid,
                            std::stod(db->answers->get(answer)["answer"]));
                    }
                }
                auto predictionValue = prediction.build();
                db->predictions->add({
                    "userId",
                    "value"}, {
                    db_int(uid),
                    std::format("{}", predictionValue)});
                response.status = 200;
                response.set_content("Successfully added prediction", "text/plain");
            } else {
                response.status = 403;
                response.set_content("Toekn does not match expected value!", "text/plain");
            }
        });
    }
};
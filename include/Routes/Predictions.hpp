#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using namespace std;
using namespace nlohmann;

class Predictions : public Route {
    using Route::Route;
 public:
    virtual void init() override {
        this->server->Get("/predictions/get/:uid/:token", [&](Request request, Response& response){
            auto uid = stoi(request.path_params["uid"]);
            auto token = request.path_params["token"];
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
                return respond(response, response_data, 400);
            }
            response_data = result;
            respond(response, response_data);
        });
        this->server->Post("/predictions/add", [&](Request request, Response& response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<string>();
            auto qid = body["questionid"].get<string>();
            auto uid = UserIdFromToken(token);
            if (authedUsers[uid] == token) {
                auto prediction = manager.create_new_prediction(uid);
                auto answers = db->answers->get_where("questionId", qid);
                auto journals = db->journals->get_where(
                    "userId",
                    db_int(uid));
                for (auto answer : answers) {
                    if (count(
                        journals.begin(),
                        journals.end(),
                        stoi(db->answers->get(answer)["journalId"])) > 0) {
                        prediction.add_valued_data(qid,
                            stod(db->answers->get(answer)["answer"]));
                    }
                }
                auto predictionValue = prediction.build();
                db->predictions->add({
                    "userId",
                    "value"}, {
                    db_int(uid),
                    to_string(predictionValue)});
                respond(response, string("Successfully added prediction"));
            } else {
                respond(response, string("Token does not match expected value!"), 403);
            }
        });
    }
};
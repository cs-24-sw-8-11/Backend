#include "Route.hpp"

class Answers : public Route {
    using Route::Route;
    virtual void init() override {
        CROW_PTR_ROUTE(app, "/answers/get/<int>/<string>")(
            [&](int answerid, std::string token){
            crow::json::wvalue x({});
            if (answerid < 0) {
                x["error"] = "Invalid id";
                return x;
            }
            auto answer = db->answers->get(answerid);
            auto allowedToGetAnswer = authedUsers[std::stoi(db->journals->get(
                std::stoi(answer["journalId"]))["userId"])] == token;
            if (allowedToGetAnswer) {
                for (auto key : answer.keys()) {
                    x[key] = answer[key];
                }
            } else {
                x["error"] = "Not allowed to access other users' answers!";
            }
            return x;
        });
    }
};
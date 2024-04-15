#include "Route.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;

class Journals : public Route {
    using Route::Route;
    virtual void init() override {
        this->server->Post("/journals/new", [&](Request request, Response response){
            auto body = nlohmann::json::parse(request.body);
            auto token = body["token"].get<std::string>();
            auto comment = body["comment"].get<std::string>();
            auto userid = UserIdFromToken(token);
            if (authedUsers[userid] == token) {
                auto data = body.at("data").items();
                auto journalid = db->journals->add({
                    "comment",
                    "userId"}, {
                    comment,
                    db_int(userid)});
                for (auto i = data.begin(); i != data.end(); ++i) {
                    auto questionId = i.value().back().get<std::string>();
                    auto answer = i.value().front().get<std::string>();
                    db->answers->add({
                        "answer",
                        "journalId",
                        "questionId"}, {
                        answer,
                        db_int(journalid), questionId});
                }
                response.status = 200;
                response.set_content("Successfully created new journal.", "text/plain");
            } else {
                response.status = 403;
                response.set_content("Token does not match expected value!", "text/plain");
            }
        });
        this->server->Get("/journals/get/:jid", [&](Request request, Response response){
            auto body = nlohmann::json::parse(request.body);
            auto jid = stoi(request.path_params.at("jid"));
            nlohmann::json response_data;
            if(jid < 0){
                response_data["error"] = "Invalid id";
                response.set_content(response_data, "application/json");
                return;
            }
            auto journal = db->journals->get(jid);
            for (auto key : journal.keys()) {
                response_data[key] = journal[key];
            }
            response_data["answers"] = db->answers->get_where(
                "journalId",
                db_int(jid));
            response.set_content(response_data, "application/json");
        });
        this->server->Get("/journals/ids/:uid", [&](Request request, Response response){
            nlohmann::json response_data;
            auto uid = stoi(request.path_params.at("uid"));
            if (uid < 0) {
                response_data["error"] = "Invalid id";
                response.set_content(response_data, "application/json");
                return;
            }
            auto journals = db->journals->get_where(
                "userId",
                db_int(uid));
            response_data = std::move(journals);
            response.set_content(std::move(response_data), "application/json");
        });
        this->server->Delete("/journals/delete/:uid/:jid/:token", [&](Request request, Response response){
            auto uid = stoi(request.path_params.at("uid"));
            auto jid = stoi(request.path_params.at("jid"));
            auto token = request.path_params.at("token");
            if (authedUsers[uid] == token) {
                db->journals->delete_item(jid);
                response.status = 202;
                response.set_content("Successfully deleted journal.", "text/plain");
            } else {
                response.status = 403;
                response.set_content("Token does not match expected value!", "text/plain");
            }
        });
    }
};
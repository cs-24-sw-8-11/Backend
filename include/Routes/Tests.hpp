#pragma once

#include <string>

#include "Route.hpp"
#include "Utils.hpp"

class Tests : public Route {
    using Route::Route;

    void init() override {
        Post("/tests/rate", [&](Request request, Response response){
            auto body = json::parse(request.body);
            auto token = body["token"].get<string>();
            auto uid = user_id_from_token(token);
            if (authedUsers[uid] == token) {
                auto pid = body["id"].get<string>();
                db["ratings"].add({
                    {"predictionId", pid},
                    {"rating", body["rating"].get<string>()},
                    {"expected", body["expected"].get<string>()}
                });
                respond(&response, string("Successfully rated prediction!"));
            } else {
                respond(&response, string("Error, user is not authorized!", 403));
            }
        }, {"token", "id", "rating", "expected"});
    }
};

#pragma once
#include <crow.h>

#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

#include "Endpoints.hpp"

class Questions : public Endpoint {
    public:
    explicit Questions() {
        CROW_ROUTE(app, "/questions/defaults")
        ([&]() {
            auto questions = db->questions->get_where("tags", "default");
            std::vector<crow::json::wvalue> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                crow::json::wvalue x({});
                for (auto key : question.keys()) {
                    x[key] = question[key];
                }
                vec.push_back(x);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
        CROW_ROUTE(app, "/questions/get/<string>")
        ([&](std::string tag) {
            auto questions = db->questions->get_where("tags", tag);
            std::vector<crow::json::wvalue> vec;
            for (auto i : questions) {
                auto question = db->questions->get(i);
                crow::json::wvalue x({});
                for (auto key : question.keys()) {
                    x[key] = question[key];
                }
                vec.push_back(x);
            }
            crow::json::wvalue wv;
            wv = std::move(vec);
            return std::move(wv);
        });
    }
};
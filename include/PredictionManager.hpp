#pragma once

#include <ranges>
#include <cstddef>
#include <string>
#include <vector>
#include <cmath>
#include <utility>
#include <iostream>
#include <algorithm>

#include "Globals.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

using namespace std;
using namespace P8;

#define DAY 86400
#define WEEK DAY*7

template<typename T = double>
T sum(vector<T> values) {
    T result;
    for (auto value : values) {
        result += value;
    }
    return result;
}

template<typename T = double>
T mean(vector<T> values) {
    return sum(values)/values.size();
}

template<typename T = double>
function<T(T)> calculate_regression(vector<T> xs, vector<T> ys) {
    auto x_mean = mean(xs);
    auto y_mean = mean(ys);
    auto numerator = 0.0;
    auto denominator = 0.0;
    for (auto [x, y] : views::zip(xs, ys)) {
        numerator += (x-x_mean) * (y-y_mean);
        denominator += pow(x - x_mean, 2);
    }
    auto slope = numerator / denominator;
    auto intercept = y_mean - slope * x_mean;

    return [=](T x){ return intercept + (slope * x); };
    // TODO(mast3r): look into precision/error (MSE?)
}

class PredictionBuilder {
    map<int, double> journal_values;

 public:
    void add_journal(int x, vector<pair<double, double>> journal) {
        vector<double> final_values;
        for (auto [value, rating] : journal) {
            final_values.push_back(rating*value);
        }
        journal_values[x] = mean(final_values);
    }

    size_t size() {
        return journal_values.size();
    }
    double build() {
        vector<double> xs;
        vector<double> ys;
        for (auto [key, value] : journal_values) {
            xs.push_back(key);
            ys.push_back(value);
        }
        auto f = calculate_regression(xs, ys);
        log<DEBUG>("Built prediction with value: {}", f((*(min_element(xs.begin(), xs.end()))) - DAY));
        return f((*(min_element(xs.begin(), xs.end()))) - DAY);
    }
};

/// @brief Manages the PredictionBuilder objects.
class PredictionManager {
 public:
    PredictionBuilder create_new_prediction(int uid) {
        PredictionBuilder builder;
        return builder;
    }
};

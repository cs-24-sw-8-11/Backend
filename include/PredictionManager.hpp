#pragma once

#include <ranges>
#include <cstddef>
#include <string>
#include <vector>
#include <cmath>
#include <utility>
#include <iostream>

#include "Globals.hpp"

using namespace std;

// these will be used in later implementations
#define LOW_STRESS 0
#define MODERATE_STRESS 14
#define HIGH_STRESS 27

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
vector<T> normalizeCDF(vector<T> values) {
    vector<T> results;
    for (auto value : values) {
        results.push_back(0.5 * erfc(-value * M_SQRT1_2));
    }
    return results;
}

template<typename T = double>
function<T(T)> calculate_regression(vector<T> xs, vector<T> ys) {
    auto x_mean = mean(xs);
    auto y_mean = mean(ys);
    auto numerator = 0.0;
    auto denominator = 0.0;
    for (auto [x, y] : ranges::zip_view(xs, ys)) {
        numerator += (x-x_mean) * (y-y_mean);
        denominator += pow(x - x_mean, 2);
    }
    auto slope = numerator / denominator;
    auto intercept = y_mean - slope * x_mean;

    return [=](T x){ return intercept + (slope * x); };
    // TODO(mast3r): look into precision/error (MSE?)
}

class PredictionBuilder {
    double prediction_value = 0.5;
    vector<pair<string, double>> valued_data;
    vector<pair<string, bool>> boolean_data;

 public:
    void add_valued_data(string qid, double data) {
        valued_data.push_back(make_pair(qid, data));
        if (VERBOSE)
            cout << "Added valued data" << endl;
    }
    void add_boolean_data(string qid, bool data) {
        boolean_data.push_back(make_pair(qid, data));
        if (VERBOSE)
            cout << "Added boolean data" << endl;
    }
    size_t size() {
        return valued_data.size() + boolean_data.size();
    }
    double build() {
        // The most important step here is to normalize the data
        // such that the stress factor is within the range
        vector<double> final_data;
        for (auto pair : valued_data) {
            final_data.push_back(pair.second);
        }
        for (auto pair : boolean_data) {
            final_data.push_back(pair.second ? 1.0 : 0.0);
        }
        // now we normalize
        if (VERBOSE)
            cout << "Built prediction with value: " << mean(final_data) << endl;
        return mean(final_data);
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

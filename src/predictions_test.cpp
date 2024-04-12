#include <format>
#include <functional>

#include "PredictionManager.hpp"
#include "TestTemplate.hpp"

auto num_entries = 1000;

using namespace std;
using namespace std::ranges;

typedef pair<string, double> LabeledDouble;
typedef pair<string, bool> LabeledBool;

class PredictionTest : public Test<std::function<void()>> {
    void init() {
    }
    public:
        PredictionManager manager;
};

template<typename T = int>
vector<T> make_range(int n, function<T(int)> functor) {
    vector<T> result;
    for (auto i = 0; i < n; i++) {
        result.push_back(functor(i));
    }
    return result;
}

template<typename T = int>
vector<T> make_range(int n) {
    vector<T> result;
    for (auto i = 0; i < n; i++)
        result.push_back((T)i);
    return result;
}

int main() {
    PredictionTest test;
    test.add_test("add-valued-data", [&](){
        auto uid = 1;
        auto data = make_range<LabeledDouble>(num_entries, [](int i){
            return make_pair(format("{}", i), i/num_entries);
        });
        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for (auto [name, value] : data) {
            predictionBuilder.add_valued_data(name, value);
        }
        assert(predictionBuilder.size() == data.size());
    });
    test.add_test("add-boolean-data", [&](){
        auto uid = 1;
        auto data = make_range<LabeledBool>(num_entries, [](int i){
            return make_pair(format("{}", i), i%2 == 0);
        });
        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for (auto [name, value] : data) {
            predictionBuilder.add_boolean_data(name, value);
        }
        assert(predictionBuilder.size() == data.size());
    });
    test.add_test("run-prediction", [&](){
        auto uid = 1;
        auto data = make_range<LabeledBool>(num_entries, [](int i){
            return make_pair(format("{}", i), true);
        });

        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for (auto [name, value] : data) {
            predictionBuilder.add_boolean_data(name, value);
        }
        // function that does the heavy lifting
        auto stresslevel = predictionBuilder.build();
        assert(stresslevel >= 0.0 && stresslevel <= 1.0);
    });
    test.run();

    PredictionTest extended;

    extended.add_test("regression", [&](){
        auto xs = make_range<double>(100);
        auto ys = make_range<double>(100);
        auto f = calculate_regression(xs, ys);
        vector<double> result;
        for (auto x : xs) {
            result.push_back(f(x+100));
        }

        for (auto [value, expectedValue] : zip_view(
            result,
            make_range<double>(100, [](int i){return i+100;}))) {
            assert(value == expectedValue);
        }
    });

    extended.add_test("applied regression", [&](){
        auto uid = 1;
        vector<double> result;
        for (auto i = 0; i < num_entries; i++) {
            auto data = make_range<LabeledDouble>(5, [](int i) {
                // positive linear data
                return make_pair(std::format("{}", i), (i%5)+i);
            });
            auto predictionBuilder = extended.manager
                .create_new_prediction(uid);
            for (auto [name, value] : data) {
                predictionBuilder.add_valued_data(name, value);
            }
            result.push_back(predictionBuilder.build());
        }
        auto f = calculate_regression(
            make_range<double>(result.size()),
            result);
        assert(std::ranges::is_sorted(make_range<double>(1000, [=](int i){ return f(i); })));
    });

    extended.run();
}

#include <format>
#include <functional>

#include "PredictionManager.hpp"
#include "TestTemplate.hpp"

auto num_entries = 1000;

class PredictionTest : public Test<std::function<void()>> {
    void init(){
    }
    public:
        PredictionManager manager;
};

template<typename T = int>
std::vector<T> make_range(int n, std::function<T(int)> functor){
    std::vector<T> result;
    for(auto i = 0; i < n; i++){
        result.push_back(functor(i));
    }
    return result;
}

int main(){
    PredictionTest test;
    test.add_test("add-valued-data", [&](){ 
        auto uid = 1;
        auto data = make_range<std::pair<std::string, double>>(num_entries, [](int i){return std::make_pair(std::format("{}", i), i/num_entries);});
        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for(auto pair : data){
            predictionBuilder.add_valued_data(pair.first, pair.second);
        }
        assert(predictionBuilder.size() == data.size());
    });
    test.add_test("add-boolean-data", [&](){
        auto uid = 1;
        auto data = make_range<std::pair<std::string, bool>>(num_entries, [](int i){return std::make_pair(std::format("{}", i), i%2==0);});
        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for(auto pair : data){
            predictionBuilder.add_boolean_data(pair.first, pair.second);
        }
        assert(predictionBuilder.size() == data.size());
    });
    test.add_test("run-prediction", [&](){
        auto uid = 1;
        auto data = make_range<std::pair<std::string, bool>>(num_entries, [](int i){return std::make_pair(std::format("{}", i), true);});

        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for(auto pair : data){
            predictionBuilder.add_boolean_data(pair.first, pair.second);
        }
        auto stresslevel = predictionBuilder.build(); // function that does the heavy lifting
        assert(stresslevel >= 0.0 && stresslevel <= 1.0);
    });
    test.run();

    PredictionTest extended;

    extended.add_test("regression test", [&](){
        auto uid = 1;
        std::vector<double> results;
        for(auto i = 0; i < num_entries; i++){
            auto data = make_range<std::pair<std::string, double>>(10, [](int i){
                return std::make_pair(std::format("{}", i), i%5);
            });
            auto builder = extended.manager.create_new_prediction(uid);
            for(auto pair : data)
                builder.add_valued_data(pair.first, pair.second);
            results.push_back(builder.build());
        }
        //auto futureData = extended.manager.create_new_regression_model(uid);
        
    });


};
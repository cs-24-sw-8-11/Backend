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

template<typename T>
std::vector<T> make_range(int n, std::function<T(int)> f){
    std::vector<T> result;
    for(auto i = 0; i < n; i++){
        result.push_back(f(i));
    }
    return result;
}

int main(){
    PredictionTest test;
    test.add_test("add-valued-data", [&](){ 
        auto uid = 1;
        auto data = make_range<std::pair<std::string, double>>(num_entries, [](int i){return std::make_pair(std::format("{}", i), i/100);});
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


};
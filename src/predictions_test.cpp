#include "PredictionManager.hpp"
#include "TestTemplate.hpp"

class PredictionTest : public Test<std::function<void()>> {
    void init(){
    }
    public:
        PredictionManager manager;
};


int main(){
    PredictionTest test;
    test.add_test("add-valued-data", [&](){ 
        auto uid = 1;
        std::vector<std::pair<std::string, int>> data = { //list of pairs, keys are the qid and values are the valued data
            std::make_pair("1", 1),
            std::make_pair("2", 4),
            std::make_pair("3", 0.6),
            std::make_pair("4", 0.7)
        };
        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for(auto pair : data){
            predictionBuilder.add_valued_data(pair.first, pair.second);
        }
        assert(predictionBuilder.size() == data.size());
    });
    test.add_test("add-boolean-data", [&](){
        auto uid = 1;
        auto data = {
            std::make_pair("1", true),
            std::make_pair("2", false),
            std::make_pair("3", false),
            std::make_pair("4", true)
        };
        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for(auto pair : data){
            predictionBuilder.add_boolean_data(pair.first, pair.second);
        }
        assert(predictionBuilder.size() == data.size());
    });
    test.add_test("run-prediction", [&](){
        auto uid = 1;
        auto data = {
            std::make_pair("1", true),
            std::make_pair("2", false),
            std::make_pair("3", false),
            std::make_pair("4", false)
        };
        auto predictionBuilder = test.manager.create_new_prediction(uid);
        for(auto pair : data){
            predictionBuilder.add_boolean_data(pair.first, pair.second);
        }
        auto stresslevel = predictionBuilder.build(); // function that does the heavy lifting
        assert(stresslevel > 0.0);
    });
    test.run();
};
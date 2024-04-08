#include <cstddef>
#include <string>
#include <vector>

#define LOW_STRESS 0
#define MODERATE_STRESS 14
#define HIGH_STRESS 27

class PredictionBuilder {
    float prediction_value = 0.5;
    std::vector<std::pair<std::string, int>> valued_data;
    std::vector<std::pair<std::string, bool>> boolean_data;
    public:
        void add_valued_data(std::string qid, int data){
            valued_data.push_back(std::make_pair(qid, data));
        }
        void add_boolean_data(std::string qid, bool data){
            boolean_data.push_back(std::make_pair(qid, data));
        }
        size_t size(){
            return valued_data.size() + boolean_data.size();
        }
        float build(){
            for(auto pair : valued_data){
                prediction_value += (pair.second);
            }
            for(auto pair : boolean_data){
                prediction_value += pair.second ? .1 : -(.1);
            }
            return prediction_value;
        }
};

class PredictionManager {
    public:
        PredictionBuilder create_new_prediction(int uid){
            PredictionBuilder builder;
            return builder;
        }
};
#include <cstddef>
#include <string>

class PredictionBuilder {
    float prediction_value = 0.0;
    size_t _size = 0;
    public:
        void add_valued_data(std::string qid, float data){
            _size++;
        }
        void add_boolean_data(std::string qid, bool data){
            _size++;
        }
        size_t size(){
            return _size;
        }
        float build(){
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
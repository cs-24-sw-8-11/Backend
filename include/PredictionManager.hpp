#include <cstddef>
#include <string>
#include <vector>
#include <cmath>

// these will be used in later implementations
#define LOW_STRESS 0
#define MODERATE_STRESS 14
#define HIGH_STRESS 27

std::vector<double> normalizeCDF(std::vector<double> values){ // https://stackoverflow.com/questions/2328258/cumulative-normal-distribution-function-in-c-c lmao
    std::vector<double> results;
    for(auto value : values){
        results.push_back(0.5 * erfc(-value * M_SQRT1_2));
    }
    return results;
}

double sum(std::vector<double> values){
    double result;
    for(auto value : values){
        result += value;
    }
    return result;
}
double mean(std::vector<double> values){
    return sum(values)/values.size();
}

class PredictionBuilder {
    double prediction_value = 0.5;
    std::vector<std::pair<std::string, double>> valued_data;
    std::vector<std::pair<std::string, bool>> boolean_data;
    public:
        void add_valued_data(std::string qid, double data){
            valued_data.push_back(std::make_pair(qid, data));
        }
        void add_boolean_data(std::string qid, bool data){
            boolean_data.push_back(std::make_pair(qid, data));
        }
        size_t size(){
            return valued_data.size() + boolean_data.size();
        }
        double build(){
            // The most important step here is to normalize the data such that the stress factor is within the range
            std::vector<double> final_data;
            for(auto pair : valued_data){
                final_data.push_back(pair.second);
            }
            for(auto pair : boolean_data){
                final_data.push_back(pair.second ? 1.0 : 0.0);
            }
            // now we normalize
            auto normalized = normalizeCDF(final_data);

            return mean(normalized);
        }
};

class PredictionManager {
    public:
        PredictionBuilder create_new_prediction(int uid){
            PredictionBuilder builder;
            return builder;
        }
};
#include <format>
#include <functional>

#include "PredictionManager.hpp"
#include "TestTemplate.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

auto num_entries = 1000;

using namespace std;
using namespace P8;

class PredictionTest : public Test<std::function<void()>> {
    void init() {
    }
    public:
        PredictionManager manager;
};

typedef std::pair<std::string, double> LabeledDouble;
typedef std::pair<std::string, std::string> LabeledString;
typedef std::pair<std::string, bool> LabeledBool;

int main() {
    PredictionTest main;

    main.add_test("add journals", [&](){
        auto builder = main.manager.create_new_prediction(0);
        for (auto i = 0; i < 7; i++) {
            vector<pair<double, double>> answers{
                {0.5, static_cast<double>(i)/7}
            };
            builder.add_journal(i, answers);
        }
        auto result = builder.build();
    });
    main.run();
}

#include <format>

#include "Utils.hpp"
#include "TestTemplate.hpp"

using namespace std;

typedef Test<function<void()>> UtilsTest;

int main() {
    auto utils = UtilsTest();
    utils.add_test("Test Make Range: edge cases", [&](){
        // testing edge cases
        auto range1 = P8::make_range(0);
        auto range2 = P8::make_range(-1);

        assert(range1.size() == 0);
        assert(range2.size() == 0);
    });
    utils.add_test("Test Make Range: funky types", [&](){
        auto range = P8::make_range<vector<map<int, string>>>(1, [](int i){
            vector<map<int, string>> result = {
                {
                    make_pair(i, "test")
                }
            };
            return result;
        });
        vector<vector<map<int, string>>> control = {{{make_pair(0, "test")}}};
        assert(range == control);
    });

    utils.add_test("Test is sorted", [&](){
        auto sorted_vector = P8::make_range(100);
        auto unsorted_vector = P8::make_range<int>(100, [](int i){ return 100-i; });

        assert(P8::is_sorted(sorted_vector));
        assert(!P8::is_sorted(unsorted_vector));
    });

    utils.add_test("Test split string", [&](){
        auto comma = "this,list,is,seperated";
        auto spaces = "this list is seperated";
        vector<string> control = {"this", "list", "is", "seperated"};
        assert(P8::split_string(comma) == control);
        assert(P8::split_string(spaces, " ") == control);
    });

    utils.add_test("Test to_lower_case", [&](){
        auto UPPER_CASE_STRING_WITH_SPECIALS = "TRKTRJOK;TRHOTRHO¤%/%&/%()&/¤%/¤HTKUITHTRH%&/%/¤#4564564648587";
        auto control =                         "trktrjok;trhotrho¤%/%&/%()&/¤%/¤htkuithtrh%&/%/¤#4564564648587";
        assert(P8::to_lower_case(UPPER_CASE_STRING_WITH_SPECIALS) == control);
    });

    utils.add_test("Test threadpool 0", [&](){
        try {
            P8::ThreadPool<string, string> pool{0};
            pool.map([](string input){ return input; }, {"test"}, [](string input, string output){});
            assert(false);
        }
        catch(exception& e){
            assert(true);
        }
    });

    utils.add_test("Test threadpool scalability", [&](){
        auto f = [&](int i){return i*2;};
        for (int i = 1; i < 100; i++) {
            P8::ThreadPool<int, int> pool{i};
            auto result = pool.map(f, P8::make_range(i), [](int input, int output){});
            assert(result.size() == i);
            assert(result == P8::make_range<int>(i, f));
        }
    });

    utils.add_test("Test randint", [&](){
        auto start = 100;
        auto end = 1000;
        for (auto value : P8::make_range<int>(100, [=](int i){
            return P8::randint(start, end);
        })){
            assert(value >= start && value < end);
        }
    });

    utils.run();
}

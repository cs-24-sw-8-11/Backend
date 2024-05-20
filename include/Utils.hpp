#pragma once
#include <format>
#include <ranges>
#include <vector>
#include <functional>
#include <future>
#include <fstream>
#include <map>
#include <iostream>
#include <cassert>
#include <mutex>
#include <memory>
#include <string>
#include <cctype>
#include <sstream>

using namespace std;

namespace P8 {
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

template<typename T = double>
bool is_sorted(vector<T> values) {
    for (auto i = 0; i < values.size()-1; i++) {
        if (values[i] > values[i+1]) {
            return false;
        }
    }
    return true;
}

vector<string> split_string(string s, string delimiter) {
    vector<string> res;
    int pos = 0;
    while (pos < s.size()) {
        pos = s.find(delimiter);
        res.push_back(s.substr(0, pos));
        s.erase(0, pos+delimiter.size());
    }
    return res;
}

vector<string> split_string(string s) {
    return split_string(s, ",");
}

string to_lower_case(string input) {
    string final_string;
    for (auto c : input) {
        final_string += tolower(c);
    }
    return final_string;
}

int randint(int start, int end) {
    if (start == end) return start;
    return rand() % (end-start) + start;
}
int randint(int end) {
    return randint(0, end);
}

template<typename Input, typename Output>
class Worker {
    function<Output(Input)> f;
    future<void> task;
    map<int, Input> jobs;
    function<void(Input, Output)> logger = [](Input in, Output out){ cout << out << endl; };
    map<int, Output> results;
    shared_ptr<mutex> results_mutex;

 public:
    explicit Worker(function<Output(Input)> f) {
        this->f = f;
        this->results_mutex = make_shared<mutex>();
        results_mutex->lock();
    }
    void start() {
        task = async(launch::async, [&](map<int, Input> _jobs){ this->run(_jobs); }, jobs);
    }
    void run(map<int, Input> jobs) {
        for (auto [i, value] : jobs) {
            results[i] = f(value);
            logger(value, results[i]);
        }
        results_mutex->unlock();
    }
    map<int, Output> get() {
        results_mutex->lock();
        return results;
    }
    void add_data(int id, Input data) {
        jobs[id] = data;
    }
    void set_logger(function<void(Input, Output)> logger) {
        this->logger = logger;
    }
};

template<typename Input, typename Output>
class ThreadPool {
    int size;
    vector<Output> map(vector<Worker<Input, Output>> &workers) {
        std::map<int, Output> results;
        for (auto& worker : workers)
            worker.start();
        for (auto& worker : workers) {
            for (auto [i, value] : worker.get()) {
                results[i] = value;
            }
        }
        vector<Output> final_result;
        for (auto i = 0; i < results.size(); i++) {
            final_result.push_back(results[i]);
        }
        return final_result;
    }

 public:
    explicit ThreadPool(int size) {
        if (size == 0)
            throw exception();
        this->size = size;
    }
    vector<Output> map(function<Output(Input)> f, vector<Input> values, function<void(Input, Output)> logger) {
        vector<Worker<Input, Output>> workers;

        for (auto i = 0; i < size; i++) {
            workers.push_back(Worker<Input, Output>(f));
        }
        for (auto& worker : workers)
            worker.set_logger(logger);
        for (auto [i, value] : views::zip(make_range(values.size()), values))
            workers[i%size].add_data(i, value);
        return map(workers);
    }
    vector<Output> map(function<Output(Input)> f, vector<Input> values) {
        vector<Worker<Input, Output>> workers;
        for (auto i = 0; i < size; i++)
            workers.push_back(Worker<Input, Output>(f));
        for (auto [i, value] : views::zip(make_range(values.size()), values))
            workers[i%size].add_data(i, value);
        return map(workers);
    }
};
class RunCommandException : public exception {
    string why;
 public:
    explicit RunCommandException(string why) : why(why) {}
    string what(){
        return why;
    }
};
map<string, string> run_cmd(string command) {
    try {
        map<string, string> result;
        auto stdout_path = "/tmp/p8_backend_stdout";
        auto stderr_path = "/tmp/p8_backend_stderr";
        auto final_command = format("bash -c '{} 2> {} 1> {}'", command, stderr_path, stdout_path);
        auto error_code = system(final_command.c_str());
        ifstream stdout_file{stdout_path};
        ifstream stderr_file{stderr_path};
        stringstream out_stream;
        stringstream err_stream;
        out_stream << stdout_file.rdbuf();
        err_stream << stderr_file.rdbuf();
        result["stdout"] = out_stream.str();
        result["stderr"] = err_stream.str();

        P8::_log<P8::DEBUG>("stdout: {}", result["stdout"]);
        P8::_log<P8::DEBUG>("stderr: {}", result["stderr"]);

        return result;
    }
    catch (exception& e) {
        throw RunCommandException("fuck");
    }
}

template<typename K, typename V>
bool mapHas(map<K, V> map, K target) {
    for (auto [key, value] : map)
        if (key == target)
            return true;
    return false;
}

map<string, map<int, string>> tag_map = {
{"education", (map<int, string>) {
    {1, ""},
    {2, "High School"},
    {3, "University"},
    {4, "Graduate"},
    }},
{"urban", (map<int, string>) {
    {1, "Rural"},
    {2, "Suburban"},
    {3, "Urban"}
    }},
{"gender", (map<int, string>) {
    {1, "Male"},
    {2, "Female"},
    {3, "Other"}
    }},
{"religion", (map<int, string>) {
    {1, "Agnostic"},
    {2, "Atheist"},
    {3, "Buddhist"},
    {4, "Christian"},
    {5, "Christian"},
    {6, "Christian"},
    {7, "Christian"},
    {8, "Hindu"},
    {9, "Jewish"},
    {10, "Muslim"},
    {11, "Sikh"},
    {12, ""}
    }},
{"orientation", (map<int, string>) {
    {1, "Heterosexual"},
    {2, "Bisexual"},
    {3, "Homosexual"},
    {4, "Asexual"},
    {5, ""}
    }},
{"race", (map<int, string>) {
    {10, "Asian"},
    {20, "Arab"},
    {30, "Black"},
    {40, "Indigenous Australian"},
    {50, "Native American"},
    {60, "White"},
    {70, ""}
    }},
{"married", (map<int, string>) {
    {1, "Married"},
    {2, "Single"}
    }},
{"age", (map<int, string>) {
    }},
{"pets", (map<int, string>) {
    }}
};

vector<string> userdata_to_tags(map<string, string> userdata) {
    vector<string> tags;

    for (auto [key, value] : userdata) {
        if (key == "userId") continue;

        if (mapHas(tag_map[key], stoi(value))) {
            if (tag_map[key][stoi(value)] == "") continue;
            tags.push_back(tag_map[key][stoi(value)]);
        }
    }
    return tags;
}

}  // namespace P8

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

#define put make_pair

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
map<string, string> run_cmd(string command) {
    map<string, string> result;
    auto stdout_path = "/tmp/p8_backend_stdout";
    auto stderr_path = "/tmp/p8_backend_stderr";
    auto final_command = format("bash -c '{} 2> {} 1> {}'", command, stderr_path, stdout_path);
    auto error_code = system(final_command.c_str());
    ifstream stdout_file{stdout_path};
    ifstream stderr_file{stdout_path};
    stdout_file >> result["stdout"];
    stderr_file >> result["stderr"];
    if (error_code) {
        cout << "command: " << final_command << endl;
        cout << "stdout:  " << result["stdout"] << endl;
        cout << "stderr:  " << result["stderr"] << endl;
    }
    return result;
}

enum UserdataKeys {
    Education,
    Urban,
    Gender,
    Religion,
    Orientation,
    Race,
    Married,
    Age,
    Pets
};

UserdataKeys userdata_key_to_enum(string key){
    map<string, UserdataKeys> d = {
        put("education", Education),
        put("urban", Urban),
        put("gender", Gender),
        put("religion", Religion),
        put("orientation", Orientation),
        put("race", Race),
        put("married", Married),
        put("age", Age),
        put("pets", Pets)
    };
    return d[to_lower_case(key)];
}

vector<string> userdata_to_tags(Row userdata){
    vector<string> tags;
    map<string, map<int, string>> d = {
        put("education", (map<int, string>){
            put(2, "High School")
        })
    };
    /*for(auto key : userdata.keys()){
        if(key == "userId") continue;

        switch(userdata_key_to_enum(key)){
            case Education:
                switch(stoi(userdata[key])){
                    case 2:
                        tags.push_back("High School");
                        break;
                    case 3:
                        tags.push_back("University");
                        break;
                    case 4:
                        tags.push_back("Graduate");
                    default:
                        continue;
                }
                break;
            case Urban:
                switch(stoi(userdata[key])){
                    case 1:
                        tags.push_back("Rural");
                        break;
                    case 2:
                        tags.push_back("Suburban");
                        break;
                    case 3:
                        tags.push_back("Urban");
                        break;
                    default:
                        continue;
                }
                break;
            case Gender:
                switch(stoi(userdata[key])){
                    case 1:
                        tags.push_back("Male");
                        break;
                    case 2:
                        tags.push_back("Female");
                        break;
                    default:
                        continue;
                }
                break;
            case Religion:
                switch(stoi(userdata[key])){
                    case 
                }
        }
    }*/
}

}  // namespace P8

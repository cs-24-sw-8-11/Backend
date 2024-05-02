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
        if (size <= 0)
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
        for (auto [i, value] : ranges::zip_view(make_range(values.size()), values))
            workers[i%size].add_data(i, value);
        return map(workers);
    }
    vector<Output> map(function<Output(Input)> f, vector<Input> values) {
        vector<Worker<Input, Output>> workers;
        for (auto i = 0; i < size; i++)
            workers.push_back(Worker<Input, Output>(f));
        for (auto [i, value] : ranges::zip_view(make_range(values.size()), values))
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
vector<string> split_string(string s, string delimiter) {
    vector<string> res;
    int pos = 0;
    while (pos < s.size()) {
        pos = s.find(delimiter);
        res.push_back(s.substr(0, pos));
        s.erase(0, pos+1);
    }
    return res;
}
vector<string> split_string(string s) {
    return split_string(s, ",");
}
}  // namespace P8

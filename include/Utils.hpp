#pragma once
#include <vector>
#include <functional>
#include <future>
#include <fstream>

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
        function<void(Output)> logger = [](Output out){ cout << out << endl; };

     public:
        bool running = true;
        map<int, Output> results;

        Worker(function<Output(Input)> f){
            this->f = f;
        }
        void start(){
            this->task = async(launch::async, [&](){ this->run(); });
        }
        void run(){
            for(auto [i, value] : jobs){
                results[i] = f(value);
                logger(results[i]);
            }
            running = false;
        }
        map<int, Output> get(){
            while(running);

            return results;
        }

        void add_data(int id, Input data){
            jobs[id] = data;
        }
        void set_logger(function<void(Output)> logger){
            this->logger = logger;
        }
    };

    template<typename Input, typename Output>
    class ThreadPool {
        int size;
        vector<Output> map(vector<Worker<Input, Output>> &workers){
            std::map<int, Output> results;

            for(auto& worker : workers)
                worker.start();

            for(auto& worker : workers){
                for(auto [i, value] : worker.get()){
                    results[i] = value;
                }
            }

            vector<Output> final_result;
            for(auto i = 0; i < results.size(); i++){
                final_result.push_back(results[i]);
            }
            return final_result;
        }
     public:

        ThreadPool(int size){
            this->size = size;
        }
        vector<Output> map(function<Output(Input)> f, vector<Input> values, function<void(Output)> logger){
            vector<Worker<Input, Output>> workers;
            
            for(auto i = 0; i < size; i++){
                auto worker = Worker<Input, Output>(f);
                worker.set_logger(logger);
                workers.push_back(worker);
            }
            for(auto [i, value] : ranges::zip_view(make_range(values.size()), values))
                workers[i%size].add_data(i, value);
            return map(workers);
        }
        vector<Output> map(function<Output(Input)> f, vector<Input> values){
            vector<Worker<Input, Output>> workers;

            for(auto i = 0; i < size; i++){
                workers.push_back(Worker<Input, Output>(f));
            }
            for(auto [i, value] : ranges::zip_view(make_range(values.size()), values))
                workers[i%size].add_data(i, value);
            return map(workers);
        }
    };

    map<string, string> run_cmd(string command){
        map<string, string> result;
        auto stdout_path = "/tmp/p8_backend_stdout";
        auto stderr_path = "/tmp/p8_backend_stderr";
        auto final_command = format("bash -c '{} 2> {} 1> {}'", command, stderr_path, stdout_path);
        auto error_code = system(final_command.c_str());
        ifstream stdout_file{stdout_path};
        ifstream stderr_file{stdout_path};
        stdout_file >> result["stdout"];
        stderr_file >> result["stderr"];
        if(error_code){
            cout << "command: " << final_command << endl;
            cout << "stdout:  " << result["stdout"] << endl;
            cout << "stderr:  " << result["stderr"] << endl;
        }

        return result;
    }
}
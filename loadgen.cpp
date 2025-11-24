#include "httplib.h"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <string>
#include <mutex>

std::atomic<long long> total_requests{0};
std::atomic<long long> total_latency{0};
static const int HOT_KEY_COUNT = 10;
std::once_flag hotkeys_init_flag; 

std::mutex mtx;

// Generate int key
long long randomKey(int tid) {
    static thread_local long long counter = 0;
    return tid * 1000000LL + counter++;
}

void Workload_GETALL(httplib::Client& cli, int tid) {
    long long key = randomKey(tid);
    cli.Get("/read?key=" + std::to_string(key));
}

void Workload_PUTALL(httplib::Client& cli, int tid) {
    long long key = randomKey(tid);
    std::string value = "val_" + std::to_string(rand() % 1000);

    cli.Post("/create?key=" + std::to_string(key) + "&value=" + value, "", "text/plain");
}

void Workload_GETPOPULAR(httplib::Client& cli, int tid) {
    std::call_once(hotkeys_init_flag, [&]() {
        for (long long k = 1; k <= HOT_KEY_COUNT; k++) {
            cli.Post("/create?key=" + std::to_string(k) +
                     "&value=hot_" + std::to_string(k),
                     "", "text/plain");
        }
    });

    // Pick a random hot key
    long long key = 1 + (rand() % HOT_KEY_COUNT);

    cli.Get("/read?key=" + std::to_string(key));
}

void clientThread(int tid, int duration, const std::string& workload) {
    httplib::Client cli("127.0.0.1", 8080);
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(5, 0);
    cli.set_write_timeout(5, 0);

    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(duration)) {
        auto req_start = std::chrono::steady_clock::now();

        try {
            if (workload == "putall") {
                Workload_PUTALL(cli, tid);
            }
            else if (workload == "getall") {
                Workload_GETALL(cli, tid);
            } 
            else {
                Workload_GETPOPULAR(cli, tid);
            } 
        } catch (...) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cerr << "Thread " << tid << " request failed\n";
        }

        auto req_end = std::chrono::steady_clock::now();
        long long latency = std::chrono::duration_cast<std::chrono::microseconds>(req_end - req_start).count();

        total_requests++;
        total_latency += latency;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " --workload [putall|getall|getpopular] --threads N --duration S\n";
        return 1;
    }

    std::string workload = argv[2];
    int threads = std::stoi(argv[4]);
    int duration = std::stoi(argv[6]);

    std::vector<std::thread> pool;
    for (int i = 0; i < threads; i++) {
        pool.emplace_back(clientThread, i, duration, workload);
    }
    for (auto& t : pool) t.join();

    double throughput = total_requests.load() / (double)duration;
    double avg_latency_ms = (total_latency.load() / (double)total_requests.load()) / 1000.0;

    std::cout << "Workload: " << workload << "\n";
    std::cout << "Threads: " << threads << "\n";
    std::cout << "Duration: " << duration << "s\n";
    std::cout << "Throughput: " << throughput << " req/s\n";
    std::cout << "Average Response Time: " << avg_latency_ms << " ms\n";

    return 0;
}

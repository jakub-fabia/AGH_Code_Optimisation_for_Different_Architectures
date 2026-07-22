#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <cctype>

std::string normalise(const std::string& input) {
    std::string s;
    for (char c : input) {
        unsigned char u = c;
        if (u < 32 || u > 126) continue;
        if (std::isspace(u)) {
            if (!s.empty() && s.back() != ' ') {
                s += ' ';
            }
        } else if (std::ispunct(u)) {
            s += ',';
        }
        else {
            s += std::tolower(u);
        }
    }
    return s;
}

std::string deduplicate(const std::string& s) {
    std::string result, prev;
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == ',') {
            prev.clear();
            result += ',';
            i++;
            continue;
        }
        while (i < s.size() && s[i] == ' ') ++i;
        size_t start = i;
        while (i < s.size() && s[i] != ' ' && s[i] != ',') ++i;
        if (start == i) break;
        std::string word = s.substr(start, i - start);
        if (word != prev) {
            if (start > 0 && s[start - 1] == ' ') result += ' ';
            result += word;
            prev = word;
        }
    }
    return result;
}

std::string parallel_process(const std::string& input, int num_threads) {
    if (input.empty() || num_threads <= 1) {
        return deduplicate(normalise(input));
    }

    std::vector<size_t> bounds(num_threads + 1);
    bounds[0] = 0;
    bounds[num_threads] = input.size();
    size_t chunk = input.size() / num_threads;
    for (int i = 1; i < num_threads; ++i) {
        size_t pos = chunk * i;
        while (pos < input.size() && !std::isspace((unsigned char)input[pos])) ++pos;
        bounds[i] = pos;
    }

    std::vector<std::string> results(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::string part = input.substr(bounds[i], bounds[i + 1] - bounds[i]);
            results[i] = deduplicate(normalise(part));
        });
    }
    for (auto& t : threads) t.join();

    std::string merged;
    for (int i = 0; i < num_threads; ++i) {
        if (results[i].empty()) continue;
        if (!merged.empty()) merged += ' ';
        merged += results[i];
    }
    return deduplicate(merged);
}

int main() {
    std::ifstream we("file.txt");
    std::string input, line;

    if (we.is_open()) {
        while (getline(we, line)) input += line + '\n';
    }

    const int num_threads = 16;
    std::string output = parallel_process(input, num_threads);
    std::cout << output << "\n";
    return 0;
}

#include <iostream>
#include <string>
#include <fstream>

void normalise(std::string& s) {
    size_t write = 0;
    for (size_t read = 0, n = s.size(); read < n; ++read) {
        unsigned char u = s[read];
        if (u < 32 || u > 126) continue;
        if (std::isspace(u)) {
            if (write > 0 && s[write - 1] != ' ') s[write++] = ' ';
        } else if (std::ispunct(u)) {
            s[write++] = ',';
        } else {
            s[write++] = std::tolower(u);
        }
    }
    s.resize(write);
}

void deduplicate(std::string& s) {
    size_t write = 0;
    size_t read = 0;
    std::string_view prev;
    
    while (read < s.size()) {
        if (s[read] == ',') {
            s[write++] = ',';
            prev = {};
            ++read;
            continue;
        }
        while (read < s.size() && s[read] == ' ') ++read;
        size_t word_start = read;
        while (read < s.size() && s[read] != ' ' && s[read] != ',') ++read;
        if (word_start == read) break;
        size_t word_len = read - word_start;
        std::string_view word(s.data() + word_start, word_len);
        if (word != prev) {
            if (word_start > 0 && s[word_start - 1] == ' ') {
                s[write++] = ' ';
            }
            if (write != word_start) {
                for (size_t k = 0; k < word_len; ++k) {
                    s[write + k] = s[word_start + k];
                }
            }
            prev = std::string_view(s.data() + write, word_len);
            write += word_len;
        }
    }
    s.resize(write);
}

int main() {
    std::ifstream we("file.txt");
    std::string input, line;

    if (we.is_open()) {
        while (getline(we, line)) input += line + '\n';
    }
    
    std::string buffer = input;
    normalise(buffer);
    deduplicate(buffer);
    std::cout << buffer << "\n";
    return 0;
}
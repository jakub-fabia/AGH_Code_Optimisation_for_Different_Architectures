#include <iostream>
#include <string>
#include <fstream>

std::string normalise(const std::string& input) {
    std::string s;
    s.reserve(input.size());
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
    result.reserve(s.size());
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

int main() {
    std::ifstream we("file.txt");
    std::string input, line;

    if (we.is_open()) {
        while (getline(we, line)) input += line + '\n';
    }
    
    std::string output = deduplicate(normalise(input));
    std::cout << output << "\n";
    return 0;
}
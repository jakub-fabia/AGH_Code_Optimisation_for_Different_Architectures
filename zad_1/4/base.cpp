#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <cctype>

std::string normalise(const std::string& input) {
    std::string s;
    s.reserve(input.size());

    std::copy_if(input.begin(), input.end(), std::back_inserter(s),
        [](char c) {
            unsigned char u = c;
            return u >= 32 && u <= 126;
        });

    std::transform(s.begin(), s.end(), s.begin(),
        [](char c) {
            unsigned char u = c;
            if (std::ispunct(u)) return ',';
            if (std::isspace(u)) return ' ';
            return (char)std::tolower(u);
        });

    auto new_end = std::unique(s.begin(), s.end(),
        [](char a, char b) { return a == ' ' && b == ' '; });
    s.erase(new_end, s.end());

    if (!s.empty() && s.front() == ' ') s.erase(s.begin());

    return s;
}

struct Token {
    bool is_comma;
    bool space_before;
    std::string word;
};

std::string deduplicate(const std::string& s) {
    std::vector<Token> tokens;
    bool seen_space = false;
    for (auto it = s.begin(); it != s.end(); ) {
        if (*it == ',') {
            tokens.push_back({true, seen_space, ""});
            seen_space = false;
            ++it;
        } else if (*it == ' ') {
            seen_space = true;
            ++it;
        } else {
            auto word_end = std::find_if(it, s.end(),
                [](char c) { return c == ' ' || c == ','; });
            tokens.push_back({false, seen_space, std::string(it, word_end)});
            seen_space = false;
            it = word_end;
        }
    }

    auto new_end = std::unique(tokens.begin(), tokens.end(),
        [](const Token& a, const Token& b) {
            return !a.is_comma && !b.is_comma && a.word == b.word;
        });
    tokens.erase(new_end, tokens.end());

    std::string result;
    result.reserve(s.size());
    for (const auto& tok : tokens) {
        if (tok.space_before && !result.empty()) result += ' ';
        if (tok.is_comma) result += ',';
        else result += tok.word;
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

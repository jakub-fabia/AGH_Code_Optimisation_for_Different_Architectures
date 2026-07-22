#include <cstdio>
#include <cstring>
#include <cstdlib>

static inline bool is_ws(unsigned char u) {
    return u == ' ' || u == '\t' || u == '\n' || u == '\r' || u == '\v' || u == '\f';
}

static inline bool is_punct(unsigned char u) {
    return (u >= 33 && u <= 47) || (u >= 58 && u <= 64)
        || (u >= 91 && u <= 96) || (u >= 123 && u <= 126);
}

size_t normalise(const char* src, size_t n, char* dst) {
    char* w = dst;
    const char* end = src + n;
    for (const char* r = src; r < end; ++r) {
        unsigned char u = (unsigned char)*r;
        if (u < 32 || u > 126) continue;
        if (is_ws(u)) {
            if (w > dst && *(w - 1) != ' ') *w++ = ' ';
        } else if (is_punct(u)) {
            *w++ = ',';
        } else {
            *w++ = (u >= 'A' && u <= 'Z') ? (char)(u | 0x20) : (char)u;
        }
    }
    return (size_t)(w - dst);
}

size_t deduplicate(char* buf, size_t n) {
    char* w = buf;
    const char* r = buf;
    const char* end = buf + n;
    const char* prev = nullptr;
    size_t prev_len = 0;

    while (r < end) {
        if (*r == ',') {
            *w++ = ',';
            ++r;
            prev = nullptr;
            prev_len = 0;
            continue;
        }
        while (r < end && *r == ' ') ++r;
        const char* word_start = r;
        while (r < end && *r != ' ' && *r != ',') ++r;
        if (word_start == r) break;
        size_t word_len = (size_t)(r - word_start);

        bool same = prev && prev_len == word_len &&
                    std::memcmp(prev, word_start, word_len) == 0;
        if (!same) {
            if (word_start > buf && *(word_start - 1) == ' ') *w++ = ' ';
            if (w != word_start) {
                std::memmove(w, word_start, word_len);
            }
            prev = w;
            prev_len = word_len;
            w += word_len;
        }
    }
    return (size_t)(w - buf);
}

int main() {
    FILE* f = std::fopen("file.txt", "rb");
    if (!f) return 1;
    std::fseek(f, 0, SEEK_END);
    long fsize = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    char* input = (char*)std::malloc((size_t)fsize);
    std::fread(input, 1, (size_t)fsize, f);
    std::fclose(f);

    char* buffer = (char*)std::malloc((size_t)fsize);
    size_t n = normalise(input, (size_t)fsize, buffer);
    size_t out_len = deduplicate(buffer, n);
    std::fwrite(buffer, 1, out_len, stdout);
    std::fputc('\n', stdout);

    std::free(buffer);
    std::free(input);
    return 0;
}

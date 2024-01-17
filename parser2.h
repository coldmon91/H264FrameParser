#include <stdint.h>
inline int is_startcode(const char* data) {
    if (memcmp(data, "\x00\x00\x00\x01", 4) ==0) {
        return 4;
    }
    else if (memcmp(data, "\x00\x00\x01", 3) == 0) {
        return 3;
    }
    return 0;
}
void find_next_nal(uint64_t prev_nal_index, const char* data, uint64_t size, std::vector<std::vector<char>>& frames) {
    uint64_t curr_index = prev_nal_index + 3;
    while (curr_index < size) {
        short startcode = is_startcode(&data[curr_index]);
        if (startcode > 0) {
            if (curr_index > prev_nal_index) {
                frames.push_back(std::vector<char>(&data[prev_nal_index], &data[curr_index]));
            }
            prev_nal_index = curr_index;
            find_next_nal(prev_nal_index, data, size, frames);
            return;
        }
        else {
            curr_index++;
        }
    }
}
void parse_h26x(int index, const char* data, int size, std::vector<std::vector<char>>& frames) {
    int begin_index=0;
    while (begin_index < size) {
        int startcode = is_startcode(&data[begin_index]);
        if (startcode > 0) {
            find_next_nal(0, &data[0], size - 0, frames);
            return;
        }
        begin_index++;
    }
}

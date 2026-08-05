#include "../include/indexed_file.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

int main() {
    const std::string path = "test.bifs";
    IndexedFile idx;

    if (!idx.create(path, 16, 32)) {
        std::cerr << "Failed to create file\n";
        return 1;
    }

    if (!idx.open(path)) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    // write record 0
    std::vector<uint8_t> data(32, 0);
    std::string s = "hello world";
    std::copy(s.begin(), s.end(), data.begin());
    if (!idx.writeRecord(0, data)) {
        std::cerr << "writeRecord failed\n";
    }

    // read record 0
    std::vector<uint8_t> out;
    if (idx.readRecord(0, out)) {
        std::cout << "Record 0: " << std::string(out.begin(), out.end()) << "\n";
    } else {
        std::cerr << "readRecord failed\n";
    }

    idx.close();
    return 0;
}

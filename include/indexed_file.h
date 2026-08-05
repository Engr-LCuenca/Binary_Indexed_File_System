#pragma once

#include <string>
#include <vector>
#include <cstdint>

// Simple on-disk indexed file interface
// Layout (binary):
// [Header]
//  - magic (4 bytes)
//  - version (1 byte)
//  - record_size (4 bytes)
//  - capacity (4 bytes)
//  - index_offset (8 bytes)
//  - data_offset (8 bytes)
// [Index area] (capacity entries)
//  - for each entry: 8 bytes offset (0 = empty)
// [Data area]
//  - fixed-size records

struct IndexedFileHeader {
    uint32_t magic;       // 'BIFS'
    uint8_t version;      // format version
    uint32_t record_size; // size of each record
    uint32_t capacity;    // max number of records
    uint64_t index_offset; // file offset where index starts
    uint64_t data_offset;  // file offset where data area starts
};

class IndexedFile {
public:
    IndexedFile() = default;
    ~IndexedFile();

    // create a new indexed file with given capacity and record_size
    bool create(const std::string &path, uint32_t capacity, uint32_t record_size);

    // open existing file
    bool open(const std::string &path);

    // close file
    void close();

    // write a record at logical index (0..capacity-1). data must be record_size bytes
    bool writeRecord(uint32_t index, const std::vector<uint8_t> &data);

    // read a record into data buffer
    bool readRecord(uint32_t index, std::vector<uint8_t> &out);

    uint32_t capacity() const { return header_.capacity; }
    uint32_t recordSize() const { return header_.record_size; }

private:
    IndexedFileHeader header_{};
    int fd_ = -1; // file descriptor (platform specific)
    std::string path_;
};

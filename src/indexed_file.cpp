#include "../include/indexed_file.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <cstdint>

static const uint32_t BIFS_MAGIC = 0x42494653; // 'BIFS'

IndexedFile::~IndexedFile() {
    close();
}

bool IndexedFile::create(const std::string &path, uint32_t capacity, uint32_t record_size) {
    // create and open file
    int fd = ::open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    if (fd < 0) return false;

    header_.magic = BIFS_MAGIC;
    header_.version = 1;
    header_.record_size = record_size;
    header_.capacity = capacity;

    // compute offsets: header + index
    header_.index_offset = sizeof(IndexedFileHeader);
    header_.data_offset = header_.index_offset + capacity * sizeof(uint64_t);

    // write header
    if (::write(fd, &header_, sizeof(header_)) != (ssize_t)sizeof(header_)) {
        ::close(fd);
        return false;
    }

    // initialize index with zeros
    std::vector<uint64_t> zeroIndex(capacity, 0);
    if (::write(fd, zeroIndex.data(), zeroIndex.size() * sizeof(uint64_t)) < 0) {
        ::close(fd);
        return false;
    }

    // extend file to hold data area
    off_t dataSize = static_cast<off_t>(capacity) * header_.record_size;
    if (::lseek(fd, header_.data_offset + dataSize - 1, SEEK_SET) == (off_t)-1) {
        ::close(fd);
        return false;
    }
    if (::write(fd, "\0", 1) != 1) {
        ::close(fd);
        return false;
    }

    ::close(fd);
    return true;
}

bool IndexedFile::open(const std::string &path) {
    fd_ = ::open(path.c_str(), O_RDWR);
    if (fd_ < 0) return false;

    // read header
    if (::read(fd_, &header_, sizeof(header_)) != (ssize_t)sizeof(header_)) {
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    if (header_.magic != BIFS_MAGIC) {
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    path_ = path;
    return true;
}

void IndexedFile::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool IndexedFile::writeRecord(uint32_t index, const std::vector<uint8_t> &data) {
    if (fd_ < 0) return false;
    if (index >= header_.capacity) return false;
    if (data.size() != header_.record_size) return false;

    // read index entry
    off_t idxPos = header_.index_offset + index * sizeof(uint64_t);
    uint64_t dataOffset = 0;
    if (::lseek(fd_, idxPos, SEEK_SET) == (off_t)-1) return false;
    if (::read(fd_, &dataOffset, sizeof(dataOffset)) != (ssize_t)sizeof(dataOffset)) return false;

    // if empty, allocate at next free slot (we will just compute offset = data_offset + index * record_size)
    if (dataOffset == 0) {
        dataOffset = header_.data_offset + static_cast<uint64_t>(index) * header_.record_size;
        // write back index
        if (::lseek(fd_, idxPos, SEEK_SET) == (off_t)-1) return false;
        if (::write(fd_, &dataOffset, sizeof(dataOffset)) != (ssize_t)sizeof(dataOffset)) return false;
    }

    // write data at dataOffset
    if (::lseek(fd_, static_cast<off_t>(dataOffset), SEEK_SET) == (off_t)-1) return false;
    if (::write(fd_, data.data(), data.size()) != (ssize_t)data.size()) return false;

    return true;
}

bool IndexedFile::readRecord(uint32_t index, std::vector<uint8_t> &out) {
    if (fd_ < 0) return false;
    if (index >= header_.capacity) return false;

    off_t idxPos = header_.index_offset + index * sizeof(uint64_t);
    uint64_t dataOffset = 0;
    if (::lseek(fd_, idxPos, SEEK_SET) == (off_t)-1) return false;
    if (::read(fd_, &dataOffset, sizeof(dataOffset)) != (ssize_t)sizeof(dataOffset)) return false;

    if (dataOffset == 0) return false; // empty

    out.resize(header_.record_size);
    if (::lseek(fd_, static_cast<off_t>(dataOffset), SEEK_SET) == (off_t)-1) return false;
    if (::read(fd_, out.data(), out.size()) != (ssize_t)out.size()) return false;

    return true;
}

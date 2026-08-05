# Binary Indexed File System (BIFS)

A minimal, educational indexed binary file format and a small C++ library to create/read/write fixed-size records with an on-disk index.

## Goals

- Demonstrate an on-disk indexed layout with a simple header, index area and data area.
- Provide a tiny, portable C++ implementation suitable for learning and small experiments.
- Serve as a portfolio piece showing systems design, binary formats, and low-level I/O.

## On-disk layout

```
[Header]
 - magic (4 bytes) 'BIFS'
 - version (1 byte)
 - record_size (4 bytes)
 - capacity (4 bytes)
 - index_offset (8 bytes)
 - data_offset (8 bytes)
[Index area] (capacity entries)
 - each entry: uint64_t data_offset (0 means unused)
[Data area]
 - fixed-size records stored at data_offset
```

## Features

- Create a BIFS file with a fixed capacity and record size.
- Write/read fixed-size records by logical index.
- The index holds file offsets to records (0 indicates empty).

## Usage

Build with a standard g++ toolchain:

```bash
mkdir build && cd build
cmake ..
make
```

Run the included test harness:

```bash
./test/main
```

## Notes and future work

- Currently records are fixed-size. Support for variable-size records and free-list management can be added.
- Add concurrency controls and simple transactional semantics for robustness.
- Provide higher-level APIs for key-value mapping and compaction.

## Files

- `include/indexed_file.h` - public header
- `src/indexed_file.cpp` - implementation
- `test/main.cpp` - small test harness

## License

MIT

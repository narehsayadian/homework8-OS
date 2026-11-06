# Operating Systems — Homework 8

**Student:** Nareh Sayadian  
**Course:** Operating Systems  
**Assignment:** Homework 8 — File System Operations  
**Programming Language:** C

## Overview

This repository contains my solutions for Operating Systems Homework 8. The assignment focuses on file handling and POSIX system calls in C.

The programs demonstrate how to create, read, write, modify, compare, and manage files using low-level system calls.

## Tasks

| File | Description |
|---|---|
| `task1.c` | Copy the contents of one file into another |
| `task2.c` | Truncate a file and display its remaining contents |
| `task3.c` | Read and display a file in reverse order |
| `task4.c` | Append log entries with the process ID |
| `task5.c` | Create a sparse file using `lseek()` |
| `task6.c` | Replace the fourth line of a file while preserving the remaining contents |
| `task7.c` | Compare two files byte by byte |

## Technologies and System Calls

- C programming language
- POSIX file operations
- `open()`
- `read()`
- `write()`
- `close()`
- `lseek()`
- `ftruncate()`
- `getpid()`

## Compilation

Compile an individual task using GCC:

```bash
gcc -Wall -Wextra -std=c11 task1.c -o task1
```

Replace `task1` with the desired task number from 1 to 7.

To compile all seven programs:

```bash
for i in {1..7}; do
    gcc -Wall -Wextra -std=c11 task$i.c -o task$i || break
done
```

## Running the Programs

For example:

```bash
./task1
```

Some programs request file paths or user input. Others use predefined filenames such as `data.txt`, `log.txt`, `sparse.bin`, and `numbers.txt`.

## Testing

All seven programs were compiled using GCC with the `-Wall -Wextra -std=c11` flags and tested with sample inputs.

The tests covered file copying, truncation, reverse reading, appending log entries, sparse file creation, line replacement, and byte-wise comparison.

---

**Repository:** [homework8-OS](https://github.com/narehsayadian/homework8-OS)

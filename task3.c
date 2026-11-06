
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define PATH_SIZE 256

// Read a file path from standard input
int read_line(char *buffer, int size) {
    int i = 0;
    char ch;
    ssize_t n;

    while (1) {
        n = read(STDIN_FILENO, &ch, 1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0) {
            if (i == 0)
                return 0;
            break;
        }

        if (ch == '\n')
            break;

        if (i >= size - 1) {
            errno = ENAMETOOLONG;
            return -1;
        }

        buffer[i++] = ch;
    }

    buffer[i] = '\0';
    return 1;
}

// Write all bytes, handling partial writes
int write_all(int fd, const char *buffer, size_t count) {
    size_t written = 0;

    while (written < count) {
        ssize_t n = write(fd, buffer + written, count - written);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0) {
            errno = EIO;
            return -1;
        }

        written += n;
    }

    return 0;
}

int main() {
    int fd;
    char path[PATH_SIZE];
    char ch;
    ssize_t bytes_read;
    off_t size;

    // Ask the user for the file path
    printf("Enter file path: ");
    fflush(stdout);

    if (read_line(path, PATH_SIZE) != 1 || path[0] == '\0') {
        fprintf(stderr, "Error: Invalid file path.\n");
        return 1;
    }

    // Open the file in read-only mode
    fd = open(path, O_RDONLY);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // Get the file size
    size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    // Read and print bytes in reverse order
    for (off_t pos = size; pos > 0; ) {
        pos--;

        if (lseek(fd, pos, SEEK_SET) == -1) {
            perror("Error moving file position");
            close(fd);
            return 1;
        }

        do {
            bytes_read = read(fd, &ch, 1);
        } while (bytes_read == -1 && errno == EINTR);

        if (bytes_read == -1) {
            perror("Error reading file");
            close(fd);
            return 1;
        }

        if (bytes_read == 0) {
            fprintf(stderr, "Error: Unexpected end of file.\n");
            close(fd);
            return 1;
        }

        if (write_all(STDOUT_FILENO, &ch, 1) == -1) {
            perror("Error printing character");
            close(fd);
            return 1;
        }
    }

    // Print a newline (also handles empty files)
    if (write_all(STDOUT_FILENO, "\n", 1) == -1) {
        perror("Error printing newline");
        close(fd);
        return 1;
    }

    // Close the file
    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}

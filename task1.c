
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define PATH_SIZE 256

// Read one line from standard input
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
    char source[PATH_SIZE];
    char destination[PATH_SIZE];
    char buffer[BUFFER_SIZE];

    int source_fd, destination_fd;
    ssize_t bytes_read;
    unsigned long long total = 0;

    // Get source path
    printf("Enter source file path: ");
    fflush(stdout);

    if (read_line(source, PATH_SIZE) != 1 || source[0] == '\0') {
        fprintf(stderr, "Error: Invalid source path.\n");
        return 1;
    }

    // Get destination path
    printf("Enter destination file path: ");
    fflush(stdout);

    if (read_line(destination, PATH_SIZE) != 1 ||
        destination[0] == '\0') {
        fprintf(stderr, "Error: Invalid destination path.\n");
        return 1;
    }

    // Open source file for reading
    source_fd = open(source, O_RDONLY);

    if (source_fd == -1) {
        perror("Error opening source file");
        return 1;
    }

    // Open/create destination file
    destination_fd = open(destination,
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);

    if (destination_fd == -1) {
        perror("Error opening destination file");
        close(source_fd);
        return 1;
    }

    // Read and copy until EOF
    while (1) {
        bytes_read = read(source_fd, buffer, BUFFER_SIZE);

        if (bytes_read < 0) {
            if (errno == EINTR)
                continue;

            perror("Error reading source file");
            close(source_fd);
            close(destination_fd);
            return 1;
        }

        if (bytes_read == 0)
            break;

        if (write_all(destination_fd, buffer, bytes_read) == -1) {
            perror("Error writing destination file");
            close(source_fd);
            close(destination_fd);
            return 1;
        }

        total += bytes_read;
    }

    // Close both files
    if (close(source_fd) == -1) {
        perror("Error closing source file");
        close(destination_fd);
        return 1;
    }

    if (close(destination_fd) == -1) {
        perror("Error closing destination file");
        return 1;
    }

    printf("File copied successfully!\n");
    printf("Total bytes copied: %llu\n", total);

    return 0;
}

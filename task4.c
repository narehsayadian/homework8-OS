
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024

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
    char line[BUFFER_SIZE];
    char output[BUFFER_SIZE + 100];
    char ch;
    int i = 0;
    int prefix_length;
    ssize_t n;
    off_t offset;

    // Read one line from standard input using read()
    printf("Enter a log message: ");
    fflush(stdout);

    while (1) {
        n = read(0, &ch, 1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("Error reading input");
            return 1;
        }

        if (n == 0 || ch == '\n')
            break;

        if (i >= BUFFER_SIZE - 1) {
            fprintf(stderr, "Error: Input line is too long.\n");
            return 1;
        }

        line[i++] = ch;
    }

    // Open log.txt in append mode
    fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd == -1) {
        perror("Error opening log file");
        return 1;
    }

    // Add PID prefix
    prefix_length = snprintf(output, sizeof(output),
                             "PID=%ld: ", (long)getpid());

    if (prefix_length < 0 ||
        prefix_length >= (int)sizeof(output)) {
        fprintf(stderr, "Error creating log prefix.\n");
        close(fd);
        return 1;
    }

    // Add the user's message
    for (int j = 0; j < i; j++) {
        output[prefix_length + j] = line[j];
    }

    // Add newline
    output[prefix_length + i] = '\n';

    // Write the complete log entry
    if (write_all(fd, output, prefix_length + i + 1) == -1) {
        perror("Error writing log file");
        close(fd);
        return 1;
    }

    // Get the current file offset
    // O_APPEND moves the file offset to the end before each write.
    // After writing, the offset is updated to the end of our entry.
    // Therefore, SEEK_CUR returns the growing file offset.
    offset = lseek(fd, 0, SEEK_CUR);

    if (offset == -1) {
        perror("Error getting file offset");
        close(fd);
        return 1;
    }

    printf("Log entry added successfully!\n");
    printf("Final file offset: %lld\n", (long long)offset);

    // Close the file
    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

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
    char buffer[100];
    ssize_t bytes_read;

    // Open or create data.txt and write the alphabet
    fd = open("data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    if (write_all(fd, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26) == -1) {
        perror("Error writing file");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    // Reopen the file for reading and writing
    fd = open("data.txt", O_RDWR);

    if (fd == -1) {
        perror("Error reopening file");
        return 1;
    }

    // Get and print the original file size
    off_t size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("Error checking original size");
        close(fd);
        return 1;
    }

    printf("Original file size: %lld bytes\n", (long long)size);

    // Truncate the file to 10 bytes
    if (ftruncate(fd, 10) == -1) {
        perror("Error truncating file");
        close(fd);
        return 1;
    }

    // Get and print the new file size
    size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("Error checking new size");
        close(fd);
        return 1;
    }

    printf("New file size: %lld bytes\n", (long long)size);

    // Return to the beginning of the file
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Error rewinding file");
        close(fd);
        return 1;
    }

    // Read the remaining content and print it
    printf("Remaining content: ");
    fflush(stdout); 

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write_all(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("Error printing content");
            close(fd);
            return 1;
        }
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    printf("\n");

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}



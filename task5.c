
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
    off_t size;

    // Create the sparse file
    fd = open("sparse.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error creating file");
        return 1;
    }

    // Write START at the beginning
    if (write_all(fd, "START", 5) == -1) {
        perror("Error writing START");
        close(fd);
        return 1;
    }

    // Skip forward by 1 MiB without writing zeros
    if (lseek(fd, 1024 * 1024, SEEK_CUR) == -1) {
        perror("Error seeking forward");
        close(fd);
        return 1;
    }

    // Write END at the new position
    if (write_all(fd, "END", 3) == -1) {
        perror("Error writing END");
        close(fd);
        return 1;
    }

    // Close the file
    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    // Reopen the file in read-only mode
    fd = open("sparse.bin", O_RDONLY);

    if (fd == -1) {
        perror("Error reopening file");
        return 1;
    }

    // Get the apparent file size
    size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    printf("Apparent file size: %lld bytes\n", (long long)size);

    // The file contains a hole that was created by lseek().
    // The filesystem does not need to allocate disk blocks for this hole.
    // Therefore, du may report much less disk usage than the apparent size.

    // Close the file
    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}

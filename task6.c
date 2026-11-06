
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024

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
    char line[32];
    char ch;
    char remainder[BUFFER_SIZE];
    size_t remainder_size = 0;
    int newline_count = 0;
    off_t fourth_line_offset = 0;

    // Step 1: Create the file and write numbers 1 through 10
    fd = open("numbers.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error creating file");
        return 1;
    }

    for (int i = 1; i <= 10; i++) {
        int length = snprintf(line, sizeof(line), "%d\n", i);

        if (write_all(fd, line, length) == -1) {
            perror("Error writing numbers");
            close(fd);
            return 1;
        }
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    // Step 2: Reopen the file for reading and writing
    fd = open("numbers.txt", O_RDWR);

    if (fd == -1) {
        perror("Error reopening file");
        return 1;
    }

    // Step 3: Find the beginning of line 4 by counting newlines
    while (newline_count < 3) {
        ssize_t n = read(fd, &ch, 1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("Error scanning file");
            close(fd);
            return 1;
        }

        if (n == 0) {
            fprintf(stderr, "Error: fourth line not found\n");
            close(fd);
            return 1;
        }

        if (ch == '\n')
            newline_count++;
    }

    // The current position is now the beginning of line 4
    fourth_line_offset = lseek(fd, 0, SEEK_CUR);

    if (fourth_line_offset == -1) {
        perror("Error getting line offset");
        close(fd);
        return 1;
    }

    // Step 4: Skip the old fourth line
    while (1) {
        ssize_t n = read(fd, &ch, 1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("Error reading fourth line");
            close(fd);
            return 1;
        }

        if (n == 0 || ch == '\n')
            break;
    }

    // Step 5: Save all content after the fourth line
    while (1) {
        ssize_t n = read(fd, remainder + remainder_size,
                         sizeof(remainder) - remainder_size);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("Error reading remaining content");
            close(fd);
            return 1;
        }

        if (n == 0)
            break;

        remainder_size += n;

        if (remainder_size == sizeof(remainder)) {
            fprintf(stderr, "Error: remainder buffer is too small\n");
            close(fd);
            return 1;
        }
    }

    // Step 6: Return to the beginning of line 4
    if (lseek(fd, fourth_line_offset, SEEK_SET) == -1) {
        perror("Error seeking to fourth line");
        close(fd);
        return 1;
    }

    // Step 7: Write the replacement and restore the remaining content
    if (write_all(fd, "100\n", 4) == -1) {
        perror("Error writing replacement");
        close(fd);
        return 1;
    }

    if (write_all(fd, remainder, remainder_size) == -1) {
        perror("Error restoring remaining content");
        close(fd);
        return 1;
    }

    // Step 8: Print the final file contents
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Error returning to beginning of file");
        close(fd);
        return 1;
    }

    printf("Final file contents:\n");
    fflush(stdout);

    while (1) {
        ssize_t n = read(fd, remainder, sizeof(remainder));

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("Error reading final file");
            close(fd);
            return 1;
        }

        if (n == 0)
            break;

        if (write_all(STDOUT_FILENO, remainder, n) == -1) {
            perror("Error printing file contents");
            close(fd);
            return 1;
        }
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}

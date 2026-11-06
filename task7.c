
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define PATH_SIZE 512

// Read one line from standard input
int read_line(char *buffer, size_t size) {
    size_t i = 0;
    char ch;

    while (1) {
        ssize_t n = read(STDIN_FILENO, &ch, 1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0 || ch == '\n')
            break;

        if (i >= size - 1) {
            errno = ENAMETOOLONG;
            return -1;
        }

        buffer[i++] = ch;
    }

    buffer[i] = '\0';
    return 0;
}

// Fill a buffer unless the end of the file is reached
int read_chunk(int fd, char *buffer, size_t capacity,
               size_t *count, int *eof) {
    *count = 0;
    *eof = 0;

    while (*count < capacity) {
        ssize_t n = read(fd, buffer + *count, capacity - *count);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0) {
            *eof = 1;
            break;
        }

        *count += n;
    }

    return 0;
}

int main() {
    int fd1, fd2;
    char path1[PATH_SIZE];
    char path2[PATH_SIZE];
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];

    off_t position = 0;

    size_t count1, count2;
    int eof1, eof2;

    // Ask for the first file path
    printf("Enter the first file path: ");
    fflush(stdout);

    if (read_line(path1, sizeof(path1)) == -1 || path1[0] == '\0') {
        perror("Invalid first file path");
        return 1;
    }

    // Ask for the second file path
    printf("Enter the second file path: ");
    fflush(stdout);

    if (read_line(path2, sizeof(path2)) == -1 || path2[0] == '\0') {
        perror("Invalid second file path");
        return 1;
    }

    // Open the first file
    fd1 = open(path1, O_RDONLY);

    if (fd1 == -1) {
        perror("Error opening first file");
        return 1;
    }

    // Open the second file
    fd2 = open(path2, O_RDONLY);

    if (fd2 == -1) {
        perror("Error opening second file");
        close(fd1);
        return 1;
    }

    // Compare both files in chunks
    while (1) {
        if (read_chunk(fd1, buffer1, BUFFER_SIZE,
                       &count1, &eof1) == -1) {
            perror("Error reading first file");
            close(fd1);
            close(fd2);
            return 1;
        }

        if (read_chunk(fd2, buffer2, BUFFER_SIZE,
                       &count2, &eof2) == -1) {
            perror("Error reading second file");
            close(fd1);
            close(fd2);
            return 1;
        }

        // Compare bytes one by one
        size_t limit = count1 < count2 ? count1 : count2;

        for (size_t i = 0; i < limit; i++) {
            if (buffer1[i] != buffer2[i]) {
                printf("Files differ at byte %lld\n",
                       (long long)(position + i));

                close(fd1);
                close(fd2);
                return 1;
            }
        }

        // Different chunk lengths mean one file ended first
        if (count1 != count2) {
            printf("Files differ at byte %lld\n",
                   (long long)(position + limit));

            close(fd1);
            close(fd2);
            return 1;
        }

        // Both files ended with the same content
        if (eof1 && eof2)
            break;

        position += count1;
    }

    printf("Files are identical\n");

    if (close(fd1) == -1) {
        perror("Error closing first file");
        close(fd2);
        return 1;
    }

    if (close(fd2) == -1) {
        perror("Error closing second file");
        return 1;
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024*(EVENT_SIZE + 16))
#define LOG_FILE "client_log.txt"

// Function to log an action with its corresponding danger level
void log_action(const char *action, int danger) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(log_file, "Action: %s, Danger Level: %d\n", action, danger);
    fclose(log_file);
}

int main() {
    int fd, wd;
    char buffer[BUF_LEN];

    // Connect to the server and send initial data folder information
    // Wait for acknowledgement from the server

    // Initialize inotify to monitor changes in the data folder and subfolders
    fd = inotify_init();
    if (fd < 0) {
        log_action("inotify failled, try later", 1);
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    wd = inotify_add_watch(fd, "data", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
    if (wd < 0) {
        log_action("inotify watch failled, try later", 1);
        perror("inotify_add_watch failled: ");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int length = read(fd, buffer, BUF_LEN);

        if (length < 0) {
        log_action("inotify read action, try later", 1);
            perror("read failled: ");
            exit(EXIT_FAILURE);
        }
 
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO)) {
                    // Capture the new tree path of data and subfolders
                    // Send it to the server in the specified data format
                    log_action("File operation", 3); // Example danger level
                } else if (event->mask & IN_MODIFY) {
                    // Capture the new tree path of data and subfolders
                    // Send it to the server in the specified data format
                    log_action("File modification", 2); // Example danger level
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    // Close connection to the server

    return 0;
}
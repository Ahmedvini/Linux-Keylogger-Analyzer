#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#define LOG_FILE "keylog_c.txt"
#define EVENT_DIR "/dev/input"
#define EVENT_PREFIX "event"
#define MAX_KEYS 256

volatile sig_atomic_t keep_running = 1;
void int_handler(int dummy) { keep_running = 0; }

int is_keyboard(const char *name) {
    return strstr(name, "kbd") || strstr(name, "keyboard") || strstr(name, "AT Translated");
}

void list_devices() {
    DIR *dir = opendir(EVENT_DIR);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    struct dirent *entry;
    int idx = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, EVENT_PREFIX, strlen(EVENT_PREFIX)) == 0) {
            char path[256];
            snprintf(path, sizeof(path), "%s/%s", EVENT_DIR, entry->d_name);
            printf("[%d] %s\n", idx, path);
            idx++;
        }
    }
    closedir(dir);
}

char* select_device() {
    static char path[256];
    int choice, idx = 0;
    char *devices[64];
    DIR *dir = opendir(EVENT_DIR);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, EVENT_PREFIX, strlen(EVENT_PREFIX)) == 0) {
            snprintf(path, sizeof(path), "%s/%s", EVENT_DIR, entry->d_name);
            int fd = open(path, O_RDONLY);
            char name[256] = "";
            if (fd >= 0) {
                ioctl(fd, EVIOCGNAME(sizeof(name)), name);
                close(fd);
            }
            devices[idx] = strdup(path);
            printf("[%d] %s (%s)\n", idx, path, name);
            idx++;
        }
    }
    closedir(dir);
    if (idx == 0) {
        printf("No input devices found.\n");
        exit(EXIT_FAILURE);
    }
    printf("Select device number: ");
    scanf("%d", &choice);
    if (choice < 0 || choice >= idx) {
        printf("Invalid selection.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < idx; ++i) if (i != choice) free(devices[i]);
    return devices[choice];
}

void log_keypresses(const char *device_path) {
    int fd = open(device_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("fopen");
        close(fd);
        exit(EXIT_FAILURE);
    }
    struct input_event ev;
    int key_counts[MAX_KEYS] = {0};
    int total = 0;
    time_t session_start = time(NULL), session_end;
    printf("Logging from: %s\nPress Ctrl+C to stop.\n", device_path);
    signal(SIGINT, int_handler);
    while (keep_running) {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n == (ssize_t)sizeof(ev)) {
            if (ev.type == EV_KEY && ev.value == 1) { // key down
                char tbuf[64];
                time_t t = ev.time.tv_sec;
                struct tm *tm_info = localtime(&t);
                strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm_info);
                fprintf(log, "%s.%06ld %d\n", tbuf, (long)ev.time.tv_usec, ev.code);
                fflush(log);
                printf("%s.%06ld %d\n", tbuf, (long)ev.time.tv_usec, ev.code);
                key_counts[ev.code]++;
                total++;
            }
        }
    }
    session_end = time(NULL);
    int unique = 0;
    for (int i = 0; i < MAX_KEYS; ++i) if (key_counts[i]) unique++;
    printf("\nSession stats:\n  Duration: %ld seconds\n  Total keys: %d\n  Unique keys: %d\n", session_end-session_start, total, unique);
    fclose(log);
    close(fd);
}

int main() {
    printf("Available input devices:\n");
    char *device = select_device();
    log_keypresses(device);
    return 0;
}

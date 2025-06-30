#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#define MAX_KEYS 512
#define LINE_LEN 128

// Map key codes to names (partial, for demo)
const char *key_names[256] = {
    [1] = "ESC", [14] = "BACKSPACE", [15] = "TAB", [28] = "ENTER", [57] = "SPACE",
    [30] = "A", [31] = "S", [32] = "D", [33] = "F", [34] = "G", [35] = "H", [36] = "J", [37] = "K", [38] = "L",
    [16] = "Q", [17] = "W", [18] = "E", [19] = "R", [20] = "T", [21] = "Y", [22] = "U", [23] = "I", [24] = "O", [25] = "P",
    [44] = "Z", [45] = "X", [46] = "C", [47] = "V", [48] = "B", [49] = "N", [50] = "M"
};

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

// Fallback for strptime if missing (very basic, only for this format)
#ifndef HAVE_STRPTIME
char *strptime(const char *s, const char *format, struct tm *tm) {
    // Only supports "%Y-%m-%d %H:%M:%S"
    int y, M, d, h, m, sec;
    if (sscanf(s, "%d-%d-%d %d:%d:%d", &y, &M, &d, &h, &m, &sec) == 6) {
        tm->tm_year = y - 1900;
        tm->tm_mon = M - 1;
        tm->tm_mday = d;
        tm->tm_hour = h;
        tm->tm_min = m;
        tm->tm_sec = sec;
        return (char*)s + 19;
    }
    return NULL;
}
#endif

void export_csv(int *key_counts, int max_keys, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "keycode,count\n");
    for (int i = 0; i < max_keys; ++i) {
        if (key_counts[i]) fprintf(f, "%d,%d\n", i, key_counts[i]);
    }
    fclose(f);
    printf("Exported CSV: %s\n", filename);
}

void export_json(int *key_counts, int max_keys, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "{\n");
    int first = 1;
    for (int i = 0; i < max_keys; ++i) {
        if (key_counts[i]) {
            if (!first) fprintf(f, ",\n");
            fprintf(f, "  \"%d\": %d", i, key_counts[i]);
            first = 0;
        }
    }
    fprintf(f, "\n}\n");
    fclose(f);
    printf("Exported JSON: %s\n", filename);
}

void analyze_log(const char *logfile) {
    FILE *f = fopen(logfile, "r");
    if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
    int key_counts[MAX_KEYS] = {0};
    char line[LINE_LEN];
    int total = 0, unique = 0;
    time_t first = 0, last = 0;
    while (fgets(line, sizeof(line), f)) {
        char date[32];
        int code;
        long usec;
        if (sscanf(line, "%s %*s %ld %d", date, &usec, &code) < 2) {
            // Try alternate format
            if (sscanf(line, "%*s %*s %d", &code) < 1) continue;
        }
        key_counts[code]++;
        total++;
        // Parse time for speed
        struct tm tm = {0};
        char tbuf[32];
        if (sscanf(line, "%[^.]", tbuf) == 1) {
            strptime(tbuf, "%Y-%m-%d %H:%M:%S", &tm);
            time_t t = mktime(&tm);
            if (first == 0) first = t;
            last = t;
        }
    }
    fclose(f);
    for (int i = 0; i < MAX_KEYS; ++i) if (key_counts[i]) unique++;
    printf("Total keys: %d\nUnique keys: %d\n", total, unique);
    if (first && last && last > first) {
        double mins = (last - first) / 60.0;
        printf("Typing speed: %.2f keys/min\n", mins > 0 ? total / mins : 0);
    }
    printf("Top 10 keys:\n");
    for (int n = 0; n < 10; ++n) {
        int max = 0, idx = -1;
        for (int i = 0; i < MAX_KEYS; ++i) {
            if (key_counts[i] > max) { max = key_counts[i]; idx = i; }
        }
        if (idx == -1 || max == 0) break;
        printf("  %s (%d): %d\n", key_names[idx] ? key_names[idx] : "?", idx, max);
        key_counts[idx] = 0;
    }
    // Export options
    char ans[8];
    printf("Export stats as CSV? (y/n): ");
    fgets(ans, sizeof(ans), stdin);
    if (ans[0] == 'y' || ans[0] == 'Y') export_csv(key_counts, MAX_KEYS, "key_stats.csv");
    printf("Export stats as JSON? (y/n): ");
    fgets(ans, sizeof(ans), stdin);
    if (ans[0] == 'y' || ans[0] == 'Y') export_json(key_counts, MAX_KEYS, "key_stats.json");
}

int main(int argc, char *argv[]) {
    const char *logfile = argc > 1 ? argv[1] : "keylog_c.txt";
    analyze_log(logfile);
    return 0;
}

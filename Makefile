CC=gcc
CFLAGS=-Wall -O2

all: keylogger_c analyze_c

keylogger_c: keylogger.c
	$(CC) $(CFLAGS) -o keylogger_c keylogger.c

analyze_c: analyze.c
	$(CC) $(CFLAGS) -o analyze_c analyze.c

clean:
	rm -f keylogger_c analyze_c *.o keylog_c.txt

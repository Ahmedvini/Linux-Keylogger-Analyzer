# Linux Keylogger & Analyzer in C (Educational Use Only)

## Features
- Reads from /dev/input/event* using Linux input_event struct
- Logs keypresses with timestamps to a file (`keylog_c.txt`)
- Lists all input devices with their names for selection
- Session stats: duration, total/unique keys after logging
- Analyzer tool: shows most-used keys, typing speed, and can export stats as CSV/JSON

## Build
```
make
make analyze_c
```

## Run (requires root):
```
sudo ./keylogger_c
```
- If `sudo` is not available, use `su` to become root, then run `./keylogger_c`.
- Select your keyboard device from the list.
- Press keys, then Ctrl+C to stop and see session stats.

## Analyzer Tool
Build:
```
make analyze_c
```
Run:
```
./analyze_c [keylog_c.txt]
```
- Shows total/unique keys, typing speed, and top 10 keys.
- Prompts to export stats as CSV (`key_stats.csv`) or JSON (`key_stats.json`).

## Troubleshooting
- **Permission denied:** You must run as root to access `/dev/input/event*` devices.
- **No such file or directory:** Run the keylogger first to generate `keylog_c.txt`.
- **Device names blank:** Some devices may not provide a name; try another device if logging does not work.

## Ethical Notice
This project is for educational and ethical use only. Do not use it on systems without explicit permission.

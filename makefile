all: compile

compile:
	g++ -std=c++17 -DUNICODE -D_UNICODE clipboard_monitor.cpp -o clipboard_monitor.exe -luser32 -lshell32 -lgdi32 -lpsapi

clear:
	rm clipboard_monitor.exe
all: compile

compile:
	g++ -std=c++11 -o clipboard_monitor.exe clipboard_monitor.cpp -luser32 -lshell32
all: compile

compile:
	g++ -o clipboard clipboard.cpp -luser32
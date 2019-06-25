# Preliminary makefile for BitmapParser, authored by Jason Kim
all: bitmapparser.cpp
		g++ -std=c++11 -g3 -Wall -Werror -pedantic -o bitmapparser_test -I ./src/ bitmapparser.cpp
clean:
		rm bitmapparser_test

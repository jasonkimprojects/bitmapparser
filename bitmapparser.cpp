// Copyright 2019 Jason Kim. All rights reserved.
/*
bitmapparser.cpp

A simple library to read, write, and edit bitmap images. 
Written as a side project to study file processing, and for fun.

The style confroms to Google's coding style guide for C++,
and has been checked with cpplint.

Jason Kim
June 13, 2019
*/

#include "src/bitmapparser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Specify a filename!\n";
        return 1;
    }
    BitmapParser test(argv[1]);
    test.print_metadata(false);
    test.print_pixels(false);
    return 0;
}

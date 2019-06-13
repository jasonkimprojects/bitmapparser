// Copyright 2019 Jason Kim. All rights reserved.
/*
bitmapparser.h

A simple library to read, write, and edit bitmap images. 
Written as a side project to study file processing, and for fun.

The style confroms to Google's coding style guide for C++,
and has been checked with cpplint.

Jason Kim
June 13, 2019
*/

#ifndef SRC_BITMAPPARSER_H_
#define SRC_BITMAPPARSER_H_

// For printing
#include <iostream>
#include <iomanip>
// For file input and output.
#include <fstream>
#include <sstream>
// For storing bitmap information.
#include <vector>
// Explicit include for compatibility.
#include <string>
// For custom exceptions.
#include <exception>

// Constants for array length. Just for consistency.
const size_t TWO = 2;
const size_t FOUR = 4;

// For organizing the 14-byte header.
struct Header {
    unsigned char signature[TWO];
    unsigned char filesize[FOUR];
    unsigned char reserved[FOUR];
    unsigned char dataoffset[FOUR];
};

/*
For organizing the 40-byte info header.
Some bitmaps have negative height, where
the pixels are stored upside down.
*/
struct InfoHeader {
    unsigned char size[FOUR];
    unsigned char width[FOUR];
    unsigned char height[FOUR];
    unsigned char planes[TWO];
    unsigned char bits_per_pixel[TWO];
    unsigned char compression[FOUR];
    unsigned char image_size[FOUR];
    unsigned char xpixelsperm[FOUR];
    unsigned char ypixelsperm[FOUR];
    unsigned char colors_used[FOUR];
    unsigned char important_colors[FOUR];
};

// Custom exception for when the bitmap signature is wrong.
class InvalidFormatException : public std::exception {
    const char* what() const throw() override {
        return "Invalid file format, not a bitmap image!";
    }
};

// Custom exception when file fails to open.
class FileOpenException : public std::exception {
    const char* what() const throw() override {
        return "Failed to open file!";
    }
};

class BitmapParser {
 private:
     FILE* fileptr_;
     Header header_;
     InfoHeader infoheader_;
     std::vector<std::vector<unsigned char> > pixels_;

 public:
     // Constructor for C-string filename
    explicit BitmapParser(const char *filename) {
        fileptr_ = nullptr;
        import(filename);
    }
    // Overloaded ctor for C++ string filename
    explicit BitmapParser(const std::string& filename) {
        fileptr_ = nullptr;
        import(filename.c_str());
    }

    Header* get_header() {
        return &header_;
    }

    InfoHeader* get_infoheader() {
        return &infoheader_;
    }

    std::vector<std::vector<unsigned char> >& get_pixels() {
        return pixels_;
    }

    void import(const char *filename) {
        // Open and check for success.
        fileptr_ = fopen(filename, "rb");
        if (fileptr_ == nullptr) throw FileOpenException();
        // Read the header.
        fread(header_.signature, 1, TWO, fileptr_);
        fread(header_.filesize, 1, FOUR, fileptr_);
        fread(header_.reserved, 1, FOUR, fileptr_);
        fread(header_.dataoffset, 1, FOUR, fileptr_);
        // Read the info header.
        fread(infoheader_.size, 1, FOUR, fileptr_);
        fread(infoheader_.width, 1, FOUR, fileptr_);
        fread(infoheader_.height, 1, FOUR, fileptr_);
        fread(infoheader_.planes, 1, TWO, fileptr_);
        fread(infoheader_.bits_per_pixel, 1, TWO, fileptr_);
        fread(infoheader_.compression, 1, FOUR, fileptr_);
        fread(infoheader_.image_size, 1, FOUR, fileptr_);
        fread(infoheader_.xpixelsperm, 1, FOUR, fileptr_);
        fread(infoheader_.ypixelsperm, 1, FOUR, fileptr_);
        fread(infoheader_.colors_used, 1, FOUR, fileptr_);
        fread(infoheader_.important_colors, 1, FOUR, fileptr_);
        // Check the signature for "BM" in ASCII hexadecimal.
        if (to_hex_str(header_.signature, TWO) != "424d") {
            throw InvalidFormatException();
        }
        print_metadata();
        print_metadata_hex();
    }

    std::string to_hex_str(const unsigned char *ptr, const size_t size) {
        size_t increments = 0;
        std::stringstream stream;
        while (increments < size) {
            stream << std::setfill('0') << std::setw(2) <<
                std::hex << static_cast<int>(*ptr);
            ++ptr;
            ++increments;
        }
        return stream.str();
    }

    void print_metadata() {
        std::cout << "HEADER\n" <<
            "\nSignature: 0x" <<
            to_hex_str(header_.signature, TWO) <<
            "\nFile Size (Bytes): " <<
            *reinterpret_cast<int*>(header_.filesize) <<
            "\nReserved Flags: " <<
            *reinterpret_cast<int*>(header_.reserved) <<
            "\nData Offset (Bytes): " <<
            *reinterpret_cast<int*>(header_.dataoffset) <<
            "\n\nINFO HEADER\n" <<
            "\nInfo Header Size (Bytes): " <<
            *reinterpret_cast<int*>(infoheader_.size) <<
            "\nImage Width (Pixels): " <<
            *reinterpret_cast<int*>(infoheader_.width) <<
            "\nImage Height (Pixels): " <<
            *reinterpret_cast<int*>(infoheader_.height) <<
            "\nPlanes: " <<
            *reinterpret_cast<int16_t*>(infoheader_.planes) <<
            "\nBits Per Pixel: " <<
            *reinterpret_cast<int16_t*>(infoheader_.bits_per_pixel) <<
            "\nCompression Type: " <<
            *reinterpret_cast<int*>(infoheader_.compression) <<
            "\nCompressed Image Size (Bytes): " <<
            *reinterpret_cast<int*>(infoheader_.image_size) <<
            "\nHorizontal Resolution (Pixels/Meter): " <<
            *reinterpret_cast<int*>(infoheader_.xpixelsperm) <<
            "\nVertical Resolution (Pixels/Meter): " <<
            *reinterpret_cast<int*>(infoheader_.ypixelsperm) <<
            "\nNumber of Actually Used Colors: " <<
            *reinterpret_cast<int*>(infoheader_.colors_used) <<
            "\nNumber of Important Colors: " <<
            *reinterpret_cast<int*>(infoheader_.important_colors) << "\n\n";
    }

    void print_metadata_hex() {
        std::cout << "CAUTION - ON x86 SYSTEMS, \n" <<
            "EVERYTHING EXCEPT THE SIGNATURE IS LITTLE ENDIAN!\n\n" <<
            "HEADER\n" <<
            "\nSignature: 0x" <<
            to_hex_str(header_.signature, TWO) <<
            "\nFile Size (Bytes): 0x" <<
            to_hex_str(header_.filesize, FOUR) <<
            "\nReserved Flags: 0x" <<
            to_hex_str(header_.reserved, FOUR) <<
            "\nData Offset (Bytes): 0x" <<
            to_hex_str(header_.dataoffset, FOUR) <<
            "\n\nINFO HEADER\n" <<
            "\nInfo Header Size (Bytes): 0x" <<
            to_hex_str(infoheader_.size, FOUR) <<
            "\nImage Width (Pixels): 0x" <<
            to_hex_str(infoheader_.width, FOUR) <<
            "\nImage Height (Pixels): 0x" <<
            to_hex_str(infoheader_.height, FOUR) <<
            "\nPlanes: 0x" <<
            to_hex_str(infoheader_.planes, TWO) <<
            "\nBits Per Pixel: 0x" <<
            to_hex_str(infoheader_.bits_per_pixel, TWO) <<
            "\nCompression Type: 0x" <<
            to_hex_str(infoheader_.compression, FOUR) <<
            "\nCompressed Image Size (Bytes): 0x" <<
            to_hex_str(infoheader_.image_size, FOUR) <<
            "\nHorizontal Resolution (Pixels/Meter): 0x" <<
            to_hex_str(infoheader_.xpixelsperm, FOUR) <<
            "\nVertical Resolution (Pixels/Meter): 0x" <<
            to_hex_str(infoheader_.ypixelsperm, FOUR) <<
            "\nNumber of Actually Used Colors: 0x" <<
            to_hex_str(infoheader_.colors_used, FOUR) <<
            "\nNumber of Important Colors: 0x" <<
            to_hex_str(infoheader_.important_colors, FOUR) << "\n\n";
    }
};
#endif  // SRC_BITMAPPARSER_H_

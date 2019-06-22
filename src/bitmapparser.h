// Copyright 2019 Jason Kim. All rights reserved.
/*
bitmapparser.h

A simple library to read, write, and edit bitmap images. 
Written as a side project to study file processing, and for fun.
This library will only make basic and absolutely needed checks
and exceptions on user behavior, and trusts the user otherwise.
A consequence is that broken bitmap images are possible if,
for example, a pixels vector of different dimension is replaced
without changing data in the header and info header.

At the time, only 24-bit color (RGB, 0-255) is supported.

The style confroms to Google's coding style guide for C++,
and has been checked with cpplint.

Jason Kim
June 13, 2019
*/

#ifndef BITMAPPARSER_H_
#define BITMAPPARSER_H_

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

// For organizing the 14-byte header.
struct Header {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
};

/*
For organizing the 40-byte info header.
Some bitmaps have negative height, where
the pixels are stored upside down.
*/
struct InfoHeader {
    uint32_t size;
    uint32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_pixels_per_meter;
    uint32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t important_colors;
};

// For representing standard RGB pixels (0-255).
struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

// Custom exception for when the bitmap signature is wrong.
class InvalidFormatException : public std::exception {
    const char* what() const throw() override {
        return "Invalid file format, not a bitmap image!";
    }
};

// Custom exception for when the color scheme is not 24-bit.
class UnsupportedException : public std::exception {
    const char* what() const throw() override {
        return "Only 24-bit RGB is supported at this time.";
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
     // Constants for word/dword size in bytes
     static const size_t WORD = 2;
     static const size_t DWORD = 4;

     /*
     Containers per section of the bitmap file:
     File pointer, header, infoheader, and pixels vector.
     */
     FILE* _fileptr;
     Header _header;
     InfoHeader _infoheader;
     std::vector<std::vector<Pixel> > _pixels;

     // Helper method for importing the header.
     void import_header() {
         /*
           The signature is the only word.
           A buffer is needed to switch the endianness
           for the signature only, so it reads 42 4D not 4D 42.
           All other fields are little endian.
           */
         uint8_t word_buf[WORD];
         fread(word_buf, 1, WORD, _fileptr);
         _header.signature = static_cast<uint16_t>(word_buf[1]) |
             (static_cast<uint16_t>(word_buf[0]) << 8);
         fread(&(_header.file_size), 1, DWORD, _fileptr);
         fread(&(_header.reserved), 1, DWORD, _fileptr);
         fread(&(_header.data_offset), 1, DWORD, _fileptr);
     }

     // Helper method for importing the info header.
     void import_infoheader() {
         // Only planes and bits per pixel are words.
         fread(&(_infoheader.size), 1, DWORD, _fileptr);
         fread(&(_infoheader.width), 1, DWORD, _fileptr);
         fread(&(_infoheader.height), 1, DWORD, _fileptr);
         fread(&(_infoheader.planes), 1, WORD, _fileptr);
         fread(&(_infoheader.bits_per_pixel), 1, WORD, _fileptr);
         fread(&(_infoheader.compression), 1, DWORD, _fileptr);
         fread(&(_infoheader.image_size), 1, DWORD, _fileptr);
         fread(&(_infoheader.x_pixels_per_meter), 1, DWORD, _fileptr);
         fread(&(_infoheader.y_pixels_per_meter), 1, DWORD, _fileptr);
         fread(&(_infoheader.colors_used), 1, DWORD, _fileptr);
         fread(&(_infoheader.important_colors), 1, DWORD, _fileptr);
     }

     // Helper method for calculating the row padding.
     size_t row_padding() {
         // Each row must be a multiple of 4 bytes
         size_t remainder = (_infoheader.width * 3) % 4;
         if (remainder == 0) {
             return 0;
         } else {
             return (4 - remainder);
         }
     }

 public:
    // Default constructor
    BitmapParser()
        : _fileptr(nullptr), _header(Header()),
        _infoheader(InfoHeader()),
        _pixels(std::vector<std::vector<Pixel> >()) {}

    // Overloaded ctor for C-string filename
    explicit BitmapParser(const char *filename)
        : _fileptr(nullptr), _header(Header()),
        _infoheader(InfoHeader()),
        _pixels(std::vector<std::vector<Pixel> >()) {
            import(filename);
    }

    // Overloaded ctor for C++ string filename
    explicit BitmapParser(const std::string& filename)
        : _fileptr(nullptr), _header(Header()),
        _infoheader(InfoHeader()),
        _pixels(std::vector<std::vector<Pixel> >()) {
            import(filename.c_str());
    }

    // Accessor for header struct
    const Header& read_header() const {
        return _header;
    }

    // Mutator for header struct as reference
    Header& header() {
        return _header;
    }

    // Mutator for replacing header struct
    void replace_header(const Header& new_header) {
        // Shallow copy is fine, no pointers
        _header = new_header;
    }

    // Accessor for info header struct
    const InfoHeader& read_infoheader() const {
        return _infoheader;
    }

    // Mutator for info header struct as reference
    InfoHeader& infoheader() {
        return _infoheader;
    }

    // Mutator for replacing infoheader struct
    void replace_infoheader(const InfoHeader& new_infoheader) {
        // Shallow copy is fine, no pointers
        _infoheader = new_infoheader;
    }

    // Accessor for pixels
    const std::vector<std::vector<Pixel> >& read_pixels() const {
        return _pixels;
    }

    // Mutator for pixels vector as reference
    std::vector<std::vector<Pixel> >& pixels() {
        return _pixels;
    }

    // Mutator for replacing pixels vector
    void replace_pixels(const std::vector<std::vector<Pixel> >& new_pixels) {
        // Shallow copy is fine, no pointers
        _pixels = new_pixels;
    }

    // Reads and parses a bitmap file
    void import(const char *filename) {
        /*
        Open and check for success.
        FYI - Visual Studio debugger requires absolute path.
        */
        _fileptr = fopen(filename, "rb");
        if (_fileptr == nullptr) throw FileOpenException();
        // Import the header and info header via helpers.
        import_header();
        import_infoheader();
        /*
        After the 14-byte header and 40-byte info header,
        there is some padding of (3 bytes per pixel) * (width) zero bytes
        The pixel data begins immediately afterwards.
        Skip the padding by advancing the file pointer.
        */
        fseek(_fileptr, (3 * _infoheader.width), SEEK_CUR);
        // Calculate row padding via helper.
        size_t num_padding_bytes = row_padding();
        // Create vectors of Pixels by height (# of rows)
        _pixels.resize(_infoheader.height);
        // Reserve in each row for future Pixel push_back
        for (std::vector<Pixel> row : _pixels) {
            row.reserve(_infoheader.width);
        }
        // Finally, read the pixels.
        for (size_t row = 0; row < _infoheader.height; ++row) {
            for (size_t col = 0; col < _infoheader.width; ++col) {
                // Read R, G, B for each pixel
                Pixel pix = {};
                fread(&(pix.red), 1, 1, _fileptr);
                fread(&(pix.green), 1, 1, _fileptr);
                fread(&(pix.blue), 1, 1, _fileptr);
                _pixels[row].push_back(pix);
            }
            // Skip the row padding before moving to the next row
            fseek(_fileptr, num_padding_bytes, SEEK_CUR);
        }
    }

    // Prints information about the header and info header.
    void print_metadata(bool hex) const {
        // For displaying text dividers
        const std::string div = "========================================";
        if (hex) {
            std::cout << "Number base: hexadecimal\n\n";
        } else {
            std::cout << "Number base: decimal\n\n";
        }
        std::cout << "HEADER\n" << div <<
            "\nSignature (hexadecimal): 0x" <<
            std::hex << _header.signature;
        // Determine hex or decimal
        if (!hex) std::cout << std::dec;
        std::cout << "\nFile Size (Bytes): " <<
            _header.file_size <<
            "\nReserved Flags: " <<
            _header.reserved <<
            "\nData Offset (Bytes): " <<
            _header.data_offset <<
            "\n\nINFO HEADER\n" << div <<
            "\nInfo Header Size (Bytes): " <<
            _infoheader.size <<
            "\nImage Width (Pixels): " <<
            _infoheader.width <<
            "\nImage Height (Pixels): " <<
            _infoheader.height <<
            "\nPlanes: " <<
            _infoheader.planes <<
            "\nBits Per Pixel: " <<
            _infoheader.bits_per_pixel <<
            "\nCompression Type: " <<
            _infoheader.compression <<
            "\nCompressed Image Size (Bytes): " <<
            _infoheader.image_size <<
            "\nHorizontal Resolution (Pixels/Meter): " <<
            _infoheader.x_pixels_per_meter <<
            "\nVertical Resolution (Pixels/Meter): " <<
            _infoheader.y_pixels_per_meter <<
            "\nNumber of Actually Used Colors: " <<
            _infoheader.colors_used <<
            "\nNumber of Important Colors: " <<
            _infoheader.important_colors << "\n\n";
    }

    /*
    Prints information about the pixels vector.
    Output may be long - recommended to pipe to file.
    */
    void print_pixels(bool hex) const {
        for (size_t row = 0; row < _infoheader.height; ++row) {
            for (size_t col = 0; col < _infoheader.width; ++col) {
                std::cout << std::dec << "Row " << row <<
                    ", Column " << col << "\n";
                const Pixel& pix = _pixels[row][col];
                if (hex) std::cout << std::hex;
                // Cout can't print uint8_t without unsigned()
                std::cout << "R: " << unsigned(pix.red) <<
                    ", G: " << unsigned(pix.green) <<
                    ", B: " << unsigned(pix.blue) << "\n\n";
            }
        }
    }
};
#endif  // BITMAPPARSER_H_

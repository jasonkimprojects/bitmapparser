// Copyright 2019 Jason Kim. All rights reserved.
/*
bitmapparser.h

A simple library to read, write, and edit bitmap images. 
Written as a side project to study file processing, and for fun.
This library will only make basic and absolutely needed checks
and exceptions on user behavior, and trusts the user otherwise.

At the time, only 24-bit color (RGB, 0-255) without compression is supported.

The style confroms to Google's coding style guide for C++,
and has been checked with cpplint.

Jason Kim
June 13, 2019
*/

#ifndef BITMAPPARSER_H_
#define BITMAPPARSER_H_

// For printing.
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

// For organizing the 40-byte info header.
struct InfoHeader {
    uint32_t size;
    uint32_t width;
    uint32_t height;
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
        // Workaround for 80 char column limit.
        std::string msg = "Invalid or incompatible file.\n";
        msg += "Only 24-bit uncompressed files are supported.\n";
        return msg.c_str();
    }
};

// Custom exception when file fails to open.
class FileOpenException : public std::exception {
    const char* what() const throw() override {
        return "Failed to open file!\n";
    }
};

// Custom exception for unexpected end-of-file.
class EOFException : public std::exception {
    const char* what() const throw() override {
        return "Unexpectedly reached end of file!\n";
    }
};

// Custom exception for errors in file I/O.
class IOException : public std::exception {
    const char* what() const throw() override {
        return "Error reading or writing file!\n";
    }
};

class BitmapParser {
 private:
     // Constants for word/dword size in bytes.
     static const size_t BYTE = 1;
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
     size_t _padding;

     /*
     Wrapper for reading and checking fread.
     No need to check return value of fread, since feof/ferror
     flags are set automatically.
     */
     void check_read(void* buffer, size_t size, size_t count, FILE* stream) {
         fread(buffer, size, count, stream);
         if (feof(stream)) throw EOFException();
         if (ferror(stream)) throw IOException();
     }

     /*
     Wrapper for writing and checking fwrite.
     No need to check return value of fwrite, since the ferror
     flag is set automatically.
     */
     void check_write(void* buffer, size_t size, size_t count, FILE* stream) {
         fwrite(buffer, size, count, stream);
         if (ferror(stream)) throw IOException();
     }

     // Helper method for importing the header.
     void import_header() {
         /*
           The signature is the only word.
           A buffer is needed to switch the endianness
           for the signature only, so it reads 42 4D not 4D 42.
           All other fields are little endian.
           Although the C standard mandates that the size of a char
           be 1 byte, using sizeof(char) here for readability.
           Compiler will replace with 1 -- no runtime performance loss.
           */
         uint8_t word_buf[WORD];
         check_read(word_buf, sizeof(char), WORD, _fileptr);
         _header.signature = static_cast<uint16_t>(word_buf[1]) |
             (static_cast<uint16_t>(word_buf[0]) << 8);
         check_read(&(_header.file_size), sizeof(char), DWORD, _fileptr);
         check_read(&(_header.reserved), sizeof(char), DWORD, _fileptr);
         check_read(&(_header.data_offset), sizeof(char), DWORD, _fileptr);
     }

     // Helper method for writing the header.
     void write_header() {
         // No need for bit shifting since it is a write.
         char signature[] = "BM";
         check_write(signature, sizeof(char), WORD, _fileptr);
         check_write(&(_header.file_size), sizeof(char), DWORD, _fileptr);
         check_write(&(_header.reserved), sizeof(char), DWORD, _fileptr);
         check_write(&(_header.data_offset), sizeof(char), DWORD, _fileptr);
     }

     // Helper method for importing the info header.
     void import_infoheader() {
         // Only planes and bits per pixel are words.
         check_read(&(_infoheader.size), sizeof(char), DWORD, _fileptr);
         check_read(&(_infoheader.width), sizeof(char), DWORD, _fileptr);
         check_read(&(_infoheader.height), sizeof(char), DWORD, _fileptr);
         check_read(&(_infoheader.planes), sizeof(char), WORD, _fileptr);
         check_read(&(_infoheader.bits_per_pixel), sizeof(char),
             WORD, _fileptr);
         check_read(&(_infoheader.compression), sizeof(char), DWORD, _fileptr);
         check_read(&(_infoheader.image_size), sizeof(char), DWORD, _fileptr);
         check_read(&(_infoheader.x_pixels_per_meter), sizeof(char),
             DWORD, _fileptr);
         check_read(&(_infoheader.y_pixels_per_meter), sizeof(char),
             DWORD, _fileptr);
         check_read(&(_infoheader.colors_used), sizeof(char), DWORD, _fileptr);
         check_read(&(_infoheader.important_colors), sizeof(char),
             DWORD, _fileptr);
     }

     // Helper method for writing the info header.
     void write_infoheader() {
         // Only planes and bits per pixel are words.
         check_write(&(_infoheader.size), sizeof(char), DWORD, _fileptr);
         check_write(&(_infoheader.width), sizeof(char), DWORD, _fileptr);
         check_write(&(_infoheader.height), sizeof(char), DWORD, _fileptr);
         check_write(&(_infoheader.planes), sizeof(char), WORD, _fileptr);
         check_write(&(_infoheader.bits_per_pixel), sizeof(char),
             WORD, _fileptr);
         check_write(&(_infoheader.compression), sizeof(char), DWORD, _fileptr);
         check_write(&(_infoheader.image_size), sizeof(char), DWORD, _fileptr);
         check_write(&(_infoheader.x_pixels_per_meter), sizeof(char),
             DWORD, _fileptr);
         check_write(&(_infoheader.y_pixels_per_meter), sizeof(char),
             DWORD, _fileptr);
         check_write(&(_infoheader.colors_used), sizeof(char), DWORD, _fileptr);
         check_write(&(_infoheader.important_colors), sizeof(char),
             DWORD, _fileptr);
     }

     // Helper method for calculating the row padding.
     size_t row_padding() {
         // Each row must be a multiple of 4 bytes.
         size_t remainder = (_infoheader.width * 3) % 4;
         if (remainder == 0) {
             return 0;
         } else {
             return (4 - remainder);
         }
     }

     /*
     Helper method for checking file correctness and compatibility.
     Returns true if correct and compatible, false otherwise.
     Not checking image size with relation to width, height, and padding
     since some images have zero bytes appended to them,
     especially ones converted and edited through Photoshop.
     (Adobe appends two 0x00 bytes.)
     */
     bool compatible() {
         // Check image signature for "BM".
         if (_header.signature != 0x424d) return false;
         // Check for data offset - no palette.
         if (_header.data_offset != 0x36) return false;
         // Check for info header size.
         if (_infoheader.size != 0x28) return false;
         // Check for # of image planes.
         if (_infoheader.planes != 1) return false;
         // Check for compression.
         if (_infoheader.compression != 0) return false;
         // Check for 24 bits per pixel.
         if (_infoheader.bits_per_pixel != 0x18) return false;
         // Check number of colors in palette.
         if (_infoheader.colors_used != 0) return false;
         // Check number of important colors.
         if (_infoheader.important_colors != 0) return false;
         // All checks passed.
         return true;
     }

 public:
    // Default constructor.
    BitmapParser()
        : _fileptr(nullptr), _header(Header()),
        _infoheader(InfoHeader()),
        _pixels(std::vector<std::vector<Pixel> >()),
        _padding(0) {}

    // Overloaded ctor for C-string filename.
    explicit BitmapParser(const char *filename)
        : _fileptr(nullptr), _header(Header()),
        _infoheader(InfoHeader()),
        _pixels(std::vector<std::vector<Pixel> >()),
        _padding(0) {
            import(filename);
    }

    // Overloaded ctor for C++ string filename.
    explicit BitmapParser(const std::string& filename)
        : _fileptr(nullptr), _header(Header()),
        _infoheader(InfoHeader()),
        _pixels(std::vector<std::vector<Pixel> >()),
        _padding(0) {
            import(filename.c_str());
    }

    // Accessor for header struct.
    const Header& read_header() const {
        return _header;
    }

    // Mutator for header struct as reference.
    Header& header() {
        return _header;
    }

    // Mutator for replacing header struct.
    void replace_header(const Header& new_header) {
        // Shallow copy is fine, no pointers.
        _header = new_header;
    }

    // Accessor for info header struct.
    const InfoHeader& read_infoheader() const {
        return _infoheader;
    }

    // Mutator for info header struct as reference.
    InfoHeader& infoheader() {
        return _infoheader;
    }

    // Mutator for replacing infoheader struct.
    void replace_infoheader(const InfoHeader& new_infoheader) {
        // Shallow copy is fine, no pointers.
        _infoheader = new_infoheader;
    }

    // Accessor for pixels.
    const std::vector<std::vector<Pixel> >& read_pixels() const {
        return _pixels;
    }

    // Mutator for pixels vector as reference.
    std::vector<std::vector<Pixel> >& pixels() {
        return _pixels;
    }

    // Mutator for replacing pixels vector.
    void replace_pixels(const std::vector<std::vector<Pixel> >& new_pixels) {
        // Shallow copy is fine, no pointers.
        _pixels = new_pixels;
    }

    // Reads and parses a bitmap file.
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
        // Calculate row padding via helper.
        _padding = row_padding();
        // Check correctness and compatibility of the image.
        if (!compatible()) throw InvalidFormatException();
        // Create vectors of Pixels by height (# of rows).
        _pixels.resize(_infoheader.height);
        // Reserve in each row for future Pixel push_back.
        for (std::vector<Pixel> row : _pixels) {
            row.reserve(_infoheader.width);
        }
        /*
        Finally, read the pixels bottom-up. The start of the file
        after the header/info header contains the bottom left pixel.
        Pixels are stored in blue, green, red order.
        Using int for row due to complications with decrementing for loops
        and unsigned values.
        */
        for (int row = _pixels.size() - 1; row >= 0; --row) {
            for (size_t col = 0; col < _infoheader.width; ++col) {
                // Read R, G, B for each pixel.
                Pixel pix = {};
                check_read(&(pix.blue), sizeof(char), BYTE, _fileptr);
                check_read(&(pix.green), sizeof(char), BYTE, _fileptr);
                check_read(&(pix.red), sizeof(char), BYTE, _fileptr);
                _pixels[row].push_back(pix);
            }
            // Skip the row padding before moving to the next row.
            fseek(_fileptr, _padding, SEEK_CUR);
        }
        // Close the file.
        fclose(_fileptr);
    }

    // Writes a bitmap file.
    void save(const char* filename) {
        // Filler zero byte for padding.
        uint8_t padding_byte = 0x0;
        /*
         Open and check for success.
         FYI - Visual Studio debugger requires absolute path.
         */
        _fileptr = fopen(filename, "wb");
        if (_fileptr == nullptr) throw FileOpenException();
        // Write the header and info header via helpers.
        write_header();
        write_infoheader();
        /*
        Finally, write the pixels bottom-up. The start of the file
        after the header/info header should contain the bottom left pixel.
        Pixels must be written in blue, green, red order.
        Using int for row due to complications with decrementing for loops
        and unsigned values.
        */
        for (int row = _pixels.size() - 1; row >= 0; --row) {
            for (size_t col = 0; col < _infoheader.width; ++col) {
                // Write R, G, B for each pixel.
                Pixel& pix = _pixels[row][col];
                check_write(&(pix.blue), sizeof(char), BYTE, _fileptr);
                check_write(&(pix.green), sizeof(char), BYTE, _fileptr);
                check_write(&(pix.red), sizeof(char), BYTE, _fileptr);
            }
            // Write row padding before moving to the next row.
            check_write(&(padding_byte), sizeof(char), _padding, _fileptr);
        }
        // Close the file.
        fclose(_fileptr);
    }

    // Prints information about the header and info header.
    void print_metadata(bool hex) const {
        // For displaying text dividers.
        const std::string div = "========================================";
        if (hex) {
            std::cout << "Number base: hexadecimal\n\n";
        } else {
            std::cout << "Number base: decimal\n\n";
        }
        std::cout << "HEADER\n" << div <<
            "\nSignature (hexadecimal): 0x" <<
            std::hex << _header.signature;
        // Determine hex or decimal.
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
    Prints information about pixels, by row. Lists padding as well.
    Output may be long - recommended to pipe to file.
    */
    void print_pixels(bool hex) const {
        if (hex) {
            std::cout << "Number base: hexadecimal\n\n";
        } else {
            std::cout << "Number base: decimal\n\n";
        }
        for (size_t row = 0; row < _infoheader.height; ++row) {
            std::cout << std::dec << "Row " << row << " (R/G/B)" <<
                "\n==============================\n";
            for (size_t col = 0; col < _infoheader.width; ++col) {
                const Pixel& pix = _pixels[row][col];
                std::cout << std::dec << "Col " << col << ":\t\t";
                if (hex) std::cout << std::hex;
                // Cout can't print uint8_t without unsigned().
                std::cout << unsigned(pix.red) << ' ' <<
                    unsigned(pix.green) << ' ' <<
                    unsigned(pix.blue) << '\n';
            }
            // Padding is 0-3 bytes, same notation in decimal and hex.
            std::cout << "Padding Bytes: " << _padding << "\n\n";
        }
    }
};
#endif  // BITMAPPARSER_H_

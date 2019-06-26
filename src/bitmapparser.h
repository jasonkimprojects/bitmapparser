// Copyright 2019 Jason Kim. All rights reserved.
/*
bitmapparser.h

A simple library to read, write, and edit bitmap images. 
Written as a side project to study file processing, and for fun.

Only 24-bit color (RGB, 0-255) without compression is supported.

The style confroms to Google's coding style guide for C++,
and has been checked with cpplint.

Jason Kim
June 13, 2019
*/

#ifndef BITMAPPARSER_H_
#define BITMAPPARSER_H_

// For printing.
#include <iostream>
// For storing bitmap information.
#include <vector>
// Explicit include for compatibility.
#include <string>
// For exceptions and error handling.
#include <stdexcept>
// For STL algorithms.
#include <algorithm>
#include <utility>

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
     // Constants for correct images.
     static const size_t CORRECT_SIG = 0x424d;
     static const size_t CORRECT_TOTAL_HEADER_SIZE = 0x36;
     static const size_t CORRECT_INFOHEADER_SIZE = 0x28;
     static const size_t CORRECT_BITS_PER_PIXEL = 0x18;
     static const size_t CORRECT_BYTES_PER_PIXEL = 3;
     static const size_t CORRECT_PLANES = 1;
     static const size_t CORRECT_COMPRESSION = 0;
     static const size_t CORRECT_COLORS_USED = 0;
     static const size_t CORRECT_IMPORTANT_COLORS = 0;

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
         else if (ferror(stream)) throw IOException();
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
         /*
         No need for bit shifting since it is a write,
         but a char[] is needed for correct endianness.
         */
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

     /*
     Helper method for checking file correctness and compatibility.
     Returns true if correct and compatible, false otherwise.
     Not checking image size with relation to width, height, and padding
     since some images have zero bytes appended to them,
     especially ones converted and edited through Photoshop.
     (Adobe appends two 0x00 bytes.)
     */
     bool compatible() const {
         // Check image signature for "BM".
         if (_header.signature != CORRECT_SIG) return false;
         // Check for data offset - no palette.
         else if (_header.data_offset !=
             CORRECT_TOTAL_HEADER_SIZE) return false;
         // Check for info header size.
         else if (_infoheader.size != CORRECT_INFOHEADER_SIZE) return false;
         // Check for # of image planes.
         else if (_infoheader.planes != CORRECT_PLANES) return false;
         // Check for compression.
         else if (_infoheader.compression != CORRECT_COMPRESSION) return false;
         // Check for 24 bits per pixel.
         else if (_infoheader.bits_per_pixel !=
             CORRECT_BITS_PER_PIXEL) return false;
         // Check number of colors in palette.
         else if (_infoheader.colors_used != CORRECT_COLORS_USED) return false;
         // Check number of important colors.
         else if (_infoheader.important_colors !=
             CORRECT_IMPORTANT_COLORS) return false;
         // All checks passed.
         else
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

    // Accessor for padding.
    const size_t read_padding() const {
        return _padding;
    }

    // Mutator for padding.
    size_t padding() {
        return _padding;
    }

    // Mutator for replacing padding.
    void replace_padding(size_t new_padding) {
        _padding = new_padding;
    }

    /*
    Returns the number of bytes for row padding for a given width.
    This function is static - it can be used as a padding calculator
    on its own without making an instance of BitmapParser.
    */
    static size_t row_padding(size_t width) {
        // Each row must be a multiple of a dword (4 bytes)
        size_t remainder = (width * CORRECT_BYTES_PER_PIXEL) % DWORD;
        if (remainder == 0) {
            return 0;
        } else {
            return (DWORD - remainder);
        }
    }

    // Overload: if no parameters are passed, uses current width.
    size_t row_padding() const {
        return row_padding(_infoheader.width);
    }

    /*
    Returns the file size given width and height.
    This function is static - it can be used as a size calculator
    on its own without making an instance of BitmapParser.
    */
    static size_t calculate_size(size_t width, size_t height) {
        return ((((CORRECT_BYTES_PER_PIXEL * width) +
            row_padding(width)) * height) + CORRECT_TOTAL_HEADER_SIZE);
    }

    // Overload: if no parameters are passed, uses current width and height.
    size_t calculate_size() const {
        return calculate_size(_infoheader.width, _infoheader.height);
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
        for (std::vector<Pixel>& row : _pixels) {
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

    // Inverts the colors of the image.
    void invert_colors() {
        const uint8_t color_max = 0xff;
        for (std::vector<Pixel>& row : _pixels) {
            for (Pixel& pix : row) {
                pix.red = color_max - pix.red;
                pix.green = color_max - pix.green;
                pix.blue = color_max - pix.blue;
            }
        }
    }

    // Flips the image horizontally.
    void flip_horizontal() {
        for (std::vector<Pixel>& row : _pixels) {
            std::reverse(row.begin(), row.end());
        }
    }

    // Flips the image vertically.
    void flip_vertical() {
        /*
        Unable to use std::reverse here due to the
        pixels in one column being in different vectors.
        */
        for (size_t col = 0; col < _pixels[0].size(); ++col) {
            size_t start_idx = 0;
            size_t end_idx = _pixels.size() - 1;
            while (start_idx < end_idx) {
                std::swap(_pixels[start_idx][col], _pixels[end_idx][col]);
                ++start_idx;
                --end_idx;
            }
        }
    }

    // Turns the image into grayscale using the average method.
    void grayscale() {
        for (std::vector<Pixel>& row : _pixels) {
            for (Pixel& pix : row) {
                // Average algorithm without overflow.
                const uint8_t avg = (pix.red / CORRECT_BYTES_PER_PIXEL) +
                    (pix.green / CORRECT_BYTES_PER_PIXEL) +
                    (pix.blue / CORRECT_BYTES_PER_PIXEL) +
                    (((pix.red % CORRECT_BYTES_PER_PIXEL) +
                    (pix.green % CORRECT_BYTES_PER_PIXEL) +
                    (pix.blue % CORRECT_BYTES_PER_PIXEL)) /
                        CORRECT_BYTES_PER_PIXEL);
                pix.red = avg;
                pix.green = avg;
                pix.blue = avg;
            }
        }
    }

    /*
    Crops the subset of _pixels from [x_begin, x_end] [y_begin, y_end].
    Indices are inclusive.
    Adjusts for metadata changes accordingly, which are:
    Header - file size
    Info Header - width and height
    (Since no compression is assumed, image size can remain zero.)
    */
    void crop(size_t x_begin, size_t y_begin, size_t x_end, size_t y_end) {
        /*
        Sanity check on indices. If negative ints are passed and casted
        to size_t they will overflow, so only four checks are needed:
        1. x_begin and x_end are smaller than the width.
        2. x_begin is smaller or equal to x_end.
        3. y_begin and y_end are smaller than the height.
        4. y_begin is smaller or equal to y_end.
        */
        if (!(x_begin < _infoheader.width && x_end < _infoheader.width))
            throw std::out_of_range(
                "x_begin and x_end must be smaller than width!\n");
        else if (!(x_begin <= x_end))
            throw std::out_of_range(
                "x_begin must be smaller than or equal to x_end!\n");
        else if (!(y_begin < _infoheader.height && y_end < _infoheader.height))
            throw std::out_of_range(
                "y_begin and y_end must be smaller than height!\n");
        else if (!(y_begin <= y_end))
            throw std::out_of_range(
                "y_begin must be smaller than or equal to y_end!\n");
        // Sanity checks passed, begin cropping.
        const size_t new_width = x_end - x_begin;
        const size_t new_height = y_end - y_begin;
        // Vector's range ctor doesn't like in-place cropping
        std::vector<std::vector<Pixel> > new_pixels(new_height,
            std::vector<Pixel>(new_width));
        // Indices for the new pixels vector.
        size_t row_idx = 0;
        size_t col_idx = 0;
        /*
        Outer loop iterator traverses the vector of vectors of Pixels;
        inner loop iterator traverses a vector of Pixels.
        */
        std::vector<std::vector<Pixel> >::iterator row_it;
        std::vector<Pixel>::iterator col_it;
        for (row_it = _pixels.begin() + y_begin;
            row_it < _pixels.begin() + y_begin + new_height;
            ++row_it) {
                for (col_it = row_it->begin() + x_begin;
                    col_it < row_it->begin() + x_begin + new_width;
                    ++col_it) {
                        new_pixels[row_idx][col_idx] = *col_it;
                        ++col_idx;
                }
                ++row_idx;
                col_idx = 0;
        }
        // Replace the Pixels vector.
        _pixels = new_pixels;
        // Change width and height
        _infoheader.width = new_width;
        _infoheader.height = new_height;
        // Replace the padding
        _padding = row_padding();
        // Calculate and change file size
        _header.file_size = calculate_size();
    }
};
#endif  // BITMAPPARSER_H_

# BitmapParser

A short header-only library to read, write, and make simple edits on bitmap images. Only images with 24-bit uncompressed color are supported, to keep the project simple.

*BitmapParser* was written over the summer of 2019 as a side project to study file processing, and for fun. Subsequent parts of my summer projects may build up on this library.

I referred to Google's style guide for C++ while writing *BitmapParser* for the sake of good readability, and the code has been checked with `cpplint`.

Last but not least, this library is licensed under the GNU GPL v3.0.

Jason Kim  
June 27, 2019

## Instructions - Core
#### 1. Using *BitmapParser* in your code:
Easy as pie, since it's just a header. `#include "bitmapparser.h"`
Note that if you download the header to a subfolder, `src/` for example, add the file path to the name: `#include "src/bitmapparser.h"`

#### 2. Included Libraries
*BitmapParser* includes the following C++ STL libraries:
 
* `iostream` for printing to `stdout`
* `vector` for storing pixels
* `string` explicitly included for portability, although `iostream` usually includes `string`
* `stdexcept` for error handling
* `algorithm` and `utility` for widely used functions

#### 3. Exceptions
*BitmapParser* will throw an `std::out_of_range` exception for the functions `crop` and `superimpose`, in addition to four custom exceptions:

* `InvalidFormatException` for invalid or incompatible files
* `FileOpenException` if the file fails to open
* `EOFException` if end of file is reached prematurely
* `IOException` for errors in reading from or writing to files

#### 4. Member Variables
*BitmapParser* has the following member variables. They are all private for the sake of encapsulation, but there are accessor and mutator functions for all of them **except** the file pointer.

* `_fileptr`: A `FILE*` to read and write bitmap files
* `_header`: This is a `Header` struct, defined in the library itself. It contains the information corresponding to a bitmap image's header. [For more information on the bitmap file structure, click here.](http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm)

* `_infoheader`: This is an `InfoHeader` struct, defined in the library itself. It contains the information corresponding to a bitmap image's info header. Click the link above for an explanation on the info header.

* `_pixels`: A `std::vector<std::vector<Pixel>>`, or a vector of vectors of Pixels (2-dimensional). `Pixel`is a struct also defined in the library, and it consists of three `uint8_t` (bytes) for the red, green, and blue channels. This is the core component of *BitmapParser* where an image file is decoded pixel by pixel.

* `_padding`: A `size_t` which stores the number of row padding bytes (0-3). Click the link and look at **Additional Info** for an explanation on row padding.

#### 5. Constructors
*BitmapParser* has three constructors: a default constructor taking no arguments, a constructor that takes a C-style string (`char*` or `char[]`) as a filename and opens the file specified, and a constructor that does the same thing but takes a C++ style `std::string`. 

* Default constructor `BitmapParser()`: `BitmapParser bp;` zero-initializes member variables, but does nothing else

* Overloaded constructors `explicit BitmapParser(const char* filename)` and `explicit BitmapParser(const std::string& filename)`: `BitmapParser bp("image.bmp");` opens *image.bmp*.

#### 6. Accessors and Mutators
The general rule for naming is that read only accessors are prefixed by **read**, write only mutators are prefixed by **replace**, and read/write functions are just the variable name minus the underscore.

`_header`:

* `const Header& read_header() const` - read only
* `Header& header()` - read and write, including members
* `void replace_header(const Header& new_header)` - write only

`_infoheader`:

* `const InfoHeader& read_infoheader() const` - read only
* `InfoHeader& infoheader()` - read and write, including members
* `void replace_infoheader(const InfoHeader& new_infoheader)` - write only

`_pixels`:

* `const std::vector<std::vector<Pixel> >& read_pixels() const` - read only
* `std::vector<std::vector<Pixel> >& pixels()` - read and write, including pixels
* `void replace_pixels(const std::vector<std::vector<Pixel> >& new_pixels)` - write only

`_padding`:

* `const size_t read_padding() const` - read only
* `size_t padding()` - read and write
* void replace_padding(size_t new_padding) - write only

#### 7. Static Functions
*BitmapParser* has two static functions that can be used without creating an instance of *BitmapParser*. There are also wrappers for the static functions that are designed to be used within the class. These wrappers take no arguments and take inputs from member variables.

* `static size_t row_padding(size_t width)`  calculates the row padding in bytes (0-3 inclusive) given the width of the image.

* `size_t row_padding() const` uses the width of the current instance instead.

* `static size_t calculate_size(size_t width, size_t height)` calculates file size in bytes given the width and height of the image.

* `size_t calculate_size() const` uses the width and height of the current instance instead.

The two functions can also be used as simple calculators. For example:  
`size_t file_size = BitmapParser.calculate_size(800, 600);`

#### 8. Reading and Writing
Both functions take a `const char*` for the file name. Please note that if you save to an existing file, **all the contents of the file will be overwritten!** The function signatures are as follows:

* `void import(const char* filename)`
* `void save(const char* filename)`

Furthermore, the function `void clear_data()` erases all data stored in this instance.

#### 9. Printing
*BitmapParser* has two functions for printing information about the image to `stdout`. The function signatures are as follows:

* `void print_metadata(bool hex) const` prints the values in the header and info header of the image. The boolean argument `hex` determines the number base of the output; `false` for decimal, and `true` for hexadecimal.

* `void print_pixels(bool hex) const` prints the RGB values of each and every pixel in the image. **The output is likely to be very long, and I recommend that you pipe it to a file instead of the console** (append `> OUTPUT_FILE_NAME` when running from the console). `hex` works the same way as before; `false` for decimal RGB values (0-255), `true` for hexadecimal RGB values (0x0-0xFF). 

## Instructions - Image Editing
In addition to reading and writing bitmap images, *BitmapParser* packs a subset of image editing functions.

#### 1. Reflections

* `void flip_horizontal()` flips the image horizontally, i.e. over the y-axis, reversing each row.

* `void flip_vertical()` flips the image vertically, i.e. over the x-axis, reversing each column.

#### 2. Transposition and Rotations

* `void transpose()` transposes the image. (Rows become columns, columns become rows.)

* `void rotate90_left()` rotates the image 90 degrees counterclockwise (-90 degrees).

* `void rotate90_right()` rotates the image 90 degrees clockwise.

#### 3. Cropping

* `void crop(size_t x_begin, size_t y_begin, size_t x_end, size_t y_end)` crops the image from width `x_begin` to `x_end` and height from `y_begin` to `y_end`, **inclusive**.

#### 4. Superimposing

* `void superimpose(const BitmapParser& other, size_t x_begin, size_t y_begin)` brings the image in the instance `other` on top of this image at offset (`x_begin`, `y_begin`).

#### 5. Color Filters
* `void invert_colors()` inverts the colors. All channels are replaced by their additive complement of 255.

* `void grayscale()` turns the image to grayscale, using the average method.

* `void sepia()` applies a sepia filter on the image, using Microsoft's channel weights.

* `void isolate_red()` preserves red channel values and eliminates green and blue hues from the image.

* `void isolate_green()` preserves green channel values and eliminates red and blue hues from the image.

* `void isolate_blue()` preserves blue channel values and eliminates red and green hues from the image.
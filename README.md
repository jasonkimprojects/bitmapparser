# BitmapParser

A short header-only library to read, write, and make simple edits on bitmap images. Only images with 24-bit uncompressed color are supported, to keep the project simple.

*BitmapParser* was written over the summer of 2019 as a side project to study file processing, and for fun. Subsequent parts of my summer projects may build up on this library.

I referred to Google's style guide for C++ while writing *BitmapParser* for the sake of good readability, and the code has been checked with `cpplint`.

Last but not least, this library is licensed under the GNU GPL v3.0.

Jason Kim  
June 27, 2019

# Instructions
#### 1. Using *BitmapParser* in your code:
Easy as pie, since it's just a header. `#include "bitmapparser.h"`
Note that if you download the header to a subfolder, `src/` for example, add the file path to the name: `#include "src/bitmapparser.h"`

#### 2. Included Libraries
*BitmapParser* includes the following C++ STL libraries:
 
* `iostream` for printing to stdout
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
* `_header`: This is a `Header` struct, defined in the library itself. It contains the information corresponding to a bitmap image's header. [For more information on the bitmap file structure, click here.] (http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm)

* `_infoheader`: This is an `InfoHeader` struct, defined in the library itself. It contains the information corresponding to a bitmap image's info header. Click the link above for an explanation on the info header.

* `_pixels`: A `std::vector<std::vector<Pixel>>`, or a vector of vectors of Pixels (2-dimensional). `Pixel`is a struct also defined in the library, and it consists of three `uint8_t` (bytes) for the red, green, and blue channels.

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
# EXIF Decode Extension for libJpeg

The C code provided here provides a simple extension to libjpeg that allows the user to access EXIF data read by libJpeg.  It is simple to add the extension to the libJpeg code itself.


# libJpeg

libJpeg is a simple but fast library for reading and writing jpeg files that have been around for a long time.  I used the instructions found on this blog: [https://numberduck.com/Blog/?nPostId=2](https://numberduck.com/Blog/?nPostId=2) to install it. Or you can download it directly from here: [https://www.ijg.org/files](https://www.ijg.org/files).  I built a library using the provided C source code.  It works well and is adequately fast.  This code was built on the [jpegsrc.v9d](http://www.ijg.org/files/jpegsrc.v9d.tar.gz) release.

What is missing even from the latest release is code that will parse the EXIF data to provide additional metadata from the file like date stamp and GPS coordinates.  The code in this Github offers a way to add EXIF parsing to the libJpeg codebase.


# Installation

Assuming the libJpeg library is already building correctly, only two additional files need to be added.



1. <span style="text-decoration:underline;">Add jdexif.h</span> to the include files of the library.  Any code that needs to access the EXIF data needs to have a #include of this file.  It also provides the user with documentation regarding the access functions.
2. <span style="text-decoration:underline;">Add jdexif.c</span> to the library.  It implements the parsing and holds a static list of the EXIF data parsed from the file and the EXIF data accessor functions.

In addition, add the following two lines to jdmarker.c:

Somewhere around line 20, add the following external reference indicated below below


```
    #include "jpeglib.h"

    extern boolean process_exif_parameters(j_decompress_ptr cinfo); // ADD THIS LINE

    typedef enum {			/* JPEG marker codes */
```


Somewhere around line 1433, add the line indicated below code.


```
      marker->process_APPn[0] = get_interesting_appn;
      marker->process_APPn[1] = process_exif_parameters; // ADD THIS LINE
      marker->process_APPn[14] = get_interesting_appn;
```


The addition of those two lines will cause the EXIF data to be written into an internal data structure when the file is opened and read.  That data can be accessed later as described below.


# EXIF Data Access

After a jpeg file has been read, any EXIF data included in the file can be accessed with one of the four functions:


```
int exifASCIIData(uint16_t tag, char* vals);
```


exifASICData returns the data for EXIF tags with type ASCII.  

**tag** is the TIFF, EXIF, or GPS tag defined in the EXIF spec.  See the description below.

**vals** is a pointer to an array of characters provided by the user.  The character string for that tag will be written to that array, so it must be large enough to handle data returned.  They are 0 terminated c style strings. 

**Return**

    A greater than 0 integer indicates the number of characters written to vals[]
    0 indicates that no data was found for that tag which means it was not in the file
    -1 indicates the data associated with that tag was found but is not ASCI data.


```
int exifUIntData(uint16_t tag, uint32_t* vals);
```
 exifUIntData returns data for tags with type UNDEFINED, BYTE, SHORT or LONG.

**tag** is the TIFF, EXIF, or GPS tag defined in the EXIF spec.  See the description below.

**vals** is a pointer to an array of 32-bit unsigned integers provided by the user.  The integer values for that tag will be written to that array, so it must be large enough to handle data returned. 

**Return**

    A greater than 0 integer indicates the number of 32-bit unsigned integers written to vals[]
    0 indicates that no data was found for that tag which means it was not in the file
    -1 indicated the data associated with that tag was found but is not unsigned integer data


```
int exifIntData(uint16_t tag, int32_t* vals);
```
 exifIntData returns data for tags with type SLONG.

**tag** is the TIFF, EXIF, or GPS tag defined in the EXIF spec.  See the description below.

**vals** is a pointer to an array of 32-bit signed integers provided by the user.  The integer values for that tag will be written to that array, so it must be large enough to handle data returned. 

**Return**

    A greater than 0 integer indicates the number of 32-bit signed integers written to vals[]
    0 indicates that no data was found for that tag which indicates it was not in the file
    -1 indicated the data associated with that tag was found but is not integer data


```
int exifRationalData(uint16_t tag, double* vals);
```


 exifRationalData returns data for tags with type RATIONAL or SRATIONAL.

**tag** is the TIFF, EXIF, or GPS tag defined in the EXIF spec.  See the description below.

**vals** is a pointer to an array of doubles provided by the user.  The doubles for that tag will be written to that array, so it must be large enough to handle data returned.  Note that in the case of rationals, the double returned is the result of dividing the numerator and denominator in the EXIF data.

**Return**

    A greater than 0 integer indicates the number of doubles written to vals[]
    0 indicates that no data was found for that tag which indicates it was not in the file
    -1 indicated the data associated with that tag was found but is not integer data


```
void tagmapFree(); 
```
tagmapFree frees the internal data structure memory that holds the EXIF data.  This function is called when a new file is opened, so it is unnecessary to call it between successive file reads.


```
boolean tagmapPrint();
```


tagmapPrint() prints all tags captured from the last file read to stdout.  It is provided as a debug function.


## EXIF Tags

Each data access function described above requires an EXIF tag as the initial argument.  The integer value of that tag is defined in the EXIF spec.   The header file jdexif.h contains a list of defines that map the tag's name to its integer value.  Some examples are:


```cpp
#define TIFFImageWidth 0x0100 // Short/Long 1 Image width
#define TIFFImageLength 0x0101 // Short/Long 1 Image height

#define EXIFDateTimeOriginal  0x9003 // Ascii 20 Date and time of original data generation
#define EXIFDateTimeDigitized  0x9004 // Ascii 20 Date and time of digital data generation

#define GPSLatitudeRef  0x0001 // Ascii 2 North or South Latitude
#define GPSLatitude  0x0002 // Rational 3 Latitude

```


Along with each defined tag, the data type of the EXIF data for that tag is included in the comment. Use this data type to determine which data access function to use. Also shown is the number of data to expect. The vals[] array should be sized to receive that number.

The table of EXIF tags was copied from [https://www.vieas.com/en/exif23.html](https://www.vieas.com/en/exif23.html) It corresponds to version 2.3 of the EXIF spec.  If you know that an EXIF tag exists but is not defined in the header file, you can still call the appropriate access function with the integer value of that tag.


## Example

Here is some example code of how to access the time and gps coordinates after a file hsa been opened.


```cpp
#include <fmt/core.h>
#include "jdexif.h"
		:
		:

   // Read the time string to location object
    char c_date[30];
    if (0 < exifASCIIData(EXIFDateTimeOriginal, c_date)) {
      fmt::print("{}\n",string(c_date));
    }
    else {
      return true;
    }

	// this code reads the GPS data from the EXIF and converts it to
	// single signed pair of numbers.
double lat = 0.0, lon = 0.0;  // final values
double td[3]; // array used to obtain the 3 values of lat and lon
char ta[10]; // character array to get the reference characters.
if (0 != exifRationalData(GPSLatitude, td)) {
     		lat = td[0] + td[1] / 60.0 + td[2] / (60.0 * 60.0);
      	exifASCIIData(GPSLatitudeRef, ta);
      	if (ta[0] == 'S') lat = -lat;
    	} else {
		fmt::print(" Latitude data not found\n");
	}
if (0 != exifRationalData(GPSLongitude, td)) {
    		lon = td[0] + td[1] / 60.0 + td[2] / (60.0 * 60.0);
    		exifASCIIData(GPSLongitudeRef, ta);
if (ta[0] == 'W') lon = -lon;
    	} else {
fmt::print(" Lalongitude data not found\n");
fmt::print("{} {}\n", lat, lon );


```



# Limitations

I was not personally interested in integrating this code in a full and complimentary way into the libJpeg library.  There were too many macros and special data types I would have to learn.  To minimize the code changes, the data structure that holds the exif data is a static structure and is only valid for the last file read.  That also means that it is not thread safe.  The correct what to add this would be to add the EXIF.data structure to cinfo decompress structure.

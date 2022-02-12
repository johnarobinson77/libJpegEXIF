#pragma once

#include <stdint.h>

#ifndef JDEXIF_H
#define JDEXIF_H
#ifdef __cplusplus
extern "C" {
#endif
// These functions are for accessing the exif data 
// 
// tagmatFree() resets the tag map to free memory and clear tagmap from previous file
void tagmapFree();

// exifASICData returns the data for tags with type ASCII.  char strings are 0 terminated.
// The return integer indicates the number of characters found or 0 if tag was not found.
// If the return interger is -1, then the data type for that exif tag does not match the ascii data types.
int exifASCIIData(uint16_t tag, char* vals);

// exifUIntData returns the interger data for tags with type UNDEFINED, BYTE, SHORT and LONG.
// The return integer indicates the number of characters found or 0 if tag was not found.
// If the return interger is -1, then the data type for that exif tag does not match the unsigned int data types.
int exifUIntData(uint16_t tag, uint32_t* vals);

// exifIntData returns the interger data for tags with type SLONG.
// The return integer indicates the number of characters found or 0 if tag was not found.
// If the return interger is -1, then the data type for that exif tag does not match the signed long data type.
int exifIntData(uint16_t tag, int32_t* vals);

// exifIntData returns the data for tags with type RATIONAL or SRATIONAL as a post devided double
// The return integer indicates the number of characters found or 0 if tag was not found.
// If the return interger is -1, then the data type for that exif tag does not match the rational data types.
int exifRationalData(uint16_t tag, double* vals);

// tagmapPrint() prints all tags that were captured from the file
boolean tagmapPrint();

#ifdef __cplusplus
}
#endif



#define TIFF_TYPE_BYTE 1
#define TIFF_TYPE_ASCII 2
#define TIFF_TYPE_SHORT 3
#define TIFF_TYPE_LONG 4
#define TIFF_TYPE_RATIONAL 5
#define TIFF_TYPE_UNDEFINED 7
#define TIFF_TYPE_SLONG 9
#define TIFF_TYPE_SRATIONAL 10

// Tags for EXIF 2.3 

// Copied from https://www.vieas.com/en/exif23.html
//		Field			  Tag //    Data type Num Comment
#define TIFFImageWidth 0x0100 // Short/Long 1 Image width
#define TIFFImageLength 0x0101 // Short/Long 1 Image height
#define TIFFBitsPerSample 0x0102 // Short 3 Number of bits per component
#define TIFFCompression 0x0103 // Short 1 Compression scheme
#define TIFFPhotometricInterpretation 0x0106 // Short 1 Pixel composition
#define TIFFImageDescription 0x010E // Ascii Any Image title
#define TIFFMake 0x010F // Ascii Any Image input equipment manufacturer
#define TIFFModel 0x0110 // Ascii Any Image input equipment model
#define TIFFStripOffsets 0x0111 // Short/Long  Image data location
#define TIFFOrientation 0x0112 // Short 1 Orientation of image
#define TIFFSamplesPerPixel 0x0115 // Short 1 Number of components
#define TIFFRowsPerStrip 0x0116 // Short/Long 1 Number of rows per strip
#define TIFFStripByteCounts 0x0117 // Short/Long  Bytes per compressed strip
#define TIFFXResolution 0x011A // Rational 1 Image resolution in width direction
#define TIFFYResolution 0x011B // Rational 1 Image resolution in height direction
#define TIFFPlanarConfiguration 0x011C // Short 1 Image data arrangement
#define TIFFResolutionUnit 0x0128 // Short 1 Unit of X and Y resolution
#define TIFFTransferFunction 0x012D // Short 3*256 Transfer function
#define TIFFSoftware 0x0131 // Ascii Any Software used
#define TIFFDateTime 0x0132 // Ascii 20 File change date and time
#define TIFFArtist 0x013B // Ascii Any Person who created the image
#define TIFFWhitePoint 0x013E // Rational 2 White point chromaticity
#define TIFFPrimaryChromaticities 0x013F // Rational 6 Chromaticities of primaries
#define TIFFJPEGInterchangeFormat 0x0201 // Long 1 Offset to JPEG SOI
#define TIFFJPEGInterchangeFormatLength 0x0202 // Long 1 Bytes of JPEG data
#define TIFFYCbCrCoefficients 0x0211 // Rational 3 Color space transformation matrix coefficients
#define TIFFYCbCrSubSampling 0x0212 // Short 2 Subsampling ratio of Y to C
#define TIFFYCbCrPositioning 0x0213 // Short 1 Y and C positioning
#define TIFFReferenceBlackWhite 0x0214 // Rational 6 Pair of black and white reference values
#define TIFFCopyright 0x8298 // Ascii Any Copyright holder
#define TIFFExif 0x8769 // Long 1 Exif tag IFD Pointer 
#define TIFFGPSInfo  0x8825 // Long 1 GPS tagIFD Pointer

//		Field			  Tag    // Data type Num Comment
#define EXIFExposureTime  0x829A // Rational  1   Exposure time
#define EXIFFNumber  0x829D // Rational 1 F number
#define EXIFExposureProgram  0x8822 // Short 1 Exposure program
#define EXIFSpectralSensitivity  0x8824 // Ascii Any Spectral sensitivity
#define EXIFPhotographicSensitivity  0x8827 // Short Any Photographic Sensitivity
#define EXIFOECF  0x8828 // Undefined Any Optoelectric conversion factor
#define EXIFSensitivityType  0x8830 // Short 1 Sensitivity Type
#define EXIFStandardOutputSensitivity  0x8831 // Long 1 Standard Output Sensitivity
#define EXIFRecommendedExposureIndex  0x8832 // Long 1 Recommended Exposure Index
#define EXIFISOSpeed  0x8833 // Long 1 ISO Speed
#define EXIFISOSpeedLatitudeyyy  0x8834 // Long 1 ISO Speed Latitude yyy
#define EXIFISOSpeedLatitudezzz  0x8835 // Long 1 ISO Speed Latitude zzz
#define EXIFExifVersion  0x9000 // Undefined 4 Exif version
#define EXIFDateTimeOriginal  0x9003 // Ascii 20 Date and time of original data generation
#define EXIFDateTimeDigitized  0x9004 // Ascii 20 Date and time of digital data generation
#define EXIFOffsetTime  0x9010 // Ascii 7 Offset Data of DateTime
#define EXIFOffsetTimeOriginal  0x9011 // Ascii 7 Offset Data of DateTimeOriginal
#define EXIFOffsetTimeDigitized  0x9012 // Ascii 7 Offset Data of DateTimeDigitized
#define EXIFComponentsConfiguration  0x9101 // Undefined 4 Meaning of each component
#define EXIFCompressedBitsPerPixel  0x9102 // Rational 1 Image compression mode
#define EXIFShutterSpeedValue  0x9201 // SRational 1 Shutter speed
#define EXIFApertureValue  0x9202 // Rational 1 Aperture
#define EXIFBrightnessValue  0x9203 // SRational 1 Brightness
#define EXIFExposureBiasValue  0x9204 // SRational 1 Exposure bias
#define EXIFMaxApertureValue  0x9205 // Rational 1 Maximum lens aperture
#define EXIFSubjectDistance  0x9206 // Rational 1 Subject distance
#define EXIFMeteringMode  0x9207 // Short 1 Metering mode
#define EXIFLightSource  0x9208 // Short 1 Light source
#define EXIFFlash  0x9209 // Short 1 Flash
#define EXIFFocalLength  0x920A // Rational 1 Lens focal length
#define EXIFSubjectArea  0x9214 // Short 2/3/2004 Subject area
#define EXIFMakerNote  0x927C // Undefined Any Manufacturer notes
#define EXIFUserComment  0x9286 // Undefined Any User comments
#define EXIFSubSecTime  0x9290 // Ascii Any DateTime subseconds
#define EXIFSubSecTimeOriginal  0x9291 // Ascii Any DateTimeOriginal subseconds
#define EXIFSubSecTimeDigitized  0x9292 // Ascii Any DateTimeDigitized subseconds
#define EXIFTemperature  0x9400 // SRational 1 Temperature
#define EXIFHumidity  0x9401 // Rational 1 Humidity
#define EXIFPressure  0x9402 // Rational 1 Pressure
#define EXIFWaterDepth  0x9403 // SRational 1 Water Depth
#define EXIFAcceleration  0x9404 // Rational 1 Acceleration
#define EXIFCameraElevationAngle  0x9404 // SRational 1 Camera Elevation Angle
#define EXIFFlashpixVersion  0xA000 // Undefined 4 Supported Flashpix version
#define EXIFColorSpace  0xA001 // Short 1 Color space information
#define EXIFPixelXDimension  0xA002 // Short/Long 1 Valid image width
#define EXIFPixelYDimension  0xA003 // Short/Long 1 Valid image height
#define EXIFRelatedSoundFile  0xA004 // Ascii 13 Related audio file
#define EXIFInteroperability IFD Pointer  0xA005 // Long 1 Interoperability tag
#define EXIFFlashEnergy  0xA20B // Rational 1 Flash energy
#define EXIFSpatialFrequencyResponse  0xA20C // Undefined Any Spatial frequency response
#define EXIFFocalPlaneXResolution  0xA20E // Rational 1 Focal plane X resolution
#define EXIFFocalPlaneYResolution  0xA20F // Rational 1 Focal plane Y resolution
#define EXIFFocalPlaneResolutionUnit  0xA210 // Short 1 Focal plane resolution unit
#define EXIFSubjectLocation  0xA214 // Short 2 Subject location
#define EXIFExposureIndex  0xA215 // Rational 1 Exposure index
#define EXIFSensingMethod  0xA217 // Short 1 Sensing method
#define EXIFFileSource  0xA300 // Undefined 1 File source
#define EXIFSceneType  0xA301 // Undefined 1 Scene type
#define EXIFCFAPattern  0xA302 // Undefined Any CFA pattern
#define EXIFCustomRendered  0xA401 // Short 1 Custom image processing
#define EXIFExposureMode  0xA402 // Short 1 Exposure mode
#define EXIFWhiteBalance  0xA403 // Short 1 White balance
#define EXIFDigitalZoomRatio  0xA404 // Rational 1 Digital zoom ratio
#define EXIFFocalLengthIn35mmFilm  0xA405 // Short 1 Focal length in 35 mm film
#define EXIFSceneCaptureType  0xA406 // Short 1 Scene capture type
#define EXIFGainControl  0xA407 // Short 1 Gain control
#define EXIFContrast  0xA408 // Short 1 Contrast
#define EXIFSaturation  0xA409 // Short 1 Saturation
#define EXIFSharpness  0xA40A // Short 1 Sharpness
#define EXIFDeviceSettingDescription  0xA40B // Undefined Any Device settings description
#define EXIFSubjectDistanceRange  0xA40C // Short 1 Subject distance range
#define EXIFCompositeImage  0xA460 // Short 1 Composite Image
#define EXIFSourceImageNumberOfCompositeImage 0xA461 // Short 1 Source Image Number Of Composite Image
#define EXIFSourceExposureTimesOfCompositeImage 0xA462 // UNDEFINED aNY Source Exposure Times Of Composite Image
#define EXIFImageUniqueID  0xA420 // Ascii 33 Unique image ID
#define EXIFCameraOwnerName  0xA430 // Ascii Any Camera Owner Name
#define EXIFBodySerialNumber  0xA431 // Ascii Any Body Serial Number
#define EXIFLensSpecification  0xA432 // Rational 4 Lens Specification
#define EXIFLensMake  0xA433 // Ascii Any Lens Make
#define EXIFLensModel  0xA434 // Ascii Any Lens Model
#define EXIFLensSerialNumber  0xA435 // Ascii Any Lens Serial Number
#define EXIFGamma  0xA500 // Rational 1 Gamma

//		Field		  Tag    // Data type Num Comment
#define GPSVersionID  0x0000 // Byte      4   GPS tag version
#define GPSLatitudeRef  0x0001 // Ascii 2 North or South Latitude
#define GPSLatitude  0x0002 // Rational 3 Latitude
#define GPSLongitudeRef  0x0003 // Ascii 2 East or West Longitude
#define GPSLongitude  0x0004 // Rational 3 Longitude
#define GPSAltitudeRef  0x0005 // Byte 1 Altitude reference
#define GPSAltitude  0x0006 // Rational 1 Altitude
#define GPSTimeStamp  0x0007 // Rational 3 GPS time (atomic clock)
#define GPSSatellites  0x0008 // Ascii Any GPS satellites used for measurement
#define GPSStatus  0x0009 // Ascii 2 GPS receiver status
#define GPSMeasureMode  0x000A // Ascii 2 GPS measurement mode
#define GPSDOP  0x000B // Rational 1 Measurement precision
#define GPSSpeedRef  0x000C // Ascii 2 Speed unit
#define GPSSpeed  0x000D // Rational 1 Speed of GPS receiver
#define GPSTrackRef  0x000E // Ascii 2 Reference for direction of movement
#define GPSTrack  0x000F // Rational 1 Direction of movement
#define GPSImgDirectionRef  0x0010 // Ascii 2 Reference for direction of image
#define GPSImgDirection  0x0011 // Rational 1 Direction of image
#define GPSMapDatum  0x0012 // Ascii Any Geodetic survey data used
#define GPSDestLatitudeRef  0x0013 // Ascii 2 Reference for latitude of destination
#define GPSDestLatitude  0x0014 // Rational 3 Latitude of destination
#define GPSDestLongitudeRef  0x0015 // Ascii 2 Reference for longitude of destination
#define GPSDestLongitude  0x0016 // Rational 3 Longitude of destination
#define GPSDestBearingRef  0x0017 // Ascii 2 Reference for bearing of destination
#define GPSDestBearing  0x0018 // Rational 1 Bearing of destination
#define GPSDestDistanceRef  0x0019 // Ascii 2 Reference for distance to destination
#define GPSDestDistance  0x001A // Rational 1 Distance to destination
#define GPSProcessingMethod  0x001B // Undefined Any Name of GPS processing method
#define GPSAreaInformation  0x001C // Undefined Any Name of GPS area
#define GPSDateStamp  0x001D // Ascii 11 GPS date
#define GPSDifferential  0x001E // Short 1 GPS differential correction
#define GPSHPositioningError  0x001F // Rational 1 Horizontal positioning error


#endif // !JDEXIF_H
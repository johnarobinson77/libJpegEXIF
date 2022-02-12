



#include <stdio.h>
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
#include "jdexif.h"


/* Declare and initialize local copies of input pointer/count */
#define INPUT_VARS(cinfo)  \
	struct jpeg_source_mgr * datasrc = (cinfo)->src;  \
	const JOCTET * next_input_byte = datasrc->next_input_byte;  \
	size_t bytes_in_buffer = datasrc->bytes_in_buffer

/* Unload the local copies --- do this only at a restart boundary */
#define INPUT_SYNC(cinfo)  \
	( datasrc->next_input_byte = next_input_byte,  \
	  datasrc->bytes_in_buffer = bytes_in_buffer )

/* Reload the local copies --- used only in MAKE_BYTE_AVAIL */
#define INPUT_RELOAD(cinfo)  \
	( next_input_byte = datasrc->next_input_byte,  \
	  bytes_in_buffer = datasrc->bytes_in_buffer )

/* Internal macro for INPUT_BYTE and INPUT_2BYTES: make a byte available.
 * Note we do *not* do INPUT_SYNC before calling fill_input_buffer,
 * but we must reload the local copies after a successful fill.
 */
#define MAKE_BYTE_AVAIL(cinfo,action)  \
	if (bytes_in_buffer == 0) {  \
	  if (! (*datasrc->fill_input_buffer) (cinfo))  \
	    { action; }  \
	  INPUT_RELOAD(cinfo);  \
	}

 /* Read a byte into variable V.
  * If must suspend, take the specified action (typically "return FALSE").
  */
#define INPUT_BYTE(cinfo,V,action)  \
	MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
		  bytes_in_buffer--; \
		  V = GETJOCTET(*next_input_byte++); )

  /* As above, but read two bytes interpreted as an unsigned 16-bit integer.
   * V should be declared unsigned int or perhaps INT32.
   */
#define INPUT_2BYTES(cinfo,V,action)  \
	MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
		  bytes_in_buffer--; \
		  V = ((unsigned int) GETJOCTET(*next_input_byte++)) << 8; \
		  MAKE_BYTE_AVAIL(cinfo,action); \
		  bytes_in_buffer--; \
		  V += GETJOCTET(*next_input_byte++); )


	 
// this section of code implements a map of IFDs.  
// Allows the parsed IFDs to be looked up by tag.

struct tagentry {
	uint16_t tag; // tiff, exif or gps tag
	uint16_t type; // data type as defined in the TIFF file spec
	uint32_t count; // count of the data of type above
	uint8_t* pvalue; // a pointer to said data.
	struct tagentry* next;
};

static struct tagentry* tagmap = NULL;

// this are the size of the types defined in the type fiels of a IFD
static int typeSize[] = { 0,1,1,2,4,8,0,1,0,4,8 };


// tagmapAdd adds a new entry to the front of the map.
static void 
tagmapAdd(uint32_t tag, uint32_t type, uint32_t count, uint8_t* pval) {
	//allocate a new tagentry and put it on the list
	struct tagentry* newTagEntry = (struct tagentry*)malloc(sizeof(struct tagentry));
	newTagEntry->next = tagmap;
	tagmap = newTagEntry;
	
	// copy in the basics
	newTagEntry->tag = (uint16_t)tag;
	newTagEntry->type = type;
	newTagEntry->count = count;
	uint32_t numBytes = count * typeSize[type];
	newTagEntry->pvalue = (uint8_t*)malloc(numBytes);
	if (newTagEntry->pvalue != NULL) {
		for (unsigned int i = 0; i < numBytes; i++)
			newTagEntry->pvalue[i] = pval[i];
	}
}

// looks up a tagmap entry by tag.
static struct tagentry* 
tagmapFind(uint16_t tag) {
	struct tagentry* current = tagmap;
	while (current != NULL) {
		if (current->tag == tag) break;
		current = current->next;
	}
	return current;
}

// tagmapFree frees the data in the tag map.
GLOBAL(void)
tagmapFree() {
	struct tagentry* current = tagmap;
	struct tagentry* next;
	while (current != NULL) {
		free(current->pvalue);
		next = current->next;
		free(current);
		current = next;
	}
	tagmap = NULL;
}

// tagmapPrint, prints all the IFD data in the tag map.
GLOBAL(boolean)
tagmapPrint() {
	struct tagentry* current = tagmap;
	char avals[100];
	uint32_t uivals[10];
	int32_t ivals[10];
	double dvals[10];
	while (current != NULL) {
		printf("%04x %2x %04x, ", current->tag, current->type, current->count);
		if (current->type == TIFF_TYPE_BYTE || current->type == TIFF_TYPE_SHORT ||
			current->type == TIFF_TYPE_LONG || current->type == TIFF_TYPE_UNDEFINED) {
			int cnt = exifUIntData(current->tag, uivals);
			for (int i = 0; i < cnt; i++) printf("%8d ", uivals[i]);
		}
		if (current->type == TIFF_TYPE_SLONG) {
			int cnt = exifIntData(current->tag, ivals);
			for (int i = 0; i < cnt; i++) printf("%8d ", ivals[i]);
		}
		if (current->type == TIFF_TYPE_ASCII) {
			int cnt = exifASCIIData(current->tag, avals);
			printf("%s ", avals);
		}
		if (current->type == TIFF_TYPE_RATIONAL || current->type == TIFF_TYPE_SRATIONAL) {
			int cnt = exifRationalData(current->tag, dvals);
			for (int i = 0; i < cnt; i++) printf("%lf ", dvals[i]);
		}
		printf("\n");

		/*
		printf("% 04x %2x %04x, ", current->tag, current->type, current->count);
		uint32_t numBytes = current->count * typeSize[current->type];
		for (int i = 0; i < numBytes; i++) printf("% 02x ", current->pvalue[i]);
		printf("\n");
		*/
		current = current->next;
	}
	return TRUE;
}


static boolean
proocess_subIFD_tags(uint8_t* data, uint32_t offset) {

	uint32_t number_of_tags, tagnum;
	uint32_t type, count;
	uint32_t dataOffset;

	/* Get the number of directory entries contained in this SubIFD */
	number_of_tags = ((uint32_t)data[offset + 1] << 8) + data[offset];
	if (number_of_tags < 2) return FALSE;
	offset += 2;

	/* Search for ExifImageWidth and ExifImageHeight Tags in this SubIFD */
	do {
		/* Get Tag number */
		tagnum = ((uint32_t)data[offset + 1] << 8) + data[offset];
		type = ((uint32_t)data[offset + 3] << 8) + data[offset + 2];
		count = ((uint32_t)data[offset + 7] << 24) + ((uint32_t)data[offset + 6] << 16) +
			((uint32_t)data[offset + 5] << 8) + data[offset + 4];
		uint32_t numBytes = count * typeSize[type];
		if (numBytes <= 4) {
			tagmapAdd(tagnum, type, count, &(data[offset + 8]));
		} else {
			dataOffset = ((uint32_t)data[offset + 11] << 24) + ((uint32_t)data[offset + 10] << 16) +
				((uint32_t)data[offset + 9] << 8) + data[offset + 8];
			dataOffset += 6;  // offset to the beginning of the TIFF data field.
			tagmapAdd(tagnum, type, count, (data+dataOffset));
		}
		offset += 12;
	} while (--number_of_tags);
	return TRUE;
}


boolean
process_exif_parameters(j_decompress_ptr cinfo) {
	boolean is_motorola; /* Flag for byte order */
	int32_t numberOfTags, tagnum;
	int32_t firstOffset, offset;
	uint32_t type, count;
	uint32_t dataOffset;
	boolean returnBool = FALSE;

	int32_t length;
	
	INPUT_VARS(cinfo);

	// read the size of the data for marker APP1
	INPUT_2BYTES(cinfo, length, return FALSE);
	length -= 2;

	/* check for a reasonable length of an IFD entry */
	if (length <= 0) return FALSE; 

	uint8_t* data = (uint8_t*)malloc(length * sizeof(uint8_t));
	if (data == NULL) return FALSE;

	// read the data for marker APP1 into a temporary data[] buffer 
	for (int i = 0; i < length; i++) {
		INPUT_BYTE(cinfo, data[i], return FALSE);
	}
	INPUT_SYNC(cinfo);

	/* Check to see that this is EXIF data 
	   If it not EXIF data don't fail because this could be other APP1 data
		 but don't continue to parse the data.*/
	if (0 != strcmp((char*)data, "Exif")) {
		returnBool = TRUE;
		goto freeAndReturn;
	}

	// if there is exif data from a previous file, clear it.
	tagmapFree();

	/* Discover byte order */
	if (data[6] == 0x49 && data[7] == 0x49)
		is_motorola = FALSE;
	else if (data[6] == 0x4D && data[7] == 0x4D)
		goto freeAndReturn;  // Bigendean is not supported.
	else
		goto freeAndReturn;  // Expected endian code was not found

	/* Check Tag Mark */
	uint32_t tagMark = ((uint32_t)data[9] << 8) + data[8];
	if (tagMark != 0x2A) goto freeAndReturn;

	/* Get first IFD offset (offset to IFD0) */

	firstOffset = ((uint32_t)data[13] << 24) + ((uint32_t)data[12] << 16) +
		((uint32_t)data[11] << 8) + data[10];
	firstOffset += 6; // account for Exif strng at the begining of the buffer;

	/* Get the number of directory entries contained in this IFD */

	numberOfTags = ((uint32_t)data[firstOffset + 1] << 8) + data[firstOffset];
	if (numberOfTags == 0) goto freeAndReturn;
	firstOffset += 2;

	/* Search for ExifSubIFD offset Tag in IFD0 */
	for (;;) {
		if (firstOffset > length - 12) goto freeAndReturn; /* check end of data segment */
		/* Get Tag number */
		tagnum = ((uint32_t)data[firstOffset + 1] << 8) + data[firstOffset];
		if (tagnum == 0x8769 || tagnum == 0x8825) { /* found ExifSubIFD or GPSSubIDF offset Tag */
			offset = ((uint32_t)data[firstOffset + 11] << 24) + ((uint32_t)data[firstOffset + 10] << 16) +
				((uint32_t)data[firstOffset + 9] << 8) + data[firstOffset + 8];
			offset += 6;  // tiff header starts at data[6]
			if (!proocess_subIFD_tags(data, offset)) { goto freeAndReturn; }
		}	else { // Otherwise addd the IDF to the tagmap.
			type = ((uint32_t)data[firstOffset + 3] << 8) + data[firstOffset + 2];
			count = ((uint32_t)data[firstOffset + 7] << 24) + ((uint32_t)data[firstOffset + 6] << 16) +
				((uint32_t)data[firstOffset + 5] << 8) + data[firstOffset + 4];
			uint32_t numBytes = count * typeSize[type];
			if (numBytes <= 4) {
				tagmapAdd(tagnum, type, count, &(data[firstOffset + 8]));
			} else {
				dataOffset = ((uint32_t)data[firstOffset + 11] << 24) + ((uint32_t)data[firstOffset + 10] << 16) +
					((uint32_t)data[firstOffset + 9] << 8) + data[firstOffset + 8];
				dataOffset += 6;  // offset to the beginning of the TIFF data field.
				tagmapAdd(tagnum, type, count, (data + dataOffset));
			}
		}
		if (--numberOfTags == 0) { break; }
		firstOffset += 12;
	}

freeAndReturn:
	free(data);
	return TRUE;
}

int exifUIntData(uint16_t tag, uint32_t* vals) {
	struct tagentry* current = tagmapFind(tag);
	if (current == NULL) return 0;  // tag not found
	if (current->type == TIFF_TYPE_BYTE || current->type == TIFF_TYPE_UNDEFINED) {
		for (unsigned int i = 0; i < current->count; i++) {
			vals[i] = current->pvalue[i];
		}
	} else if (current->type == TIFF_TYPE_SHORT) {
		for (unsigned int i = 0; i < current->count; i++) {
			int n = i * 2;
			vals[i] = current->pvalue[n] + ((uint32_t)current->pvalue[n + 1] << 8);
		}
	}	else if (current->type == TIFF_TYPE_LONG) {
		for (unsigned int i = 0; i < current->count; i++) {
			int n = i * 4;
			vals[i] = current->pvalue[n] + ((uint32_t)current->pvalue[n + 1] << 8) +
				((uint32_t)current->pvalue[n + 2] << 16) + ((uint32_t)current->pvalue[n + 3] << 24);
		}
	}	else return -1;  // call doesent match IFD type so return -1	
	return current->count; // return the number of data words place in the vals array
}

int exifIntData(uint16_t tag, int32_t* vals) {
	struct tagentry* current = tagmapFind(tag);
	if (current == NULL) return 0;  // tag not found
	if (current->type == TIFF_TYPE_SLONG) {
		for (unsigned int i = 0; i < current->count; i++) {
			int n = i * 4;
			vals[i] = current->pvalue[n] + ((int32_t)current->pvalue[n + 1] << 8) +
				((int32_t)current->pvalue[n + 2] << 16) + ((int32_t)current->pvalue[n + 3] << 24);
		}
	}	else return -1;  // call doesent match IFD type so return -1;
	return current->count;  // return the number of data words place in the vals array
}

int exifASCIIData(uint16_t tag, char* vals) {
	struct tagentry* current = tagmapFind(tag);
	if (current == NULL) return 0;  // tag not found
	if (current->type == TIFF_TYPE_ASCII) {
		for (unsigned int i = 0; i < current->count; i++) {
			vals[i] = current->pvalue[i];
		}
	}	else return -1;  // call doesent match IFD type so return -1;
	return current->count; // return the number of data words place in the vals array
}

int exifRationalData(uint16_t tag, double* vals) {
	struct tagentry* current = tagmapFind(tag);
	if (current == NULL) return 0;  // tag not found
	double val;
	if (current->type == TIFF_TYPE_RATIONAL) {
		for (unsigned int i = 0; i < current->count; i++) {
			int n = i * 8;
			uint32_t numerator = current->pvalue[n] + ((uint32_t)current->pvalue[n + 1] << 8) +
				((uint32_t)current->pvalue[n + 2] << 16) + ((uint32_t)current->pvalue[n + 3] << 24);
			uint32_t denominator = current->pvalue[n + 4] + ((uint32_t)current->pvalue[n + 5] << 8) +
				((uint32_t)current->pvalue[n + 6] << 16) + ((uint32_t)current->pvalue[n + 7] << 24);
			if (denominator == 0) val = 0;
			else val = (double)numerator / (double)denominator;
			vals[i] = val;
		}
	} else	if (current->type == TIFF_TYPE_SRATIONAL) {
		for (unsigned int i = 0; i < current->count; i++) {
			int n = i * 8;
			int32_t numerator = current->pvalue[n] + ((int32_t)current->pvalue[n + 1] << 8) +
				((int32_t)current->pvalue[n + 2] << 16) + ((int32_t)current->pvalue[n + 3] << 24);
			int32_t denominator = current->pvalue[n + 4] + ((int32_t)current->pvalue[n + 5] << 8) +
				((int32_t)current->pvalue[n + 6] << 16) + ((int32_t)current->pvalue[n + 7] << 24);
			if (denominator == 0) val = 0;
			else val = (double)numerator / (double)denominator;
			vals[i] = val;
		}
	} else return -1;  // call doesent match IFD type so return -1;
	return current->count; // return the number of data words place in the vals array
}

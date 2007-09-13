//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#ifndef IECORE_CINEON_H
#define IECORE_CINEON_H


// definitions

// U8	unsigned 8 bit integer	(FF hex)
// U16     unsigned 16 bit integer (FFFF hex)
// U32	unsigned 32 bit integer	(FFFFFFFF hex)
// S32	signed 32 bit integer	(80000000 hex)
// R32	32 bit real number	(7F800000 hex (+ infinity))
// ASCII				(NULL  00 hex)

#define U32 unsigned int
#define ASCII char
#define U16 unsigned short
#define R32 float
#define S32 int
#define U8 unsigned char

// The assumption the relation of code value to data metric is linear is made. Therefore, given 
// the minimum and maximum code values for a given (colour) channel, and the associated metric 
// quantity represented, any value in between can be found via linear interpolation. For example, 
// if the minimum code value is 0, representing 0.2 log exposure, and the maximum code value is 
// 1169 representing 3.4 log exposure, then a code value of 584 represents 1.8 log exposure.

typedef struct fileinformation {
  U32 magic;                    // Magic number (802A5FD7 - hex) indicates
                                // the start of image file and byte ordering
    
  U32 image_data_offset;        // Offset to image data in bytes

  U32 section_header_length;    // Generic (fixed format) section header
                                // length in bytes	

  U32 industry_header_length;   // Industry specific (fixed format) section
				// header length in bytes

  U32 variable_header_length;   // Length in bytes of variable length section

  U32 total_file_size;          // Total image file size in bytes (includes
				// header, image data and padding if any)

  ASCII version[8];             // Version number of header format

  ASCII file_name[100];         // Image file name

  ASCII creation_date[12];      // Creation date "yyyy:mm:dd"
    
  ASCII creation_time[12];      // Creation time "hh:mm:ssxxx" (xxx for timezone)

  ASCII reserved[36];           // reserved for future use

} FileInformation;


typedef struct _channel_information {
  U8    byte_0;  // see table
  U8    byte_1;  // see table 1
  U8    bpp;     // bits per pixel
  U8    _unused; // 1 byte space for word alignment
  U32	pixels_per_line; // pixels per line
  U32	lines_per_image; // lines per image
  R32	min_data_value;  // Minimum data value
  R32	min_quantity;    // Minimum quantity represented
  R32	max_data_value;  // Maximum data value
  R32	max_quantity;    // Maximum quantity represented
} ImageInformationChannelInformation;

typedef struct image_information {
  U8 orientation;               // orientation enumeration:
                                // val 	line scan dir	page scan direction
				// ---	-------------	-------------------
				//   0	left to right	top to bottom
				//   1	left to right	bottom to top
				//   2	right to left	top to bottom
				//   3	right to left	bottom to top
				//   4	top to bottom	left to right
				//   5	top to bottom	right to left
				//   6	bottom to top	left to right
				//   7	bottom to top	right to left

  U8 channel_count;             // Number of channels ( 1 - 8 )

  U8 _unused_001;               // UNUSED (2 byte space for word alignment)
  U8 _unused_002;

  ImageInformationChannelInformation channel_information[8];

  // Table 1:
  // Channel Designator Codes:

  // byte_0     0 = Universal metric
  // 		1 - 254 = vendor specific (i.e. 1 = Kodak);

  // byte_1	if Byte 0 = 0 
  // 	        Universal Metric
  // 	        0 = B & W
  //            1 = red        (r,g,b printing density)
  //            2 = green      (r,g,b printing density)
  //            3 = blue       (r,g,b printing density)
  //            4 = red        (r,g,b CCIR XA/11)
  //            5 = green      (r,g,b CCIR XA/11)
  //            6 = blue       (r,g,b CCIR XA/11)
  //            7 - 254        TBD - reserved
  
  //            if 0 < byte_0 < 255
  //            0 - 254        Vendor defined

  R32 white_point_x;          // White point (colour temperature) - x, y pair
  R32 white_point_y;          // White point (colour temperature) - x, y pair

  R32 red_primary_x;          // Red primary chromaticity - x, y pair 
  R32 red_primary_y;          // Red primary chromaticity - x, y pair 

  R32 green_primary_x;        // Green primary chromaticity - x, y pair 
  R32 green_primary_y;        // Green primary chromaticity - x, y pair 

  R32 blue_primary_x;         // Blue primary chromaticity - x, y pair 
  R32 blue_primary_y;         // Blue primary chromaticity - x, y pair 

  ASCII label[200];           // Label text (other label info in user
                              // area - font, size, location)

  ASCII reserved[28];         // Reserved for future use

} ImageInformation;


typedef struct imagedataformatinformation { 

  U8 interleave;                // Data interleave (if all channels are not
                                // the same sptial resolution, data interleave must be 2, channel interleave)
				// 0 = pixel interleave ( rgbrgbrgbrgb... )
				// 1 = line interleave (rrr.ggg.bbb.rrr.ggg.bbb.)
				// 2 = channel interleave (rrr...ggg...bbb...)
				// 3 - 254 = user defined

  U8 packing;                   // Packing (see note 1 )
				// 0 = user all bits( bitfields) - TIGHTEST - no byte, word or long word boundaries
                                // 1 = byte ( 8 bit ) boundaries - left justified
                                // 2 = byte ( 8 bit ) boundaries - right justified
                                // 3 = word (16 bit) bondaries - left justified
                                // 4 = word ( 16 bit) boundaries - right justified
                                // 5 = longword ( 32 bit ) boundaries - left justified
                                // 6 = longword ( 32 bit ) boundaries - right justified 

                                // High order bit = 0 - pack at most one pixel per cell
                                // High order bit = 1 - pack at many fields as possible per cell

  U8 data_signed;               // Data signed or unsigned
				// 0 = unsigned
				// 1 = signed

  U8 sense;                     // Image sense
				// 0 = positive image
				// 1 = negative image

  U32 eol_padding;              // end of line padding - number of bytes
  U32 eoc_padding;              // end of channel padding - number of bytes

  ASCII reserved[20];           // reserved for future use

} ImageDataFormatInformation;


// Note 1 (on "packing" options 1-6)
// Define a CELL to be a BYTE (8 bits), WORD (16 bits) or LONGWORD (32bits).
//
// Define a FIELD to be one occurence of a channel value. For example, with 3 channels 
// (r,g,b), pixel interleaved, field 1 is r1, field 2 is g1, field 3 is b1, field 4 is r2, etc. 
// With 3 channels (r, g, b), channel interleaved, field 1 is r1, field 2 is r2, field 3 is r3, etc.
//
// The high order bit of the packing specifier either restricts packing to at most one pixel 
// (n channels) per cell, or allows fields from adjacent pixels to spill over cell boundaries.
//
// How to intepret PACKING specifier
//
// If number of channels = 1 OR data interleave = 1 or 2 (line or channel interleave):
//   Pack as many fields into the cell as possible, with appropriate justification, then align 
//   on the next cell boundary. The high order bit is a "don't care" in this case.
//
// If number of channels is > 1 AND data interleave = 0 (pixel interleave)
//
//   If high orer bit is clear
//
//      n = number of channels
//
//      Pack as many fields into the cell as posssible up to n fields, with appropriate 
//      justification, then align on the next cell boundary. 
//
//   If the high order bit is set
//
//      Pack as many fields into the cell as possible, with appropriate justification, then alighn on the next cell boundary. 
//
// Examples
//
//     Number of channels = 4	6 bits 	6 bits 	6 bits	6 bits 	8 bits
//     All channels, 6 bits deep	field1	field2	field3	field4	empty
//     Data interleave = 0		ch1[1]	ch2[1]	ch3[1]	ch4[1]	xxxxxxxx
//     Packing = 5 (High order bit clear)
//
//     Number of channels = 4	6 bits 	6 bits 	6 bits	6 bits 	6 bits 2 bits
//     All channels, 6 bits deep	field1	field2	field3	field4	field5 empty
//     Data interleave = 0		ch1[1]	ch2[1]	ch3[1]	ch4[1]	ch1[2]xx
//     Packing = 5 (High order bit set)




typedef struct image_origination_information {
  S32 x_offset;                    // X offset (correlate digital data to source media)
  S32 y_offset;                    // Y offset (correlate digital data to source media)

  ASCII filename[100];             // image filename
  ASCII creation_date[12];         // creation date (i.e. "yyyy:mm:dd" )
  ASCII creation_time[12];         // creation time (i.e. "hh:mm:ssxxx" where xxx is time zone (i.e PST))
  ASCII input_device[64];          // input device
  ASCII device_model[32];          // input device model number
  ASCII device_serial_number[32];  // input device serial number
  R32 x_input_device_pitch;        // X input device pitch (samples/mm) (X determined by image orientation)
  R32 y_input_device_pitch;        // Y input device pitch (samples/mm) (Y determined by image orientation)
  U32 gamma;                       // Image gamma of capture device
	//R32 gamma;                       // Image gamma of capture device
  ASCII reserved[40];              // Reserved for future use

} ImageOriginationInformation;


typedef struct motion_picture_information {
  U8 mp_001;
  U8 mp_002;
  U8 mp_003;
  U8 mp_004;
  U32 mp_005;
  U32 mp_006;
  ASCII mp_007[32];
  U32 mp_008;
  R32 mp_009;
  ASCII mp_010[32];
  ASCII mp_011[200];
  ASCII reserved[740];
} MotionPictureInformation;


#endif

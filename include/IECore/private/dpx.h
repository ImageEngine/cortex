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

#ifndef IECORE_DPX_H
#define IECORE_DPX_H

// these structures ripped from:
// http://www.cineon.com/ff_draft.php

// definitions
#define U32 unsigned int
#define ASCII unsigned char
#define U16 unsigned short
#define R32 float
#define U8 unsigned char

namespace IECore {

typedef struct file_information {
  U32   magic;        /* magic number 0x53445058 (SDPX) or 0x58504453 (XPDS) */
  U32   image_data_offset;/* offset to image data in bytes */
  ASCII vers[8];          /* which header format version is being used (v1.0)*/
  U32   file_size;        /* file size in bytes */
  U32   ditto_key;        /* read time short cut - 0 = same, 1 = new */
  U32   gen_hdr_size;     /* generic header length in bytes */
  U32   ind_hdr_size;     /* industry header length in bytes */
  U32   user_data_size;   /* user-defined data length in bytes */
  ASCII file_name[100];   /* image file name */
  ASCII create_time[24];  /* file creation date "yyyy:mm:dd:hh:mm:ss:LTZ" */
  ASCII creator[100];     /* file creator's name */
  ASCII project[200];     /* project name */
  ASCII copyright[200];   /* right to use or copyright info */
  U32   key;              /* encryption ( FFFFFFFF = unencrypted ) */
  ASCII Reserved[104];    /* reserved field TBD (need to pad) */
} DPXFileInformation;

typedef struct _image_information {
  U16    orientation;          /* image orientation */
  U16    element_number;       /* number of image elements */
  U32    pixels_per_line;      /* or x value */
  U32    lines_per_image_ele;  /* or y value, per element */
  struct _image_element {
    U32    data_sign;        /* data sign (0 = unsigned, 1 = signed ) */
    U32    ref_low_data;     /* reference low data code value */
    R32    ref_low_quantity; /* reference low quantity represented */
    U32    ref_high_data;    /* reference high data code value */
    R32    ref_high_quantity;/* reference high quantity represented */
    U8     descriptor;       /* descriptor for image element */
    U8     transfer;         /* transfer characteristics for element */
    U8     colorimetric;     /* colormetric specification for element */
    U8     bit_size;         /* bit size for element */
    U16    packing;          /* packing for element */
    U16    encoding;         /* encoding for element */
    U32    data_offset;      /* offset to data of element */
    U32    eol_padding;      /* end of line padding used in element */
    U32    eo_image_padding; /* end of image padding used in element */
    ASCII  description[32];  /* description of element */
  } image_element[8];         
  
  U8 reserved[52];             /* reserved for future use (padding) */
} DPXImageInformation;

typedef struct _image_orientation {
  U32   x_offset;               /* X offset */
  U32   y_offset;               /* Y offset */
  R32   x_center;               /* X center */
  R32   y_center;               /* Y center */
  U32   x_orig_size;            /* X original size */
  U32   y_orig_size;            /* Y original size */
  ASCII file_name[100];         /* source image file name */
  ASCII creation_time[24];      /* source image creation date and time */
  ASCII input_dev[32];          /* input device name */
  ASCII input_serial[32];       /* input device serial number */
  U16   border[4];              /* border validity (XL, XR, YT, YB) */
  U32   pixel_aspect[2];        /* pixel aspect ratio (H:V) */
  U8    reserved[28];           /* reserved for future use (padding) */
} DPXImageOrientation;

typedef struct _motion_picture_film_header
{
    ASCII film_mfg_id[2];    /* film manufacturer ID code (2 digits from film edge code) */
    ASCII film_type[2];      /* file type (2 digits from film edge code) */
    ASCII offset[2];         /* offset in perfs (2 digits from film edge code)*/
    ASCII prefix[6];         /* prefix (6 digits from film edge code) */
    ASCII count[4];          /* count (4 digits from film edge code)*/
    ASCII format[32];        /* format (i.e. academy) */
    U32   frame_position;    /* frame position in sequence */
    U32   sequence_len;      /* sequence length in frames */
    U32   held_count;        /* held count (1 = default) */
    R32   frame_rate;        /* frame rate of original in frames/sec */
    R32   shutter_angle;     /* shutter angle of camera in degrees */
    ASCII frame_id[32];      /* frame identification (i.e. keyframe) */
    ASCII slate_info[100];   /* slate information */
    U8    reserved[56];      /* reserved for future use (padding) */
} DPXMotionPictureFilm;

typedef struct _television_header
{
    U32 time_code;           /* SMPTE time code */
    U32 userBits;            /* SMPTE user bits */
    U8  interlace;           /* interlace ( 0 = noninterlaced, 1 = 2:1 interlace*/
    U8  field_num;           /* field number */
    U8  video_signal;        /* video signal standard (table 4)*/
    U8  unused;              /* used for byte alignment only */
    R32 hor_sample_rate;     /* horizontal sampling rate in Hz */
    R32 ver_sample_rate;     /* vertical sampling rate in Hz */
    R32 frame_rate;          /* temporal sampling rate or frame rate in Hz */
    R32 time_offset;         /* time offset from sync to first pixel */
    R32 gamma;               /* gamma value */
    R32 black_level;         /* black level code value */
    R32 black_gain;          /* black gain */
    R32 break_point;         /* breakpoint */
    R32 white_level;         /* reference white level code value */
    R32 integration_times;   /* integration time(s) */
    U8  reserved[76];        /* reserved for future use (padding) */
} DPXTelevisionHeader;

};

#endif

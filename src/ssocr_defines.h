/*
* SegmentDisplayOCR AviSynth Filter
* Copyright (C) 2004-2013 Erik Auerswald <auerswal@unix-ag.uni-kl.de>
* Copyright (C) 2013 Cristiano Fontana <fontanacl@ornl.gov>
* Copyright (C) 2014 Lcferrum <lcferrum@yandex.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SSOCR_DEFINES_H
#define SSOCR_DEFINES_H

#define OCRF_VERSION_STRING "v 1.0"

/* filter defaults */
#define OCRF_LOG_FILE ""
#define OCRF_LOG_APPEND true
#define OCRF_INTERVAL 1
#define OCRF_TIME_FORMAT "seconds"
#define OCRF_DEBUG true
#define OCRF_LOCALIZED_OUTPUT true
#define OCRF_INVERTED false
#define OCRF_THRESHOLD "50"

/* a one is recognized by width/height ratio < ONE_RATIO */
#define ONE_RATIO_NUM 1
#define ONE_RATIO_DEN 3

/* a minus sign is recognized by height/width ratio < MINUS_RATIO */
#define MINUS_RATIO_NUM 1
#define MINUS_RATIO_DEN 3

/* a colon is recognized by height/max_height ratio < COLON_RATIO */
#define COLON_RATIO_NUM 2
#define COLON_RATIO_DEN 3

/* a dot (decimal point/thousands separator) is recognized by 
* height/max_height and width/max_height ratio < DOT_RATIO */
#define DOT_RATIO_NUM 1
#define DOT_RATIO_DEN 6

/* to find segment need # of pixels */
#define NEED_PIXELS 1

/* ignore # of pixels when checking a column fo black or white */
#define IGNORE_PIXELS 0

/* segments
 *
 *  A     -
 * F B   | |
 *  G     -
 * E C   | |
 *  D     -
 *
 *  */
#define HORIZ_UP 1
#define VERT_LEFT_UP 2
#define VERT_RIGHT_UP 4
#define HORIZ_MID 8
#define VERT_LEFT_DOWN 16
#define VERT_RIGHT_DOWN 32
#define HORIZ_DOWN 64
#define ALL_SEGS 127
#define DECIMAL 128
#define MINUS 256
#define COLON 512

/* digits */
#define D_ZERO (ALL_SEGS & ~HORIZ_MID)
#define D_ONE (VERT_RIGHT_UP | VERT_RIGHT_DOWN)
#define D_TWO (ALL_SEGS & ~(VERT_LEFT_UP | VERT_RIGHT_DOWN))
#define D_THREE (ALL_SEGS & ~(VERT_LEFT_UP | VERT_LEFT_DOWN))
#define D_FOUR (ALL_SEGS & ~(HORIZ_UP | VERT_LEFT_DOWN | HORIZ_DOWN))
#define D_FIVE (ALL_SEGS & ~(VERT_RIGHT_UP | VERT_LEFT_DOWN))
#define D_SIX (ALL_SEGS & ~VERT_RIGHT_UP)
#define D_SEVEN (HORIZ_UP | VERT_RIGHT_UP | VERT_RIGHT_DOWN)
#define D_ALTSEVEN (VERT_LEFT_UP | D_SEVEN)
#define D_EIGHT ALL_SEGS
#define D_NINE (ALL_SEGS & ~VERT_LEFT_DOWN)
#define D_ALTNINE (ALL_SEGS & ~(VERT_LEFT_DOWN | HORIZ_DOWN))
#define D_DECIMAL DECIMAL
#define D_MINUS MINUS
#define D_COLON COLON
#define D_HEX_A (ALL_SEGS & ~HORIZ_DOWN)
#define D_HEX_b (ALL_SEGS & ~(HORIZ_UP | VERT_RIGHT_UP))
#define D_HEX_C (ALL_SEGS & ~(VERT_RIGHT_UP | HORIZ_MID | VERT_RIGHT_DOWN))
#define D_HEX_c (HORIZ_MID | VERT_LEFT_DOWN | HORIZ_DOWN)
#define D_HEX_d (ALL_SEGS & ~(HORIZ_UP | VERT_LEFT_UP))
#define D_HEX_E (ALL_SEGS & ~(VERT_RIGHT_UP | VERT_RIGHT_DOWN))
#define D_HEX_F (ALL_SEGS & ~(VERT_RIGHT_UP | VERT_RIGHT_DOWN | HORIZ_DOWN))
#define D_UNKNOWN 0

/* various enums */
enum SsocrThreshold {ABSOLUTE_THRESHOLD, ITERATIVE_THRESHOLD, ADAPTIVE_THRESHOLD};
enum SsocrStates {DARK, LIGHT, UNKNOWN};

/* maximum RGB component value */
#define MAXRGB 255

/* doubles are assumed equal when they differ less than EPSILON */
#define EPSILON 0.0000001

#endif //SSOCR_DEFINES_H

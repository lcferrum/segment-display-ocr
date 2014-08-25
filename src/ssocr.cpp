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

#include <windows.h>
#include "ssocr.h"

const unsigned char Ssocr::red[3]={76, 84, 255};
const unsigned char Ssocr::blue[3]={29, 255, 107};
const unsigned char Ssocr::green[3]={149, 43, 21};
const unsigned char Ssocr::gray[3]={127, 128, 128};

Ssocr::Ssocr(bool black_on_white, double thresh, SsocrThreshold thresh_flags):
	thresh(thresh), thresh_flags(thresh_flags), black_on_white(black_on_white), recognized_digits()
{}

Ssocr& Ssocr::Recognize(const SsocrImg &input, SsocrImg *output, const char* dec_sep, const char* neg_sign)
{
	int number_of_digits=0; /* found this number of digits */
	int w, h; /* width, height */
	SsocrStates col=UNKNOWN; /* is column dark or light? */
	SsocrStates row=UNKNOWN; /* is row dark or light? */
	int max_dig_h=0, max_dig_w=0; /* maximum height & width of digits found */
	bool find_dark; /* state of search */
	int found_pixels=0; /* how many pixels are already found */
	double abs_thresh; /* absolute threshold */

	std::vector<digit_struct> digits; /* position of digits in image */
	recognized_digits.clear();

	/* adapt threshold to image */
	abs_thresh=input.adapt_threshold(thresh, 0, 0, -1, -1, thresh_flags);

	/* get image parameters */
	w=input.GetWidth();
	h=input.GetHeight();
	if (output&&(output->GetHeight()!=h||output->GetWidth()!=w))
		return *this;

	/* horizontal partition */
	find_dark=true;
	for (int i=0; i<w; i++) {
		/* check if column is completely light or not */
		col=UNKNOWN;
		found_pixels=0;
		for (int j=0; j<h; j++) {
			if (input.is_pixel_set(i, j, abs_thresh, black_on_white)) { /* dark */
				found_pixels++;
				if (found_pixels>IGNORE_PIXELS) /* 1 dark pixels darken the whole column */
					col=DARK;
			} else if (col==UNKNOWN) /* light */
				col=LIGHT;
		}
		/* save digit position and draw partition line for DEBUG */
		if (find_dark&&col==DARK) {
			/* beginning of digit */
			digits.push_back(digit_struct());
			digits[number_of_digits].x1=i;
			digits[number_of_digits].y1=0;
			if (output)
				output->DrawYuvVerticalLine(i, 0, h-1, red); /* red line for start of digit */
			find_dark=false;
		} else if (!find_dark&&col==LIGHT) {
			/* end of digit */
			digits[number_of_digits].x2=i;
			digits[number_of_digits].y2=h-1;
			number_of_digits++;
			if (output)
				output->DrawYuvVerticalLine(i, 0, h-1, blue); /* blue line for end of digit */
			find_dark=true;
		}
	}

	/* after the loop above the program should be in state FIND_DARK,
	* i.e. after the last digit some light was found
	* if it is still searching for light end the digit at the border of the
	* image */
	if (!find_dark) {
		digits[number_of_digits].x2=w-1;
		digits[number_of_digits].y2=h-1;
		number_of_digits++;
		find_dark=true;
	}

	/* find upper and lower boundaries of every digit */
	for (int d=0; d<number_of_digits; d++) {
		bool found_top=false;
		find_dark=true;
		/* start from top of image and scan rows for dark pixel(s) */
		for (int j=0; j<h; j++) {
			row=UNKNOWN;
			found_pixels=0;
			/* is row dark or light? */
			for (int i=digits[d].x1; i<=digits[d].x2; i++) {
				if (input.is_pixel_set(i, j, abs_thresh, black_on_white)) { /* dark */
					found_pixels++;
					if (found_pixels>IGNORE_PIXELS) /* 1 pixels darken row */
						row=DARK;
				} else if (row==UNKNOWN)
					row=LIGHT;
			}
			/* save position of digit and draw partition line for DEBUG */
			if (find_dark&&row==DARK) {
				if (found_top) { /* then we are searching for the bottom */
					digits[d].y2=j;
					find_dark=false;
					if (output)
						output->DrawYuvHorizontalLine(digits[d].x1, digits[d].x2, digits[d].y2, green); /* green line */
				} else { /* found the top line */
					digits[d].y1=j;
					found_top=true;
					find_dark=false;
					if (output)
						output->DrawYuvHorizontalLine(digits[d].x1, digits[d].x2, digits[d].y1, green); /* green line */
				}
			} else if (!find_dark&&row==LIGHT) {
				/* found_top has to be true because otherwise we were still looking for
				* dark */
				digits[d].y2=j;
				find_dark=true;
 				if (output)
					output->DrawYuvHorizontalLine(digits[d].x1, digits[d].x2, digits[d].y2, green); /* green line */
			}
		}
		/* if we are still looking for light, use the bottom */
		if (!find_dark) {
			digits[d].y2=h-1;
			find_dark=true;
			if (output)
				output->DrawYuvHorizontalLine(digits[d].x1, digits[d].x2, digits[d].y2, green); /* green line */
		}
	}
	
	/* determine maximum digit dimensions */
	for (int d=0; d<number_of_digits; d++) {
		digits[d].w=digits[d].x2-digits[d].x1;
		digits[d].h=digits[d].y2-digits[d].y1;

		if (max_dig_w<digits[d].w)
			max_dig_w=digits[d].w;
		if (max_dig_h<digits[d].h)
			max_dig_h=digits[d].h;

		if (output) /* draw rectangles around digits */
			output->DrawYuvRectangle(digits[d].x1, digits[d].y1, digits[d].x2, digits[d].y2, gray); /* gray rectangle */
	}

	/* at this point the digit 1, colon, decimal point (or thousands separator)
	* and minus sign can be identified by relative size */
	for (int d=0; d<number_of_digits; d++) {
		/* skip digits with zero dimensions */
		if (!digits[d].w||!digits[d].h)
			continue;

		/* if width of digit is less than ONE_RATIO of its height it is a 1
		* (the default 1/3 is arbitarily chosen -- normally seven segment
		* displays use digits that are 2 times as high as wide) and if height 
		* is less than COLON_RATIO of the maximum digit height it is colon */
		if (ONE_RATIO_NUM*digits[d].h>ONE_RATIO_DEN*digits[d].w) {
			if (COLON_RATIO_NUM*max_dig_h>COLON_RATIO_DEN*digits[d].h)
				digits[d].digit=D_COLON;
			else
				digits[d].digit=D_ONE;
			continue;
		}

		/* if height of a digit is less than DOT_RATIO of the maximum digit height,
		* and its width is also less than DOT_RATIO of the maximum digit height, 
		* assume it is a decimal point */
		if ((DOT_RATIO_NUM*max_dig_h>DOT_RATIO_DEN*digits[d].h)&&
			(DOT_RATIO_NUM*max_dig_h>DOT_RATIO_DEN*digits[d].w)) {
			digits[d].digit=D_DECIMAL;
			continue;
		}

		/* if height of digit is less than MINUS_RATIO of its height it is a minus
		* (the default 1/3 is arbitarily chosen -- normally seven segment
		* displays use digits that are 2 times as high as wide) */
		if (MINUS_RATIO_NUM*digits[d].w>=MINUS_RATIO_DEN*digits[d].h) {
			digits[d].digit=D_MINUS;
		}
 	}

	/* now the digits are located and they have to be identified */
	/* iterate over digits */
	for (int d=0; d<number_of_digits; d++) {
		int middle=0, quarter=0, three_quarters=0; /* scanlines */
		/* if digits[d].digit == D_ONE/D_DECIMAL/D_MINUS/D_COLON do nothing */
		if (digits[d].digit==D_UNKNOWN) {
			int third=1; /* in which third we are */
			int half;
			/* check horizontal segments */
			/* vertical scan at x == middle */
			middle=(digits[d].x1+digits[d].x2)/2;
			for (int j=digits[d].y1; j<=digits[d].y2; j++) {
				if (input.is_pixel_set(middle, j, abs_thresh, black_on_white)) { /* dark i.e. pixel is set */
					if (output)
						if (third==1)
							output->SetYuvPixel(middle, j, red);
						else if (third==2)
							output->SetYuvPixel(middle, j, green);
						else if (third==3)
							output->SetYuvPixel(middle, j, blue);
					found_pixels++;
				}
				/* pixels in first third count towards upper segment */
				if (j>=digits[d].y1+digits[d].h/3&&third==1) {
					if (found_pixels>=NEED_PIXELS)
						digits[d].digit|=HORIZ_UP; /* add upper segment */
					found_pixels=0;
					third++;
				} else if (j>=digits[d].y1+2*digits[d].h/3&&third==2) {
					/* pixels in second third count towards middle segment */
					if (found_pixels>=NEED_PIXELS)
						digits[d].digit|=HORIZ_MID; /* add middle segment */
					found_pixels=0;
					third++;
				}
			}
			/* found_pixels contains pixels of last third */
			if (found_pixels>=NEED_PIXELS)
				digits[d].digit|=HORIZ_DOWN; /* add lower segment */
			found_pixels=0;
			/* check upper vertical segments */
			half=1; /* in which half we are */
			quarter=digits[d].y1+digits[d].h/4;
			for (int i=digits[d].x1; i<=digits[d].x2; i++) {
				if (input.is_pixel_set(i, quarter, abs_thresh, black_on_white)) { /* dark i.e. pixel is set */
					if (output)
						if (half==1)
							output->SetYuvPixel(i, quarter, red);
						else if (half==2)
							output->SetYuvPixel(i, quarter, green);
					found_pixels++;
				}
				if (i>=middle&&half==1) {
					if (found_pixels>=NEED_PIXELS)
						digits[d].digit|=VERT_LEFT_UP;
					found_pixels=0;
					half++;
				}
			}
			if (found_pixels>=NEED_PIXELS)
				digits[d].digit|=VERT_RIGHT_UP;
			found_pixels=0;
			/* check lower vertical segments */
			half=1; /* in which half we are */
			three_quarters=digits[d].y1+3*digits[d].h/4;
			for (int i=digits[d].x1; i<=digits[d].x2; i++) {
				if (input.is_pixel_set(i, three_quarters, abs_thresh, black_on_white)) { /* dark i.e. pixel is set */
					if (output)
						if (half==1)
							output->SetYuvPixel(i, three_quarters, red);
						else if (half==2)
							output->SetYuvPixel(i, three_quarters, green);
					found_pixels++;
				}
				if (i>=middle&&half==1) {
					if (found_pixels>=NEED_PIXELS)
						digits[d].digit|=VERT_LEFT_DOWN;
					found_pixels=0;
					half++;
				}
			}
			if (found_pixels>=NEED_PIXELS)
				digits[d].digit|=VERT_RIGHT_DOWN;
			found_pixels = 0;
		}
	}

	for (std::vector<digit_struct>::iterator it=digits.begin(); it!=digits.end(); it++)
		switch(it->digit) {
			case D_ZERO: 
				recognized_digits.push_back('0');
				break;
			case D_ONE: 
				recognized_digits.push_back('1'); 
				break;
			case D_TWO: 
				recognized_digits.push_back('2'); 
				break;
			case D_THREE: 
				recognized_digits.push_back('3'); 
				break;
			case D_FOUR: 
				recognized_digits.push_back('4'); 
				break;
			case D_FIVE: 
				recognized_digits.push_back('5'); 
				break;
			case D_SIX: 
				recognized_digits.push_back('6'); 
				break;
			case D_SEVEN: /* fallthrough */
			case D_ALTSEVEN: 
				recognized_digits.push_back('7'); 
				break;
			case D_EIGHT: 
				recognized_digits.push_back('8'); 
				break;
			case D_NINE: /* fallthrough */
			case D_ALTNINE: 
				recognized_digits.push_back('9'); 
				break;
			case D_DECIMAL: 
				recognized_digits.append(dec_sep); 
				break;
			case D_MINUS: 
				recognized_digits.append(neg_sign); 
				break;
			case D_COLON: 
				recognized_digits.push_back(':'); 
				break;
			case D_HEX_A: 
				recognized_digits.push_back('a'); 
				break;
			case D_HEX_b: 
				recognized_digits.push_back('b'); 
				break;
			case D_HEX_C: /* fallthrough */
			case D_HEX_c: 
				recognized_digits.push_back('c'); 
				break;
			case D_HEX_d: 
				recognized_digits.push_back('d'); 
				break;
			case D_HEX_E: 
				recognized_digits.push_back('e'); 
				break;
			case D_HEX_F: 
				recognized_digits.push_back('f'); 
				break;
			/* finding a digit with no set segments is not supposed to happen */
			case D_UNKNOWN: 
				recognized_digits.push_back('?'); 
				break;
			default: 
				recognized_digits.push_back('?'); 
				break;
		}

	return *this;
}

std::string Ssocr::GetLastRecognizedDigits()
{
	return recognized_digits;
}
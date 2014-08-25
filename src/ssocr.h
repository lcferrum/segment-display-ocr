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

#ifndef SSOCR_H
#define SSOCR_H

#include <string>
#include <vector>
#include "ssocr_defines.h"
#include "ssocr_imgproc.h"

class Ssocr {
private: 
	struct digit_struct {
		int x1, y1, x2, y2, w, h, digit;
	};

	static const unsigned char red[3];
	static const unsigned char blue[3];
	static const unsigned char green[3];
	static const unsigned char gray[3];

	double thresh; /* border between light and dark */
	SsocrThreshold thresh_flags; /* see ssocr_defines.h file */
	bool black_on_white;
	std::string recognized_digits;
public:
	Ssocr(bool black_on_white, double thresh, SsocrThreshold thresh_flags);
	Ssocr& Recognize(const SsocrImg &input, SsocrImg *output, const char* dec_sep, const char* neg_sign);
	std::string GetLastRecognizedDigits();
};

#endif //SSOCR_H
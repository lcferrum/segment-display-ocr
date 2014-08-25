/*
* SegmentDisplayOCR AviSynth Filter
* Copyright (C) 2004-2013 Erik Auerswald <auerswal@unix-ag.uni-kl.de>
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

#ifndef SSOCR_IMGPROC_H
#define SSOCR_IMGPROC_H

#include "ssocr_defines.h"
#include "yuvimg.h"

class SsocrImg: public YuvImg {
private: 
	/* clip value thus that it is in the given interval [min,max] */
	int clip(int value, int min, int max) const;
	/* get minimum lum value */
	double get_minval(int x, int y, int w, int h) const;
	/* get maximum luminance value */
	double get_maxval(int x, int y, int w, int h) const;
	/* compute dynamic threshold value from the rectangle (x,y),(x+w,y+h) of source_image */
	double get_threshold(double fraction, int x, int y, int w, int h) const;
	/* determine threshold by an iterative method */
	double iterative_threshold(double thresh, int x, int y, int w, int h) const;
public:
	SsocrImg(const PVideoFrame &src, const VideoInfo &vi);
	/* check if a pixel is set regarding current foreground/background colors */
	bool is_pixel_set(int x, int y, double threshold, bool black_on_white) const;
	/* adapt threshold to image values */
	double adapt_threshold(double thresh, int x, int y, int w, int h, SsocrThreshold thresh_flags) const;
};

#endif //SSOCR_IMGPROC_H
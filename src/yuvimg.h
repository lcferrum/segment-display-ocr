/*
* SegmentDisplayOCR AviSynth Filter
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

#ifndef YUVIMG_H
#define YUVIMG_H

#include "avisynth.h"

class YuvImg {
protected: 
	struct PlaneData {
		unsigned char* ptr;
		int pitch;
        int width;
        int height;
        int width_sub;
        int height_sub;
	};

	int img_height;
	int img_width;
	bool read_only;
	PlaneData yuv_data[3];
public:
	YuvImg(const PVideoFrame &src, const VideoInfo &vi);
	int GetHeight() const;
	int GetWidth() const;
	unsigned char QueryYuvLuma(int x, int y) const;
	void SetYuvPixel(int x, int y, const unsigned char *yuv_color);
	void DrawYuvHorizontalLine(int x1, int x2, int y, const unsigned char *yuv_color);
	void DrawYuvVerticalLine(int x, int y1, int y2, const unsigned char *yuv_color);
	void DrawYuvRectangle(int x1, int y1, int x2, int y2, const unsigned char *yuv_color);
	void MakeMonochrome();
};

#endif //YUVIMG_H
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

#include <algorithm>
#include <windows.h>
#include "yuvimg.h"

YuvImg::YuvImg(const PVideoFrame &src, const VideoInfo &vi):
	img_height(vi.height), img_width(vi.width), yuv_data(), read_only(!src->IsWritable())
{
	const int yuv_planes[3]={PLANAR_Y, PLANAR_U, PLANAR_V};

	for (int p=0; p<3; p++) {
		if (read_only)
			yuv_data[p].ptr=(unsigned char*)src->GetReadPtr(yuv_planes[p]);	
		else
			yuv_data[p].ptr=src->GetWritePtr(yuv_planes[p]);
		yuv_data[p].pitch=src->GetPitch(yuv_planes[p]);
		yuv_data[p].width=src->GetRowSize(yuv_planes[p]);
		yuv_data[p].height=src->GetHeight(yuv_planes[p]);
		yuv_data[p].width_sub=vi.GetPlaneWidthSubsampling(yuv_planes[p]);
		yuv_data[p].height_sub=vi.GetPlaneHeightSubsampling(yuv_planes[p]);
	}
}

int YuvImg::GetHeight() const
{
	return img_height;
}

int YuvImg::GetWidth() const
{
	return img_width;
}

unsigned char YuvImg::QueryYuvLuma(int x, int y) const
{
	if (x<0||y<0||x>=img_width||y>=img_height)
		return 0;
	return *(yuv_data[0].ptr+yuv_data[0].pitch*y+x);
}

void YuvImg::SetYuvPixel(int x, int y, const unsigned char *yuv_color)
{
	if (read_only||x<0||y<0||x>=img_width||y>=img_height)
		return;
	for (int p=0; p<3; p++)
		*(yuv_data[p].ptr+yuv_data[p].pitch*(y>>yuv_data[p].height_sub)+(x>>yuv_data[p].width_sub))=yuv_color[p];
}

void YuvImg::DrawYuvHorizontalLine(int x1, int x2, int y, const unsigned char *yuv_color)
{
	if (read_only||x1<0||x2<0||y<0||x1>=img_width||x2>=img_width||y>=img_height||x1>x2)
		return;
	for (int p=0; p<3; p++) {
		unsigned char* ptr=yuv_data[p].ptr+yuv_data[p].pitch*(y>>yuv_data[p].height_sub);
		std::fill(ptr+(x1>>yuv_data[p].width_sub), ptr+(x2>>yuv_data[p].width_sub)+1, yuv_color[p]); 
	}
}

void YuvImg::DrawYuvVerticalLine(int x, int y1, int y2, const unsigned char *yuv_color)
{
	if (read_only||x<0||y1<0||y2<0||x>=img_width||y1>=img_height||y2>=img_height||y1>y2)
		return;
	for (int p=0; p<3; p++) {
		unsigned char* ptr=yuv_data[p].ptr+yuv_data[p].pitch*(y1>>yuv_data[p].height_sub);
		while (ptr<=yuv_data[p].ptr+yuv_data[p].pitch*(y2>>yuv_data[p].height_sub)) {
			*(ptr+(x>>yuv_data[p].width_sub))=yuv_color[p];
			ptr+=yuv_data[p].pitch;
		}
	}
}

void YuvImg::DrawYuvRectangle(int x1, int y1, int x2, int y2, const unsigned char *yuv_color)
{
	if (read_only||x1<0||x2<0||y1<0||y2<0||x1>=img_width||x2>=img_width||y1>=img_height||y2>=img_height||x1>x2||y1>y2)
		return;
	for (int p=0; p<3; p++) {
		unsigned char* ptr=yuv_data[p].ptr+yuv_data[p].pitch*(y1>>yuv_data[p].height_sub);
		std::fill(ptr+(x1>>yuv_data[p].width_sub), ptr+(x2>>yuv_data[p].width_sub)+1, yuv_color[p]);
		while (ptr<yuv_data[p].ptr+yuv_data[p].pitch*(y2>>yuv_data[p].height_sub)) {
			*(ptr+(x1>>yuv_data[p].width_sub))=yuv_color[p];
			*(ptr+(x2>>yuv_data[p].width_sub))=yuv_color[p];
			ptr+=yuv_data[p].pitch;
		}
		std::fill(ptr+(x1>>yuv_data[p].width_sub), ptr+(x2>>yuv_data[p].width_sub)+1, yuv_color[p]);
	}
}

void YuvImg::MakeMonochrome()
{
	if (read_only)
		return;
	for (int p=1; p<3; p++)
		std::fill(yuv_data[p].ptr, yuv_data[p].ptr+yuv_data[p].pitch*yuv_data[p].height, (unsigned char)128); 
}
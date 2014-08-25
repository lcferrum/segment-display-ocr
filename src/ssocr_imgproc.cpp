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

#include <math.h>
#include <windows.h>
#include "ssocr_imgproc.h"

SsocrImg::SsocrImg(const PVideoFrame &src, const VideoInfo &vi):
	YuvImg(src, vi)
{}

/* clip value thus that it is in the given interval [min,max] */
int SsocrImg::clip(int value, int min, int max) const
{
	return (value<min)?min:((value>max)?max:value);
}

/* check if a pixel is set regarding current foreground/background colors */
bool SsocrImg::is_pixel_set(int x, int y, double treshold, bool black_on_white) const
{
	if (black_on_white==(QueryYuvLuma(x, y)<treshold/100.0*MAXRGB))
		return true;
	else
		return false;
}

/* adapt threshold to image values values */
double SsocrImg::adapt_threshold(double thresh, int x, int y, int w, int h, SsocrThreshold thresh_flags) const
{
	switch (thresh_flags) {
		case ABSOLUTE_THRESHOLD:
			return thresh;
			break;
		case ADAPTIVE_THRESHOLD:
			return get_threshold(thresh/100.0, x, y, w, h);
			break;
		case ITERATIVE_THRESHOLD:
			return iterative_threshold(thresh, x, y, w, h);
			break;
		default:
			return thresh;
			break;
	}
}

/* compute dynamic threshold value from the rectangle (x,y),(x+w,y+h) of source_image */
double SsocrImg::get_threshold(double fraction, int x, int y, int w, int h) const
{
	int xi, yi; /* iteration variables */
	int lum; /* luminance of pixel */
	double minval=(double)MAXRGB, maxval=0.0;

	/* special value -1 for width or height means image width/height */
	if (w==-1) w=img_width;
	if (h==-1) h=img_height;	

	/* assure valid coordinates */
	if (x+w>img_width) x=img_width-w;
	if (y+h>img_height) y=img_height-h;
	if (x<0) x=0;
	if (y<0) y=0;

	/* find the threshold value to differentiate between dark and light */
	for (xi=0; (xi<w)&&(xi<img_width); xi++) {
		for (yi=0; (yi<h)&&(yi<img_height); yi++) {
			lum=QueryYuvLuma(x+xi, y+yi);
			if (lum<minval) minval=lum;
			if (lum>maxval) maxval=lum;
		}
	}

	return (minval+fraction*(maxval-minval))*100/MAXRGB;
}

/* determine threshold by an iterative method */
double SsocrImg::iterative_threshold(double thresh, int x, int y, int w, int h) const
{
	int xi, yi; /* iteration variables */
	int lum; /* luminance of pixel */
	unsigned int size_white, size_black; /* size of black and white groups */
	unsigned long int sum_white, sum_black; /* sum of black and white groups */
	unsigned int avg_white, avg_black; /* average values of black and white */
	double old_thresh; /* old threshold computed by last iteration step */
	double new_thresh; /* new threshold computed by current iteration step */
	int thresh_lum; /* luminance value of threshold */

	/* adjusting threshold to image */
	thresh=get_threshold(thresh/100.0, x, y, w, h);

	/* normalize threshold (was given as a percentage) */
	new_thresh=thresh/100.0;

	/* special value -1 for width or height means image width/height */
	if (w==-1) w=img_width;
	if (h==-1) h=img_height;	

	/* assure valid coordinates */
	if (x+w>img_width) x=img_width-w;
	if (y+h>img_height) y=img_height-h;
	if (x<0) x=0;
	if (y<0) y=0;

	/* find the threshold value to differentiate between dark and light */
	do {
		thresh_lum=(int)(MAXRGB*new_thresh);
		old_thresh=new_thresh;
		size_black=sum_black=size_white=sum_white = 0;
		for(xi=0; (xi<w)&&(xi<img_width); xi++) {
			for(yi=0; (yi<h)&&(yi<img_height); yi++) {
				lum=QueryYuvLuma(xi, yi);
				if (lum<=thresh_lum) {
					size_black++;
					sum_black+=lum;
				} else {
					size_white++;
					sum_white+=lum;
				}
			}
		}
		if (!size_white) 
			return thresh;
		if (!size_black)
			return thresh;
		avg_white=sum_white/size_white;
		avg_black=sum_black/size_black;
		new_thresh=(avg_white+avg_black)/(2.0*MAXRGB);
	} while (fabs(new_thresh-old_thresh)>EPSILON);

	return new_thresh*100;
}

/* get minimum lum value */
double SsocrImg::get_minval(int x, int y, int w, int h) const
{
	int xi, yi; /* iteration variables */
	int minval=MAXRGB;
	int lum=0;

	/* special value -1 for width or height means image width/height */
	if(w==-1) w=img_width;
	if(h==-1) h=img_height;	

	/* assure valid coordinates */
	if (x+w>img_width) x=img_width-w;
	if (y+h>img_height) y=img_height-h;
	if (x<0) x=0;
	if (y<0) y=0;

	/* find the minimum value in the image */
	for(xi=0; (xi<w)&&(xi<img_width); xi++) {
		for(yi=0; (yi<h)&&(yi<img_height); yi++) {
			lum=clip(QueryYuvLuma(xi, yi), 0, 255);
			if (lum<minval) minval=lum;
		}
	}

	return minval;
}

/* get maximum luminance value */
double SsocrImg::get_maxval(int x, int y, int w, int h) const
{
	int xi, yi; /* iteration variables */
	int lum=0;
	int maxval=0;

	/* special value -1 for width or height means image width/height */
	if (w==-1) w=img_width;
	if (h==-1) h=img_height;

	/* assure valid coordinates */
	if (x+w>img_width) x=img_width-w;
	if (y+h>img_height) y=img_height-h;
	if (x<0) x=0;
	if (y<0) y=0;

	/* find the minimum value in the image */
	for(xi=0; (xi<w)&&(xi<img_width); xi++) {
		for(yi=0; (yi<h)&&(yi<img_height); yi++) {
			lum=clip(QueryYuvLuma(xi, yi), 0, 255);
			if (lum>maxval) maxval=lum;
		}
	}

	return maxval;
}
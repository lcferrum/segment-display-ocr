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

#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <windows.h>
#include "yuvimg.h"
#include "ssocr_imgproc.h"
#include "ssocr_defines.h"
#include "ocrf.h"

const AVS_Linkage *AVS_linkage=NULL;

//Filter object is created when AVS file is first read by AviSynth
//Filters are created in order of appearance in AVS file
OCRFilter::OCRFilter(PClip child, const char* log_file, bool log_append, int interval, const char* time_format, bool debug, bool localized_output, bool inverted, const char* threshold, IScriptEnvironment *env):
	GenericVideoFilter(child),
	fduration(((double)vi.fps_denominator/vi.fps_numerator*1000)), timer(interval*1000), last_frame(-1), time_format(SEC), debug(debug), log_file(), csv_sep(), dec_sep(), neg_sign(), ssocr()
{
	if (interval<0)
		env->ThrowError("SegmentDisplayOCR: interval can't be negative number!");

	if (!strcmp("seconds", time_format)) {
		this->time_format=SEC;
	} else if (!strcmp("mseconds", time_format)) {
		this->time_format=MSEC;
	} else if (!strcmp("timestamp", time_format)) {
		this->time_format=TMS;
	} else if (!strcmp("frame", time_format)) {
		this->time_format=FRAME;
	} else if (!strcmp("realtime", time_format)) {
		this->time_format=RTM;
	} else {
		env->ThrowError("SegmentDisplayOCR: unknown time_format \"%s!\"", time_format);
	}

	if (vi.IsFieldBased()) {
		env->ThrowError("SegmentDisplayOCR: non-interlaced video only!");
	}

	if (!vi.IsPlanar()||!vi.IsYUV()||vi.IsY8()) {
		env->ThrowError("SegmentDisplayOCR: YV12, YV16, YV24 and YV411 video only!");
	}

	if (strlen(log_file)>0) {
		//By default ofstream opens files with RW sharing enabled (_SH_DENYNO)
		//This allows simultaneous write to single log for several filter instances
		//Append mode (std::ios::app) is required for simultaneous write but it's incompatible with truncate (std::ios::trunc)
		if (!log_append)
			CloseHandle(CreateFile(log_file, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
		this->log_file.open(log_file, std::ios::app);
		if (!this->log_file.is_open())
			env->ThrowError("SegmentDisplayOCR: error while opening file \"%s\"!", log_file);
	}

	double thresh;
	SsocrThreshold thresh_flags;
	if (!ParseThreshold(threshold, thresh, thresh_flags))
		env->ThrowError("SegmentDisplayOCR: unrecognized threshold string \"%s\"!", threshold);
	if (thresh<0.0||thresh>100.0)
		env->ThrowError("SegmentDisplayOCR: threshold should be between 0 and 100!");
	ssocr=new Ssocr(!inverted, thresh, thresh_flags);

	if (!(localized_output&&GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SNEGATIVESIGN, neg_sign, 5)))
		strcpy(neg_sign, "-");
	if (!(localized_output&&GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, dec_sep, 4)))
		strcpy(dec_sep, ".");
	if (!(localized_output&&GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, csv_sep, 4)))
		strcpy(csv_sep, ",");
	if (!(localized_output&&GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, sdate_fmt, 80)))
		strcpy(sdate_fmt, "yyyy'-'MM'-'dd");
	if (!(localized_output&&GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, time_fmt, 80)))
		strcpy(time_fmt, "HH':'mm':'ss");
}

//Filter is destroyed when AVS file is closed
OCRFilter::~OCRFilter()
{
	delete ssocr;
	if (log_file.is_open())
		log_file.close();
}

//Rounding algorithm from Java 7
int OCRFilter::Round(double num)
{
	if (num!=0.49999999999999994)
		return (int)floor(num+0.5);
	else
		return 0;
}

//GetFrame is called only when client or parent filter requests frame
PVideoFrame __stdcall OCRFilter::GetFrame(int n, IScriptEnvironment *env)
{
	PVideoFrame src=child->GetFrame(n, env);
	
	//For the sake of optimization, following variables are computed only ones per iteration
	unsigned int cur_mseconds=Round(fduration*n);
	bool newer=IsNewer(n);
	bool alarm=CheckTimer(cur_mseconds);
	std::string timestamp=(debug||(time_format==TMS&&alarm))?GetTimestamp(cur_mseconds):"";

	if (debug) {
		PVideoFrame dst=src;
		env->MakeWritable(&dst);	//MakeWritable creates a writable copy of input frame (read-only original remains valid)
		SsocrImg dst_img(dst, vi);
		dst_img.MakeMonochrome();
		if (alarm) {
			ssocr->Recognize(SsocrImg(src, vi), &dst_img, dec_sep, neg_sign);
			if (newer)
				Log(ssocr->GetLastRecognizedDigits(), timestamp, cur_mseconds, n);
		}
		DebugOSD(env, dst, timestamp, ssocr->GetLastRecognizedDigits(), n, newer, alarm);
		return dst;
	} else {
		if (alarm&&newer) {
			ssocr->Recognize(SsocrImg(src, vi), NULL, dec_sep, neg_sign);
			Log(ssocr->GetLastRecognizedDigits(), timestamp, cur_mseconds, n);
		}
		return src;
	}
}

void OCRFilter::DebugOSD(IScriptEnvironment *env, PVideoFrame &src, const std::string &timestamp, const std::string &value, int cur_frame, bool newer, bool alarm)
{
	int textcolor=0xf0f080;	//Orange
	if (!newer)
		textcolor=0xf08080;	//Red
	else if (alarm)
		textcolor=0x80f080;	//Green

	std::ostringstream out_str;
	out_str<<timestamp<<"\nFRAME: "<<cur_frame<<"\n"<<value;
	env->ApplyMessage(&src, vi, out_str.str().c_str(), vi.width/2, textcolor, 0, 0);
}

std::string OCRFilter::GetTimestamp(unsigned int cur_mseconds)
{
	std::ostringstream out_str;

	unsigned int dsp_mseconds=cur_mseconds%1000;
	unsigned int dsp_seconds=cur_mseconds/1000%60;
	unsigned int dsp_minutes=cur_mseconds/60000%60;
	unsigned int dsp_hours=cur_mseconds/3600000;

	out_str.fill('0');
	if (dsp_hours>99)
		out_str<<"EE";
	else
		out_str<<std::setw(2)<<dsp_hours;
	out_str<<':'<<std::setw(2)<<dsp_minutes<<':'<<std::setw(2)<<dsp_seconds<<'.'<<std::setw(3)<<dsp_mseconds;

	return out_str.str();
}

bool OCRFilter::CheckTimer(unsigned int cur_mseconds)
{
	if (!timer)
		return true;

	unsigned int last_alarm=cur_mseconds-cur_mseconds%timer;
	if (cur_mseconds>=last_alarm&&cur_mseconds<(last_alarm+Round(fduration)))
		return true;
	else
		return false;
}

bool OCRFilter::IsNewer(int cur_frame)
{
	if (cur_frame<=last_frame)
		return false;
	last_frame=cur_frame;
	return true;
}

void OCRFilter::Log(const std::string &digits, const std::string &timestamp, unsigned int cur_mseconds, int cur_frame)
{
	if (log_file.is_open()) {
		SYSTEMTIME lt;
		char *rtm_buf;
		int sdate_len, rtm_len;

		switch (time_format) {
			case TMS:
				log_file<<'"'<<timestamp<<'"';
				break;
			case RTM:
				GetLocalTime(&lt);
				rtm_len=sdate_len=GetDateFormat(LOCALE_USER_DEFAULT, 0, &lt, sdate_fmt, NULL, 0);
				rtm_len+=GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lt, time_fmt, NULL, 0);
				rtm_buf=new char[rtm_len];
				GetDateFormat(LOCALE_USER_DEFAULT, 0, &lt, sdate_fmt, rtm_buf, rtm_len-1);
				rtm_buf[sdate_len-1]=' ';
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lt, time_fmt, rtm_buf+sdate_len, rtm_len-sdate_len-1);
				log_file<<'"'<<rtm_buf<<'"';
				delete rtm_buf;
				break;
			case SEC:
				log_file<<cur_mseconds/1000;
				break;
			case MSEC:
				log_file<<cur_mseconds;
				break;
			case FRAME:
				log_file<<cur_frame;
				break;
		}
		log_file<<csv_sep<<'"'<<digits<<'"'<<std::endl;
	}
}

bool OCRFilter::ParseThreshold(std::string threshold, double &thresh, SsocrThreshold &thresh_flags)
{
	if (!threshold.length())
		return false;

	if (std::isalpha(threshold[threshold.length()-1])) {
		switch (threshold[threshold.length()-1]) {
			case 'i':
				thresh_flags=ITERATIVE_THRESHOLD;
				break;
			case 'a':
				thresh_flags=ABSOLUTE_THRESHOLD;
				break;
			default:
				return false;
				break;
		}
		threshold.resize(threshold.length()-1);
	} else {
		thresh_flags=ADAPTIVE_THRESHOLD;
	}
	std::istringstream iss(threshold);
	iss>>std::noskipws>>thresh;
	if (!iss.eof()||iss.fail())
		return false;
	else
		return true;
}

AVSValue __cdecl OCRFilter::Create(AVSValue args, void* user_data, IScriptEnvironment *env) 
{
	return new OCRFilter(args[0].AsClip(), args[1].AsString(OCRF_LOG_FILE), args[2].AsBool(OCRF_LOG_APPEND), args[3].AsInt(OCRF_INTERVAL), args[4].AsString(OCRF_TIME_FORMAT), args[5].AsBool(OCRF_DEBUG), args[6].AsBool(OCRF_LOCALIZED_OUTPUT), args[7].AsBool(OCRF_INVERTED), args[8].AsString(OCRF_THRESHOLD), env);
}

AVSValue __cdecl OCRFilter::Runtime(AVSValue args, void* user_data, IScriptEnvironment *env)
{
	if (!args[0].IsClip())
		env->ThrowError("RtmSegmentDisplayOCR: no clip supplied!");
	PClip child=args[0].AsClip();
	VideoInfo vi=child->GetVideoInfo();

	AVSValue cn=env->GetVar("current_frame");
	if (!cn.IsInt())
		env->ThrowError("RtmSegmentDisplayOCR: this filter can only be used within ConditionalFilter");
	PVideoFrame src=child->GetFrame(cn.AsInt(), env);
	
	if (vi.IsFieldBased()) {
		env->ThrowError("RtmSegmentDisplayOCR: non-interlaced video only!");
	}

	if (!vi.IsPlanar()||!vi.IsYUV()||vi.IsY8()) {
		env->ThrowError("RtmSegmentDisplayOCR: YV12, YV16, YV24 and YV411 video only!");
	}

	double thresh;
	SsocrThreshold thresh_flags;
	if (!ParseThreshold(args[2].AsString(OCRF_THRESHOLD), thresh, thresh_flags))
		env->ThrowError("RtmSegmentDisplayOCR: unrecognized threshold string \"%s\"!", args[2].AsString(OCRF_THRESHOLD));
	if (thresh<0.0||thresh>100.0)
		env->ThrowError("RtmSegmentDisplayOCR: threshold should be between 0 and 100!");

	//AVSValue doesn't make an internal copy of string - it simply stores a pointer to it
	//SaveString copies string into ScriptEnvironment object so AVSValue string remains valid after function returns
	//SaveString frees saved strings only when AVS file is closed - it will eat up memory if used too often
	return env->SaveString(Ssocr(!args[1].AsBool(OCRF_INVERTED), thresh, thresh_flags).Recognize(SsocrImg(src, vi), NULL, ".", "-").GetLastRecognizedDigits().c_str());
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment *env, const AVS_Linkage *vectors) 
{
	AVS_linkage=vectors;
	env->AddFunction("SegmentDisplayOCR", "c[log_file]s[log_append]b[interval]i[time_format]s[debug]b[localized_output]b[inverted]b[threshold]s", OCRFilter::Create, NULL);
	env->AddFunction("RtmSegmentDisplayOCR", "c[inverted]b[threshold]s", OCRFilter::Runtime, NULL);
	return "SegmentDisplayOCR " OCRF_VERSION_STRING;
}
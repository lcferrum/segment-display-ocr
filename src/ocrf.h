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

#ifndef MAIN_H
#define MAIN_H

#include <string>
#include "ssocr.h"
#include "avisynth.h"

class OCRFilter: public GenericVideoFilter {
private: 
	enum TFEnum {TMS, RTM, SEC, MSEC, FRAME};
	double fduration;			//Frame duration in mseconds
	unsigned int timer;			//Timer in mseconds
	int last_frame;				//Last processed frame
	TFEnum time_format;
	bool debug;
	std::ofstream log_file;
	char csv_sep[4];
	char dec_sep[4];
	char neg_sign[5];
	char sdate_fmt[80];
	char time_fmt[80];
	Ssocr *ssocr;

	int Round(double num);
	std::string GetTimestamp(unsigned int cur_mseconds);
	bool CheckTimer(unsigned int cur_mseconds);
	bool IsNewer(int cur_frame);
	void DebugOSD(IScriptEnvironment *env, PVideoFrame &src, const std::string &timestamp, const std::string &value, int cur_frame, bool newer, bool alarm);
	void Log(const std::string &digits, const std::string &timestamp, unsigned int cur_mseconds, int cur_frame);
	static bool ParseThreshold(std::string threshold, double &thresh, SsocrThreshold &thresh_flags);
public:
	OCRFilter(PClip child, const char* log_file, bool log_append, int interval, const char* time_format, bool debug, bool localized_output, bool inverted, const char* threshold, IScriptEnvironment *env);
	~OCRFilter();

	//Overloaded functions:
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
	//SetCacheHints and GetVersion are already properly defined in GenericVideoFilter and IClip respectively
	//GetVersion will return AVISYNTH_INTERFACE_VERSION of current AviSynth C++ API (avisynth.h) so AviSynth won't load this plugin if it has incompatible API version

	//IScriptEnvironment::ApplyFunc callbacks:
	static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment *env);
	static AVSValue __cdecl Runtime(AVSValue args, void* user_data, IScriptEnvironment *env);
};

//Exported functions:
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment *env, const AVS_Linkage *vectors);

#endif //MAIN_H
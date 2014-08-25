SegmentDisplayOCR AviSynth Filter

1. License
----------
Copyright (c) 2014 Lcferrum

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

2. About
--------
SegmentDisplayOCR is a seven-segment display recognition filter for AviSynth.
It has built in logging functionality (it will log frame recognition results)
and also can be used in AviSynth conditional filters. The main purpose of this
filter is to process readings of various digital instruments (e.g. digital
multimeters) captured on video. So if your favourite instrument lacks interface
for connecting it to PC you can capture it's readings on cam and convert them
to computer readable format with SegmentDisplayOCR filter.

3. Where to get
---------------
You can compile SegmentDisplayOCR by yourself (refer to COMPILE.TXT that comes
with the sources) or download binary distribution from Sourceforge:

	https://sourceforge.net/projects/segmentdisplayocr/files/SegmentDisplayOCR/
	
Main project homepage is at GitHub:

	https://github.com/lcferrum/segment-display-ocr

4. Installation
---------------
Install AviSynth (SegmentDisplayOCR requires version 2.6.0 and higher). Refer
to official AviSynth wiki (http://avisynth.nl/index.php/Main_Page) for AviSynth
installation instructions. Don't forget to install necessary codecs (especially
YUV codec).

Copy SegmentDisplayOCR.dll file to AviSynth plugins directory - it's in your
AviSynth installation directory. That's all - now SegmentDisplayOCR will be
automatically loaded by AviSynth every time AVS script is run.

If you are new to AviSynth it would be a good idea to read some tutorials on
official wiki.
	
4. Usage
--------
SegmentDisplayOCR(clip [, string log_file="", bool log_append=true, 
    int interval=1, string time_format="seconds", bool debug=true,
    bool localized_output=true, bool inverted=false, string threshold="50"]) 
RtmSegmentDisplayOCR(clip [, bool inverted=false, string threshold="50"])

SegmentDisplayOCR will try to recognize each frame of input video clip. It
expects to find a clear picture of single seven-segment display in each frame.
If your video is blurry, noisy, distorted and overall of low quality it's
highly recommended to enhance video quality by applying appropriate AviSynth 
filters before SegmentDisplayOCR filter. If you have more than one 
seven-segment display on the video, it's recommended to crop video so it would
contain only single display.

You can use debug feature of SegmentDisplayOCR (turned on by default) to tune
up SegmentDisplayOCR and preceding filters parameters. While in debug mode,
SegmentDisplayOCR will add various debug info to output video that will help
determine if input video is recognized properly. If not in debug mode
SegmentDisplayOCR will output original unchanged video.

SegmentDisplayOCR will try to recognize every N-th frame starting from the 
first one. N is determined by interval variable (in seconds). For example for
PAL video (30 FPS) one second interval means that recognition will occur every
30-th frame. Recognition will only occur during forward playback of the video.
Rewinded frames won't be recognized.

Recognition results can be logged by SegmentDisplayOCR. By default logging is
turned off. Log file format is a CSV consisting of two-field records: time and
recognition result. It's perfectly safe to use single log file for several
instances of SegmentDisplayOCR filter in single AVS script - log won't get
corrupted and will contain results from every instance.

RtmSegmentDisplayOCR is a runtime function based on SegmentDisplayOCR filter.
It can be used with e.g. ConditionalFilter or WriteFile. Returns recognized
digits (as string) for current_frame of input clip.

SegmentDisplayOCR and RtmSegmentDisplayOCR can recognize all digits (0-9),
hexademical characters (A-F), decimal point, colon and minus sign. 
SegmentDisplayOCR and RtmSegmentDisplayOCR expect input video to be
non-interlaced and in one of the following color spaces: YV12, YV16, YV24 or
YV411.

Parameters:

log_file [optional, default: empty string]
    Path to log file. If empty - no logging will occur. If file doesn't exist
    it will be created. Relative path is relative to AVS script location.

log_append [optional, default: true]
    If true - log file will be appended. If false - log file file will be
    truncated.

interval [optional, default: 1]
    Recognition interval in seconds. It means that every N seconds a frame will
    be recognized starting with frame 0. If interval is 0 - every frame will be
    recognized. Interval can't be negative number.

time_format [optional, default: "seconds"]
    String which determines time format for log file. This parameter can be one
    of the following values:
        "seconds"   - seconds elapsed from the video start
        "mseconds"  - millisecond elapsed from the video start
        "timestamp" - time elapsed from the video start in the form of
                      timestamp "HH:MM:SS.mmm"
        "frame"     - number of current frame
        "realtime"  - current system time in the form of timestamp which is 
                      determined by localized_output parameter
    All time formats, except the last one, are calculated based on current
    frame number and counted from the start of the video.

debug [optional, default: true]
    Add debug information to output video. If debug is false - output video is
    complete copy of input. Debug information includes: timestamp, frame
    number, last recognized value and edges of recognized characters. Also,
    input video becomes monochrome in debug mode. Text information is colored
    according to current recognition state: green - current frame is processed,
    orange - current frame is skipped, red - current frame was
    already skipped or processed.

localized_output [optional, default: true]
    If true - gets decimal point, minus sign, CSV separator and realtime
    timestamp format from system locale. If false - uses locale indifferent
    values: "." for decimal point, "-" for minus sign, "," for CSV separator,
    "YYYY-MM-DD HH:MM:SS" for realtime timestamp. RtmSegmentDisplayOCR always
    uses locale indifferent values.

inverted [optional, default: false]
    If true - assumes that seven-segment display on the video has white digits
    on black background (instead of black digits on white background).

threshold [optional, default: "50"]
    Sets the threshold (in percents, can be real number) used to distinguish
    dark pixels from white pixels. By default threshold is adapted to actual
    luminance range of each frame. You can change this behaviour by adding
    modificator to the number (that's why threshold is a string):
        "a" - absolute threshold (won't be adapted to a frame)
        "i" - iterative threshold (threshold adapted to a frame using
              one-dimensional k-means clustering), best suited for blurry
              images
    For example, "39.5i" means 39.5% iterative threshold.

5. Use cases
------------

5.1 Getting started
-------------------
Start with this simple script:

	AviSource("input.avi")
	SegmentDisplayOCR()

Instead of AviSource you can use other apporpriate media input filter - check
out "Internal filters" article on AviSynth wiki. Now open this script in your
favorite video player and watch SegmentDisplayOCR debug output. This will help
you determine if video is recognized properly by SegmentDisplayOCR, so you can
tweak some parameters or add other filters before SegmentDisplayOCR to enhance
input video quality.

5.2 Processing whole video file
-------------------------------
After your video recognition script is set up and you are sure that recognition
process is working ok for your input video file, it's time to get log file with
recognition results. Basically, you need to do just two things to get the log:
turn on the logging for SegmentDisplayOCR filter and make AviSynth process
whole video.

First step is trivial (refer to Usage section). In the second step you should
in one way or another request all of the frames from your video so AviSynth
would pass each of them through SegmentDisplayOCR. This can be done by simply
playing AVS script in (almost) any video player or processing AVS script with
some kind of video editor or converter. The most suitable software I have found
for this purpose is avs2avi converter. This program is able to do no actual
converting and just cycle through all of the frames producing no output. Run it
like this with your script:

	avs2avi input.avs -c null -o n

5.3 Working with video camera
-----------------------------
You can actually recognize video directly from your camera in realtime using
AviSynth magick. To do this connect your camera to AviSynth using GRF file. GRF
file consists of DirectShow filter graph which can also include various video
sources such as your camera connected to PC.

GRF files are created in GraphEdit (part of Microsoft DirectShow SDK) and it's
various clones (e.g. GraphStudio). Open GraphEdit (or something similar), click
"Insert Filters" and under "WDM Streaming Capture Devices" category you should
find your camera - add it to the project. Now save it as GRF file.

Open resulting GRF file like this:

	DirectShowSource("camera.grf", audio=false, framecount=2147483647)
	SegmentDisplayOCR()
	
You should explicitly set video length via framecount parameter if using live
input. Framecount in this example is maximum possible framecount that AviSynth
supports. Though there is no guarantee that your player/editor/converter will
work with input video of such length.

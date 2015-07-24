#pragma once
#include "slag/slag_interface.h"

#include "opencv2/highgui/highgui.hpp"

class Frame : public slag::Message
{
public:
	Frame(void);
	virtual ~Frame(void);
public:
	cv::Mat image;
};

class __declspec(dllexport) VideoSource : public slag::Module
{
public:
	VideoSource(void);
	virtual ~VideoSource(void);

	bool Initialize(int settingsc, const char* settingsv[]);
	slag::Message** Compute(slag::Message* const * input, int inputPortNumber, int* outputPortNumber);

private:
	std::string text;
	cv::Mat picture;
	cv::VideoCapture* capture;
	Frame output;
	slag::Message* output_array;
};

#pragma once
#include "AbstractInterface.h"

#include "opencv2/highgui/highgui.hpp"

class Frame : public Message
{
public:
	Frame(void);
	virtual ~Frame(void);
public:
	cv::Mat image;
};

class VideoSource : public Module
{
public:
	VideoSource(void);
	virtual ~VideoSource(void);

	bool Initialize(int settingsc, const char* settingsv[]);
	Message** Compute(Message** input, int inputPortNumber, int* outputPortNumber);

private:
	std::string text;
	cv::Mat picture;
	cv::VideoCapture capture;
	std::vector<Message*> output_array;
};

#pragma once
#include "AbstractInterface.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

class Frame : public MyMessage
{
public:
	Frame(void);
	virtual ~Frame(void);
public:
	cv::Mat image;
};

class VideoSource : public MyModule
{
public:
	VideoSource(void);
	virtual ~VideoSource(void);

	MyMessage** Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber);

protected:
	virtual bool InitializeCallback(int settingsc, const char** settingsv);

private:
    size_t frameNum;
	cv::Mat picture;
	cv::VideoCapture capture;
	std::vector<MyMessage*> output_array;
};

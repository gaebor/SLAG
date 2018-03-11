#include "ImageProcessor.h"

#include "VideoSource.h"

#include <chrono>
#include <thread>
#include <math.h>

ImageProcessor::ImageProcessor()
    : lag(0.0)
{
}

ImageProcessor::~ImageProcessor()
{
}

MyMessage** ImageProcessor::Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber)
{
    *outputPortNumber = 0;
    fake_msg[0] = nullptr;

	if (inputPortNumber > 0)
	{
		auto input_msg = dynamic_cast<Frame*>((MyMessage*)(input[0]));
		if (input_msg)
		{
			img.create(input_msg->image.rows, input_msg->image.cols, CV_8UC4);
			cv::mixChannels(input_msg->image, img, {0, 0, 0, 1, 0, 2});

			*outputPicture = img.data;
			*outputPictureWidth = img.cols;
			*outputPictureHeight = img.rows;
			*strout = nullptr;
			*strout_length = 0;		

            std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1,1>>(lag));
		}
	}
    return (MyMessage**)fake_msg;
}

inline bool ImageProcessor::InitializeCallback(int settingsc, const char ** settingsv)
{
    if (settingsc > 0)
        lag = abs(atof(settingsv[0]));
    return true;
}

#include "VideoSource.h"

Frame::Frame(void)
{
}


Frame::~Frame(void)
{
}

VideoSource::VideoSource(void): capture(nullptr)
{
}


VideoSource::~VideoSource(void)
{
	delete capture;
}

bool VideoSource::Initialize( int settingsc, const char* settingsv[] )
{
	auto x = settingsv[0];
	for (int i = 0; i < settingsc; ++i)
	{
		if (0 == strcmp("-d", settingsv[i]))
		{
			if (settingsc > i+1)
			{
				capture = new cv::VideoCapture(atoi(settingsv[i+1]));
				if (capture->isOpened())
					return true;
			}else
				return false;
		}
		else if (0 == strcmp("-f", settingsv[i]))
		{
			if (settingsc > i+1)
			{
				capture = new cv::VideoCapture(settingsv[i+1]);
				if (capture->isOpened())
					return true;
			}else
				return false;
		}
	}
	return false;
}

slag::Message** VideoSource::Compute( slag::Message* const * input, int inputPortNumber, int* outputPortNumber )
{
	*capture >> output.image;

	outputPicture.width = output.image.cols;
	outputPicture.height = output.image.rows;
	outputPicture.imageInfo = output.image.data;

	output_array = &output;
	*outputPortNumber = 1;
	return &output_array;
}

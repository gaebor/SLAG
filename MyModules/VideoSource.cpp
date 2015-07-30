#include "VideoSource.h"

#include "opencv2/imgproc/imgproc.hpp"

Frame::Frame(void)
{
}


Frame::~Frame(void)
{
}

VideoSource::VideoSource(void)
{
}


VideoSource::~VideoSource(void)
{
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
				capture.open(atoi(settingsv[++i]));
				return capture.isOpened();
			}else
				return false;
		}
		else if (0 == strcmp("-f", settingsv[i]))
		{
			if (settingsc > i+1)
			{
				capture.open(std::string(settingsv[++i]));
				return capture.isOpened();
			}else
				return false;
		}
	}
	return false;
}

MyMessage** VideoSource::Compute( MyMessage** input, int inputPortNumber, int* outputPortNumber )
{
	auto output = new Frame();
	capture >> output->image;

	if (output->image.empty())
	{
		delete output;
		output_array.clear();
		*outputPortNumber =	0;
		return nullptr;
	}
	
	cv::cvtColor(output->image, picture, CV_BGR2RGBA);

	*outputPictureWidth = picture.cols;
	*outputPictureHeight = picture.rows;
	*outputPicture = picture.data;

	output_array.assign(1,output);
	*outputPortNumber = output_array.size();
	return output_array.data();
}

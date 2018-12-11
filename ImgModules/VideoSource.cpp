#include "VideoSource.h"

#include "opencv2/imgproc/imgproc.hpp"

Frame::Frame(void)
{
}


Frame::~Frame(void)
{
}

VideoSource::VideoSource(void) : frameNum(0)
{
}


VideoSource::~VideoSource(void)
{
}

bool VideoSource::InitializeCallback( int settingsc, const char** settingsv )
{
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

MyMessage** VideoSource::Compute( MyMessage**, int, int* outputPortNumber )
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
	//TODO color conversion should depend on MyModule::imageType
	cv::cvtColor(output->image, output->image, cv::COLOR_RGB2RGBA);

	picture = output->image;

	*outputPictureWidth = picture.cols;
	*outputPictureHeight = picture.rows;
	*outputPicture = picture.data;

    str = std::to_string(++frameNum);
    *strout = str.c_str();
    *strout_length = (int)str.size();

	output_array.assign(1,output);
	*outputPortNumber = (int)output_array.size();
	return output_array.data();
}

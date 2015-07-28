#include "VideoSource.h"

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

Message** VideoSource::Compute( Message** input, int inputPortNumber, int* outputPortNumber )
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
	*outputPictureWidth = output->image.cols;
	*outputPictureHeight = output->image.rows;
	*outputPicture = output->image.data;

	output_array.assign(1,output);
	*outputPortNumber = output_array.size();
	return output_array.data();
}

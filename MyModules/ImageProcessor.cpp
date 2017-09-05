#include "ImageProcessor.h"

#include "VideoSource.h"

ImageProcessor::ImageProcessor()
{
}

ImageProcessor::~ImageProcessor()
{
}

MyMessage** ImageProcessor::Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber)
{
	if (inputPortNumber == 0)
	{
		*outputPortNumber = 0;
		return nullptr;
	}
	else
	{
		auto input_msg = dynamic_cast<Frame*>((MyMessage*)(input[0]));
		if (input_msg)
		{
			img.create(input_msg->image.rows, input_msg->image.cols, CV_8UC4);
			cv::mixChannels(input_msg->image, img, {0, 0, 0, 1, 0, 2});

			*outputPicture = img.data;
			*outputPictureWidth = img.cols;
			*outputPictureHeight = img.rows;
			*outputText = nullptr;
			*outputTextLength = 0;

			*outputPortNumber = 0;
			return (MyMessage**)fake_msg;
		}else
			return nullptr;
	}
}

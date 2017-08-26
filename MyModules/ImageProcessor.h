#pragma once

#include "AbstractInterface.h"

#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"

class ImageProcessor : public MyModule
{
public:
	ImageProcessor();
	virtual ~ImageProcessor();

	MyMessage** Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber);
private:
	void* fake_msg[1];
	cv::Mat img;
};


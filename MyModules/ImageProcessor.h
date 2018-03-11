#pragma once

#include "AbstractInterface.h"

#include "opencv2/core/core.hpp"
#include "opencv2/core/mat.hpp"

class ImageProcessor : public MyModule
{
public:
	ImageProcessor(void);
	virtual ~ImageProcessor();

    //! always returns a valid 0 length output (no output, but always successfully)
	MyMessage** Compute(MyMessage** input, int inputPortNumber, int* outputPortNumber);
protected:
    bool InitializeCallback(int settingsc, const char** settingsv);
private:
    double lag;
	void* fake_msg[1];
	cv::Mat img;
};


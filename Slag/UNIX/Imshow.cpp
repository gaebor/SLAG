#include "../OS_dependent.h"

#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>
#include <memory>

#include "slag/slag_interface.h"

ImageType get_image_type(void)
{
	return ImageType::RGB;
}

void handle_output_image( const std::string& module_name_and_instance, int w, int h, ImageType type, const unsigned char* data )
{	
}

void terminate_output_image()
{
}

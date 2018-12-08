#include "Additionals.h"

#include "slag_interface.h"

ImageType get_image_type(void)
{
	return ImageType::RGB;
}

void handle_output_image( const std::string&, int w, int h, ImageType, const unsigned char*)
{	
}

void terminate_output_image()
{
}

void configure_output_image(const std::vector<std::string>&)
{
}

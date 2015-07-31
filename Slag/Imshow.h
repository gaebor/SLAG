#pragma once
#include "InternalTypes.h"

//!please guarantee that the @param imageContainer is protected against data race during the call
void Imshow(const char* window_name, const ImageContainer& imageContainer);

//!call this in a loop
void FeedImshow();
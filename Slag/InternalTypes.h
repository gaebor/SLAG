#pragma once
#include <memory>

#include <vector>

#include "AsyncQueue.h"
#include "slag/slag_interface.h"

inline size_t GetByteDepth(ImageType t)
{
	switch (t)
	{
	case RGB:
	case BGR:  return 3;
	case RGBA: return 4;
	case GREY:
	default:   return 4;
	}
}

typedef std::shared_ptr<void> ManagedMessage;
typedef std::shared_ptr<void> ManagedModule;

typedef AsyncQueue<ManagedMessage> MessageQueue;

struct ImageContainer
{
	ImageContainer();
	int w;
	int h;
	ImageType type;
	std::vector<unsigned char> data;
};

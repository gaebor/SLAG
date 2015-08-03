#pragma once
#include <memory>

#include "AsyncQueue.h"

typedef std::shared_ptr<void> ManagedMessage;
typedef std::shared_ptr<void> ManagedModule;

typedef AsyncQueue<ManagedMessage> MessageQueue;

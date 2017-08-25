#pragma once
#include <memory>

#include "aq/AsyncQueue.h"

typedef std::shared_ptr<void> ManagedMessage;
typedef std::shared_ptr<void> ManagedModule;

typedef aq::AsyncQueue<ManagedMessage> MessageQueue;

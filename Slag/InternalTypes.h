#pragma once

#include <memory>
#include <mutex>

#include "aq/AsyncQueue.h"

typedef std::shared_ptr<void> ManagedMessage;
typedef std::shared_ptr<void> ManagedModule;

typedef aq::AsyncQueue<ManagedMessage> MessageQueue;
typedef std::shared_ptr<MessageQueue> ManagedQueue;

typedef std::lock_guard<std::mutex> AutoLock;

#pragma once

#include "aq/AsyncQueue.h"
#include "InternalTypes.h"

typedef aq::AsyncQueue<ManagedMessage> MessageQueue;
typedef std::shared_ptr<MessageQueue> ManagedQueue;

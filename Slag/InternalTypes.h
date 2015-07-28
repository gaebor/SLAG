#include <memory>

#include "AsyncQueue.h"
#include "slag/slag_interface.h"

class ManagedModule : public std::shared_ptr<void>
{
public:
	ManagedModule(SlagDestroyModule_t deleter, void* module = nullptr);

	~ManagedModule();
};

class ManagedMessage : public std::shared_ptr<void>
{
public:
	ManagedMessage(SlagDestroyMessage_t deleter, void* message = nullptr);
	~ManagedMessage();
};

typedef AsyncQueue<ManagedMessage> MessageQueue;

#include <memory>

#include "AsyncQueue.h"
#include "slag/slag_interface.h"

class ManagedModule : public std::shared_ptr<void>
{
public:
	ManagedModule(slag::SlagDestroyModule deleter, void* module = nullptr)
		: std::shared_ptr<void>(module, deleter){}

	~ManagedModule(){}
};

class ManagedMessage : public std::shared_ptr<void>
{
public:
	ManagedMessage(slag::SlagDestroyMessage deleter, void* message = nullptr)
		: std::shared_ptr<void>(message, deleter){}
	~ManagedMessage(){}
};

typedef AsyncQueue<ManagedMessage> MessageQueue;
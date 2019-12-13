#pragma once

#include <memory>
#include <mutex>

#include "aq/AsyncQueue.h"

namespace slag {

    typedef std::shared_ptr<void> ManagedMessage;
    typedef std::shared_ptr<void> ManagedModule;

    typedef aq::AsyncQueue<ManagedMessage> MessageQueue;
    typedef std::shared_ptr<MessageQueue> ManagedQueue;

    template<bool enabled = true>
    struct AutoLock;

    template<>
    struct AutoLock<true> : std::lock_guard<std::mutex>
    {
        typedef std::lock_guard<std::mutex> MyType;
        using MyType::MyType;
    };

    template<>
    struct AutoLock<false>
    {
    public:
        AutoLock(std::mutex&){}
    };

}
#pragma once

#include "Poco/Event.h"
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"

template <class T>
class ExclusiveAccess
{
	typedef Poco::ScopedLock<Poco::Mutex> AutoLock;

public:
	template <class X>
	ExclusiveAccess(X x):_content(x), _editable(true)
	{
		_editable.set();
	}

	ExclusiveAccess():_content(), _editable(true)
	{
		_editable.set();
	}
	
	template<class Modifier>
	void Modify(Modifier modifier)
	{
		//this event is auto-reset,
		//so other threads have to wait for the end of this function, where it gets signaled again
		_editable.wait();
		modifier(_content); //edit
		_editable.set(); //becomes editable again
	}

	~ExclusiveAccess(){}

	//! lock it for a while
	void NonEditable()
	{
		_editable.wait(); //waits until it becomes editable and leaves it un-editable for a while
	}
	//! lets edit again
	/*!
		WARNING: don't make it editable, without being NonEditable first
		otherwise concurrent edits may happen
	*/
	void MakeEditable()
	{
		_editable.set();
	}
	//! un-managed access
	const T& Get()const{return _content;}
	//! un-managed access, use MakeEditable first
	T& Set(){return _content;}
private:
	Poco::Event _editable;
	T _content;
};

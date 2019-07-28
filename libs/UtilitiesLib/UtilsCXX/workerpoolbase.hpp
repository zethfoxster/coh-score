#ifndef _WORKERPOOLBASE_HPP
#define _WORKERPOOLBASE_HPP

#include "taskthread.hpp"

class ThreadedWorkerBase : public StepTaskThread {
public:
};

class ThreadedWorkerPoolBase {
public:
	void freeze() {}
	void thaw() {}
};

#endif

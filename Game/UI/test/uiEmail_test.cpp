#include "UnitTest++.h"

#if 0
struct ChatRecorder {
	ChatRecorder() {
		eaDestroyEx((EArrayHandle*)&sChatMessages, free);
	}

	virtual ~ChatRecorder() {
		eaDestroyEx((EArrayHandle*)&sChatMessages, free);
	}
};

TEST_FIXTURE(ChatRecorder, rejectUntradeableEnhancement) {
	
}
#endif

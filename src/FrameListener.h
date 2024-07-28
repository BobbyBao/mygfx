#pragma once

#include "misc/Callable.h"

namespace mygfx {

	class FrameChangeListener : public TCallable1<FrameChangeListener> {
	public:
		virtual void onFrameChange() {}
		virtual void onCall1() { onFrameChange(); }
	};

}

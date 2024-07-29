#include "DemoApp.h"

using namespace mygfx;

static std::vector<DemoDesc*> sDemos;

DemoDesc::DemoDesc(const char* name, const char* desc, const std::function<Demo* ()>& creator) {
	this->name = name;
	this->desc = desc;
	this->creator = creator;

	sDemos.push_back(this);
}

DemoApp::DemoApp() {
}

void DemoApp::onStart() {

}

void DemoApp::setDemo(Demo* demo) {
	if (demo == mActiveDemo) {
		return;
	}

	if (mActiveDemo) {
		mActiveDemo->stop();
	}

	mActiveDemo = demo;

	if (mActiveDemo) {
		mActiveDemo->start();
	}
}

void DemoApp::onGUI() {
	if (mActiveDemo) {
		mActiveDemo->gui();
	}
}

void DemoApp::onUpdate(double delta) {
	if (mActiveDemo) {
		mActiveDemo->update(delta);
	}
}

void DemoApp::onDraw(GraphicsApi& cmd) {
	if (mActiveDemo) {
		mActiveDemo->draw(cmd);
	}
}

void DemoApp::onDestroy() {
	if (mActiveDemo) {
		mActiveDemo->stop();
	}
}
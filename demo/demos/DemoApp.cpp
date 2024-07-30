#include "DemoApp.h"

using namespace mygfx::demo;

static std::vector<DemoDesc*> sDemos;

DemoDesc::DemoDesc(const char* name, const std::function<Demo* ()>& creator) {
	this->name = name;
	this->creator = creator;

	sDemos.push_back(this);
}

DemoApp::DemoApp() {
}

void DemoApp::onStart() {
	if (sDemos.size() > 0) {
		setDemo(0);
	}
}

void DemoApp::setDemo(int index) {
	if (index == mActiveDemoIndex) {
		return;
	}

	mActiveDemoIndex = index;
	auto demo = sDemos[mActiveDemoIndex]->creator();
	demo->mName = sDemos[mActiveDemoIndex]->name;
	setDemo(demo);
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
	
	ImGui::SetNextWindowPos({10.0f, 10.0f});
	ImGui::SetNextWindowSize({300.0f, 20.0f});
	ImGui::SetNextWindowBgAlpha(0.5f);

	if (ImGui::Begin("Demos", nullptr, ImGuiWindowFlags_NoDecoration)) {

		const char* preview_value = mActiveDemo ? mActiveDemo->mName : "";

		if (ImGui::BeginCombo("Active Demo", preview_value)) {
			for (int i = 0; i < sDemos.size(); i++) {
				auto desc = sDemos[i];
				if(ImGui::Selectable(desc->name)){
					setDemo(i);
				}
			}

			ImGui::EndCombo();
		}
	}

	ImGui::End();

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

	ShaderLibs::clean();
}
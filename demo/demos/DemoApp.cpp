#include "DemoApp.h"

using namespace mygfx;
using namespace mygfx::demo;

static std::vector<DemoDesc*> sDemos;

DemoDesc::DemoDesc(const char* name, const std::function<Demo* ()>& creator) {
	this->name = name;
	this->creator = creator;

	sDemos.push_back(this);
}

DemoApp::DemoApp(int argc, char** argv) : Application(argc, argv) {
}

Result<void> DemoApp::onStart() {
	if (sDemos.size() > 0) {
		setDemo(0);
	}

	co_return;
}

void DemoApp::setDemo(int index) {
	if (index == mActiveDemoIndex) {
		return;
	}

	mActiveDemoIndex = index;
	auto demo = sDemos[mActiveDemoIndex]->creator();
	demo->mName = sDemos[mActiveDemoIndex]->name;
	demo->mApp = this;
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
	ImGui::SetNextWindowSize({-1.0f, -1.0f});
	ImGui::SetNextWindowBgAlpha(0.75f);

	if (ImGui::Begin("Demos", nullptr, ImGuiWindowFlags_NoDecoration)) {
		ImGui::Text("CPU:	%s", mCPUName.c_str());
		ImGui::Text("GPU:	%s", gfxApi().getDeviceName());
		ImGui::Text("FPS:	%d", mLastFPS);
		ImGui::Text("DrawCall:%d", Stats::getDrawCall());
		

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

		if (mActiveDemo) {
			mActiveDemo->gui();
		}

		ImGui::End();


	}


}

void DemoApp::onUpdate(double delta) {
	if (mActiveDemo) {
		mActiveDemo->update(delta);
	}
}

void DemoApp::onPreDraw(GraphicsApi& cmd) {
	if (mActiveDemo) {
		mActiveDemo->preDraw(cmd);
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
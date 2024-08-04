#pragma once

#include "Application.h"
#include "ShaderLibs.h"
#include "core/MathTypes.h"

namespace mygfx::demo {

	using namespace math;
	
	class DemoApp;

	class Demo : public utils::RefCounted {
	public:
		virtual Result<void> start() {
			co_return;
		}
		virtual void gui() {}
		virtual void update(double delta) {}
		virtual void preDraw(GraphicsApi& cmd) {}
		virtual void draw(GraphicsApi& cmd) {}
		virtual void stop() {}
	protected:
		const char* mName = "";
		String mDesc;
		DemoApp* mApp = nullptr;

		friend class DemoApp;
	};

	struct DemoDesc {
		const char* name;
		std::function<Demo* ()> creator;
		DemoDesc(const char* name, const std::function<Demo* ()>& creator);
	};

#define DEF_DEMO(TYPE, NAME)\
	inline static DemoDesc s_##TYPE(NAME, []() { return new TYPE(); });

	class DemoApp : public Application {
	public:
		DemoApp(int argc = 0, char** argv = nullptr);

		void setDemo(int index);
		void setDemo(Demo* demo);
	protected:
		Result<void> onStart() override;
		void onDestroy() override;
		void onGUI() override;
		void onUpdate(double delta) override;
		void onPreDraw(GraphicsApi& cmd) override;
		void onDraw(GraphicsApi& cmd) override;

		int mActiveDemoIndex = -1;
		Ref<Demo> mActiveDemo;
	};

}
#pragma once

#include "GraphicsDefs.h"
#include "Application.h"

namespace mygfx::demo {

	class Demo : public utils::RefCounted {
	public:
		virtual void start() {}
		virtual void gui() {}
		virtual void update(double delta) {}
		virtual void draw(GraphicsApi& cmd) {}
		virtual void stop() {}
	protected:
		const char* mName = "";
		String mDesc;

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
		DemoApp();

		void setDemo(int index);
		void setDemo(Demo* demo);
	protected:
		void onStart() override;
		void onDestroy() override;
		void onGUI() override;
		void onUpdate(double delta) override;
		void onDraw(GraphicsApi& cmd) override;

		int mActiveDemoIndex = -1;
		Ref<Demo> mActiveDemo;
	};

}
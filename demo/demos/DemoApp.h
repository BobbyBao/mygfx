#pragma once

#include "GraphicsDefs.h"
#include "Application.h"

namespace mygfx {

	class Demo : public utils::RefCounted {
	public:
		virtual void start() {}
		virtual void gui() {}
		virtual void update(double delta) {}
		virtual void draw(GraphicsApi& cmd) {}
		virtual void stop() {}

	};

	struct DemoDesc {
		String name;
		String desc;
		std::function<Demo* ()> creator;

		DemoDesc(const char* name, const char* desc, const std::function<Demo* ()>& creator);
	};

	class DemoApp : public Application {
	public:
		DemoApp();

		void setDemo(Demo* demo);
	protected:
		void onStart() override;
		void onDestroy() override;
		void onGUI() override;
		void onUpdate(double delta) override;
		void onDraw(GraphicsApi& cmd) override;

		Ref<Demo> mActiveDemo;
	};

}
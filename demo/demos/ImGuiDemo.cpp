#include "DemoApp.h"

namespace mygfx::demo {
	

	class  ImGuiDemo : public Demo {
	public:		
		void gui() override {
			ImGui::ShowDemoWindow();
		}
	};

	DEF_DEMO(ImGuiDemo, "ImGui demo");	
}
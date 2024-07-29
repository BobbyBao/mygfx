#include "DemoApp.h"

namespace mygfx::demo {
	
	class  TriangleDemo : public Demo {
	public:

		void draw(GraphicsApi& cmd) override {

		}
	};

	DEF_DEMO(TriangleDemo, "Triangle demo");	
}
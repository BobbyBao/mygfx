#include "Light.h"

namespace mygfx {
	
	Light::Light() {
	}

	Object* Light::createObject() {
		return new Light();
	}

	void Light::cloneProcess(Object* destNode) {
		Light* light = (Light*)destNode;
		light->mColor = mColor;
		light->mIntensity = mIntensity;
		light->mType = mType;
		light->mRange = mRange;
		light->mSpotInnerConeAngle = mSpotInnerConeAngle;
		light->mSpotOuterConeAngle = mSpotOuterConeAngle;
	}
}
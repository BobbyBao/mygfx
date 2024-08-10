#pragma once

#include "Component.h"
#include "core/Maths.h"

namespace mygfx {
	
	enum class LightType {
		Directional,
		Point,
		Spot,
	};

	class Light : public Component {
	public:
		Light();

		PROPERTY_GET_SET_1(float3, Color)
		PROPERTY_GET_SET(float, Intensity)
		PROPERTY_GET_SET(LightType, Type)
		PROPERTY_GET_SET(float, Range)
		PROPERTY_GET_SET(float, SpotInnerConeAngle)
		PROPERTY_GET_SET(float, SpotOuterConeAngle)

	protected:
		Object* createObject() override;
		void cloneProcess(Object* destNode) override;

		float3 mColor {1.0f};
		float mIntensity {1.0f };
		LightType mType = LightType::Directional;
		float mRange = 10.0f;
		float mSpotInnerConeAngle;
		float mSpotOuterConeAngle;
	};

}
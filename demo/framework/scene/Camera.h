#pragma once

#include "Node.h"


namespace mygfx {

	class Camera : public Node {
	public:
		Camera();
		
		PROPERTY_GET_SET(float, Fov)
		PROPERTY_GET_SET(float, AspectRatio)

		PROPERTY_GET_SET(float, NearPlane)
		PROPERTY_GET_SET(float, FarPlane)

		PROPERTY_GET_SET_BOOL(bool, Ortho)
		PROPERTY_GET_SET_1(float2, OrthoSize)

		void setPerspective(float fov, float aspect, float znear, float zfar);

		mat4 getEffectiveWorldTransform() const;
				
		vec3 getDirection() const;
		vec3 getUp() const;
		vec3 getSide() const;

		const mat4& getViewMatrix() const;
		const mat4& getProjMatrix() const;
	protected:
		Node* createNode() override;
		void cloneProcess(Node* destNode) override;
		void onTransformChanged() override;
		void updateProjection() const;

		bool	mOrtho = false;
		float2	mOrthoSize {100.0f};
		float   mFov = pi<float>() / 4;
		float   mAspectRatio = 1.0f;
		float   mNearPlane = 0.1f;
		float   mFarPlane = 1000.0f;

		mutable mat4 mView;
		mutable mat4 mProjection;
	};

}
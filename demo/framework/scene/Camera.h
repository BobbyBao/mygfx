#pragma once

#include "Node.h"


namespace mygfx {

	class Camera : public Node {
	public:
		Camera();
		
		void setPerspective(float fov, float aspect, float znear, float zfar);

		mat4 getEffectiveWorldTransform() const;

		bool getNearPlane() const { return mNearPlane; }
		bool getFarPlane() const { return mFarPlane; }
		
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
		float	mOrthoSize = 20.0f;
		float   mFov = pi<float>() / 4;
		float   mAspectRatio = 1.0f;
		float   mNearPlane = 0.1f;
		float   mFarPlane = 1000.0f;

		mutable mat4 mView;
		mutable mat4 mProjection;
	};

}
#include "Camera.h"
#include "GraphicsConsts.h"

namespace mygfx {
	
    Camera::Camera() = default;

	Node* Camera::createNode() {
		return new Camera();
	}

	void Camera::cloneProcess(Node* destNode) {
        Camera* camera = (Camera*)destNode;
        camera->mOrtho = mOrtho;
        camera->mOrthoSize = mOrthoSize;
        camera->mFov = mFov;
		camera->mAspectRatio = mAspectRatio;
		camera->mNearPlane = mNearPlane;
		camera->mFarPlane = mFarPlane;
	}

	void Camera::setPerspective(float fov, float aspect, float znear, float zfar)
	{
		mFov = radians(fov);
        mAspectRatio = aspect;
		mNearPlane = znear;
		mFarPlane = zfar;
		mViewDirty = true;
	}

    mat4 Camera::getEffectiveWorldTransform() const
	{
        auto result =  glm::mat4_cast(getWorldRotation());
		result[3] = vec4(getWorldPosition(), 1.0f);
        return result;
    }
		
    void Camera::onTransformChanged() {
        mViewDirty = true;
    }

	vec3 Camera::getDirection() const
	{
        return ((mat3)getEffectiveWorldTransform()) * vec3(0.0f, 0.0f, -1.0f);
	}

    vec3 Camera::getUp() const
	{
		return ((mat3)getEffectiveWorldTransform()) * vec3(0.0f, 1.0f, 0.0f);
	}

    vec3 Camera::getSide() const
	{
		return ((mat3)getEffectiveWorldTransform()) * vec3(1.0f, 1.0f, 0.0f);
	}

    const mat4& Camera::getViewMatrix() const
	{
        if (mViewDirty) {
            mView = inverse(getEffectiveWorldTransform());
            mViewDirty = false;
        }

        return mView;
    } 

    const mat4& Camera::getProjMatrix() const
    {
        if (mProjDirty) {
            updateProjection();
        }

        return mProjection;
    }

    void Camera::updateProjection() const
    {
        if (mOrtho)
            mProjection = orthographic(-mOrthoSize.x, mOrthoSize.x, -mOrthoSize.y, mOrthoSize.y, mNearPlane, mFarPlane, InvertedDepth);
        else
            mProjection = perspective(mFov, mAspectRatio, mNearPlane, mFarPlane, InvertedDepth);
        
        mProjDirty = false;
    }
    
}
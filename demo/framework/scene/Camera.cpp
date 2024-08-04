#include "Camera.h"
#include "GraphicsConsts.h"

namespace mygfx {
	
    Camera::Camera() = default;
    
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
		return glm::translate(glm::mat4_cast(getWorldRotation()), getWorldPosition());
    }
		
    void Camera::onTransformChanged() {
        mViewDirty = true;
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
            mProjection = orthographic(-mOrthoSize, mOrthoSize, -mOrthoSize, mOrthoSize, mNearPlane, mFarPlane, InvertedDepth);
        else
            mProjection = perspective(mFov, mAspectRatio, mNearPlane, mFarPlane, InvertedDepth);
        
        mProjDirty = false;
    }
    
}
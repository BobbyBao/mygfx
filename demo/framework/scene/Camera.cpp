#include "Camera.h"
#include "GraphicsConsts.h"
#include "Node.h"

namespace mygfx {
	
    Camera::Camera() = default;

	Object* Camera::createObject() {
		return new Camera();
	}

	void Camera::cloneProcess(Object* destNode) {
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
        auto result =  glm::mat4_cast(mOwner->getWorldRotation());
		result[3] = vec4(mOwner->getWorldPosition(), 1.0f);
        return result;
    }
		
    void Camera::onTransformChanged() {
        mViewDirty = true;
    }

	vec3 Camera::getDirection() const
	{
        return mOwner->getDirection();
	}

    vec3 Camera::getUp() const
	{
        return mOwner->getUp();
	}

    vec3 Camera::getSide() const
	{
        return mOwner->getSide();
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
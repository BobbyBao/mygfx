#pragma once
#include "scene/Camera.h"

namespace mygfx {
enum class CameraMode {
    Orbit,
    FreeFlight
};

class CameraController : public Component {
public:
    CameraController();

    void setMode(CameraMode mode)
    {
        mCameraMode = mode;
    }

    void reset(const Vector3& eyePos, const Vector3& lookAt);

    float getYaw() const { return mYaw; }
    float getPitch() const { return mPitch; }
    float getDistance() const { return mDistance; }
    void setSpeed(float speed) { mSpeed = speed; }
    void setDistance(float dis) { mDistance = dis; }

    void lookAt(const Vector3& eyePos, const Vector3& lookAt);
    void lookAt(float yaw, float pitch, float distance, const Vector3& at);

    void update(float deltaTime);

private:
    void updateCameraPolar(float yaw, float pitch, float x, float y, float distance, float deltaTime = 0.0);
    void updateCameraWASD(float yaw, float pitch, float deltaTime);
    void onSetOwner(Node* node) override;

    CameraMode mCameraMode = CameraMode::Orbit;
    Vector3 mLastMoveDir;
    float mDistance;
    float mSpeed = 40.0f;
    float mYaw = 0.0f;
    float mPitch = 0.0f;
    Ref<Camera> mCamera = nullptr;
};

}
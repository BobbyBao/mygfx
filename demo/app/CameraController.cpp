#include "CameraController.h"
#include "ImGui.h"
#include "scene/Camera.h"
#include "scene/Node.h"

namespace mygfx {

static vec3 moveWASD()
{
    float scale = 1.0f;
    float x = 0, y = 0, z = 0;

    if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_W)) {
        z = -scale;
    }

    if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_S)) {
        z = scale;
    }

    if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_A)) {
        x = -scale;
    }

    if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_D)) {
        x = scale;
    }

    if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_E)) {
        y = scale;
    }

    if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_Q)) {
        y = -scale;
    }

    float len = std::max(1.0e-3f, sqrtf(x * x + y * y + z * z));
    return vec3(x / len, y / len, z / len);
}

static Vector3 polarToVector(float yaw, float pitch)
{
    return Vector3(sinf(yaw) * cosf(pitch), sinf(pitch), cosf(yaw) * cosf(pitch));
}

CameraController::CameraController()
    : mCamera(nullptr)
{
    mDistance = -1;
    mLastMoveDir = vec3(0, 0, 0);
}

void CameraController::onSetOwner(Node* node)
{
    mCamera = node->getComponent<Camera>();
}

void CameraController::reset(const vec3& eyePos, const vec3& lookAt)
{
    this->lookAt(eyePos, lookAt);
}

void CameraController::lookAt(const vec3& eyePos, const vec3& lookAt)
{
    mCamera->getOwner()->lookAt(eyePos, lookAt);
    auto& v = mCamera->getViewMatrix();

    mDistance = length(lookAt - eyePos);
    vec3 zBasis = row(v, 2);
    mYaw = atan2(zBasis.x, zBasis.z);
    float fLen = sqrt(zBasis.z * zBasis.z + zBasis.x * zBasis.x);
    mPitch = atan2(zBasis.y, fLen);
}

void CameraController::lookAt(float yaw, float pitch, float distance, const vec3& at)
{
    lookAt(at + polarToVector(yaw, pitch) * distance, at);
}

void CameraController::updateCameraWASD(float yaw, float pitch, float deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();

    auto& v = mCamera->getViewMatrix();
    auto movedir = moveWASD();
    float speed = mSpeed;
    // Smooth movement
    if (std::abs(movedir.x) > 0.1f || std::abs(movedir.y) > 0.1f || std::abs(movedir.z) > 0.1f) {
        if (ImGui::IsKeyDown(ImGuiKey::ImGuiMod_Shift))
            speed = speed + (5.0f - speed) * (1.0f - std::exp(-(float)deltaTime * 1.0f));
        else
            speed = speed + (2.0f - speed) * (1.0f - std::exp(-(float)deltaTime * 3.0f));

    } else {
        speed = speed + (0.0f - speed) * (1.0f - std::exp(-(float)deltaTime * 8.0f));
    }

    mLastMoveDir = mLastMoveDir + (movedir - mLastMoveDir) * (1.0f - std::exp(-(float)deltaTime * 3.0f));

    auto node = mCamera->getOwner();

    node->translate(mLastMoveDir * speed * (float)deltaTime);

    vec3 dir = polarToVector((float)yaw, (float)pitch) * mDistance;

    lookAt(node->getPosition(), node->getPosition() - dir);

    if (io.MouseDown[2]) {
        node->translate(10 * (float)deltaTime * (RIGHT * -io.MouseDelta.x + UP * io.MouseDelta.y));
    }

    if (io.MouseWheel != 0.0f) {
        node->translate(FORWARD * io.MouseWheel);
    }
}

inline vec3 transform(const mat4& m, const vec3& v)
{
    return (m * vec4(v, 1));
}

void CameraController::updateCameraPolar(float yaw, float pitch, float x, float y, float distance, float deltaTime)
{
    pitch = std::max(-MATH_HALF_PI + 1e-2f, std::min(pitch, MATH_HALF_PI - 1e-2f));

    auto& v = mCamera->getViewMatrix();

    auto move = mCamera->getSide() * x * distance / 10.0f;
    move += mCamera->getUp() * y * distance / 10.0f;
    move += transform(transpose(v), (moveWASD() * mSpeed * (float)deltaTime));

    auto node = mCamera->getOwner();
    // Trucks camera, moves the camera parallel to the view plane.
    node->setPosition(node->getPosition() + move);

    // Orbits camera, rotates a camera about the target
    vec3 dir = mCamera->getDirection();
    vec3 pol = polarToVector(yaw, pitch);
    vec3 at = node->getPosition() + (dir * mDistance);

    lookAt(at + (pol * distance), at);
}

void CameraController::update(float deltaTime)
{

    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse) {
        io.MouseDelta.x = 0;
        io.MouseDelta.y = 0;
        io.MouseWheel = 0;
    }

    auto yaw = mYaw;
    auto pitch = mPitch;
    auto distance = mDistance;

    if (mCameraMode == CameraMode::Orbit) {

        if (!io.MouseWheel && (!io.MouseDown[0] || (!io.MouseDelta.x && !io.MouseDelta.y)))
            return;

        if ((io.KeyCtrl == false) && (io.MouseDown[0] == true)) {
            yaw -= io.MouseDelta.x / 100.f;
            pitch += io.MouseDelta.y / 100.f;
        }

        //  Orbiting
        distance -= (float)io.MouseWheel / 3.0f;
        distance = std::max(distance, 0.1f);

        bool panning = (io.KeyCtrl == true) && (io.MouseDown[0] == true);

        updateCameraPolar(yaw, pitch,
            panning ? -io.MouseDelta.x / 100.0f : 0.0f,
            panning ? io.MouseDelta.y / 100.0f : 0.0f,
            distance, io.DeltaTime);
    } else if (mCameraMode == CameraMode::FreeFlight) {

        if ((io.KeyCtrl == false) && (io.MouseDown[1] == true)) {
            yaw -= io.MouseDelta.x / 100.f;
            pitch += io.MouseDelta.y / 100.f;
        }

        //  WASD
        updateCameraWASD(yaw, pitch, io.DeltaTime);
    }
}
}

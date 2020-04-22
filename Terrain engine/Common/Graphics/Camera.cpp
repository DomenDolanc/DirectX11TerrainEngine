#include "pch.h"
#include "Camera.h"
#include <stdint.h>
#include <algorithm>

using namespace Terrain_engine;

using namespace DirectX;
using namespace Windows::Foundation;


const float Camera::INITIAL_TRAVEL_SPEED = 1.0f;

void Camera::MoveForward()
{
    Translate({ 0.0f, 0.0f, -m_TravelSpeed });
    UpdateSpeed();
}

void Camera::MoveBackward()
{
    Translate({ 0.0f, 0.0f, m_TravelSpeed });
    UpdateSpeed();
}

void Camera::MoveLeft()
{
    Translate({ -m_TravelSpeed, 0.0f, 0.0f });
    UpdateSpeed();
}

void Camera::MoveRight()
{
    Translate({ m_TravelSpeed, 0.0f, 0.0f });
    UpdateSpeed();
}

void Terrain_engine::Camera::Translate(DirectX::XMFLOAT3 translation)
{
    XMStoreFloat3(&translation, XMVector3Transform(
        XMLoadFloat3(&translation),
        XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f) * XMMatrixScaling(m_TravelSpeed, m_TravelSpeed, m_TravelSpeed)
    ));

    m_position = { m_position.x + translation.x, m_position.y + translation.y, m_position.z + translation.z };
}

void Camera::UpdateSpeed()
{
    const float MAX_TRAVEL_SPEED = 5.0f;
    if (m_TravelSpeed > MAX_TRAVEL_SPEED)
        return;

    m_TravelSpeed += m_Acceleration;
}

void Terrain_engine::Camera::Stop()
{
    m_TravelSpeed = 0.0;
    m_position = { 0.0, 0.0, 0.0 };
}

void Terrain_engine::Camera::setYaw(float yaw)
{
    m_Yaw = yaw;
}

double Terrain_engine::Camera::getYaw()
{
    return m_Yaw;
}

void Terrain_engine::Camera::setPitch(float pitch)
{
    m_Pitch = pitch;
}

double Terrain_engine::Camera::getPitch()
{
    return m_Pitch;
}

DirectX::XMMATRIX Terrain_engine::Camera::GetMatrix()
{
    const XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
    const auto lookVector = XMVector3Transform(forwardBaseVector, XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0));
    const auto camPositon = m_Eye + XMLoadFloat3(&m_position);
    const auto camTarget = camPositon + lookVector;
    
    XMStoreFloat3(&m_CurrentPositon, camPositon);
    return XMMatrixLookAtRH(camPositon, camTarget, m_Up);
}

DirectX::XMFLOAT3 Terrain_engine::Camera::getEye()
{
    return m_CurrentPositon;
}

void Terrain_engine::Camera::setEye(DirectX::XMFLOAT3 position)
{
    m_Eye = XMLoadFloat3(&position);
}

void StopCameraMovement()
{
}

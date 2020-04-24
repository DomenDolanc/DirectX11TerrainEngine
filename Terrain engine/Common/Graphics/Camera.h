#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\DirectXMesh.h"

namespace Terrain_engine
{
    class Camera
    {
    public:
        void MoveForward();
        void MoveBackward();
        void MoveLeft();
        void MoveRight();

        void Translate(DirectX::XMFLOAT3 translation);

        void UpdateSpeed();
        void Stop();

        void setYaw(float yaw);
        double getYaw();

        void setPitch(float pitch);
        double getPitch();

        DirectX::XMFLOAT3 getEye();

        DirectX::XMMATRIX GetMatrix();
        DirectX::XMMATRIX GetReflectionMatrix();

        static const float INITIAL_TRAVEL_SPEED;
    private:

        DirectX::XMVECTOR m_Eye = { 0.0f, 5000.0f, 0.0f, 0.0f };
        DirectX::XMVECTOR m_At = { 0.0f, 0.0f, 0.0f, 0.0f };
        DirectX::XMVECTOR m_Up = { 0.0f, 1.0f, 0.0f, 0.0f };

        DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 m_CurrentPositon = { 0.0f, 0.0f, 0.0f };
        float m_Pitch;
        float m_Yaw;
        float m_TravelSpeed = INITIAL_TRAVEL_SPEED;
        const float m_Acceleration = 0.5f;
    };
}



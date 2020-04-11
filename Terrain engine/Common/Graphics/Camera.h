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

        void setYaw(double yaw);
        double getYaw();

        void setPitch(double pitch);
        double getPitch();

        DirectX::XMMATRIX GetMatrix();
        DirectX::XMFLOAT3 GetEye();

        static const float INITIAL_TRAVEL_SPEED;
    private:

        DirectX::XMVECTOR m_Eye = { 0.0f, 5000.0f, 0.0f, 0.0f };
        DirectX::XMVECTOR m_At = { 0.0f, 0.0f, 0.0f, 0.0f };
        DirectX::XMVECTOR m_Up = { 0.0f, 1.0f, 0.0f, 0.0f };

        DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 m_CurrentPositon = { 0.0f, 0.0f, 0.0f };
        double m_Pitch;
        double m_Yaw;
        float m_TravelSpeed = INITIAL_TRAVEL_SPEED;
        const float m_Acceleration = 0.5f;
    };
}



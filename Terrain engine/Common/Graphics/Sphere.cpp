#include "pch.h"
#include "Sphere.h"
#include <stdint.h>
#include <algorithm>
#include <math.h>

using namespace Terrain_engine;

using namespace DirectX;
using namespace Windows::Foundation;

Sphere::Sphere(std::shared_ptr<DX::DeviceResources> deviceResources)
{
    m_deviceResources = deviceResources;
    m_scaling = 100;
}

Sphere::Sphere(std::shared_ptr<DX::DeviceResources> deviceResources, DirectX::XMFLOAT3 pos, int radius)
{
    m_deviceResources = deviceResources;

    m_scaling = 100;
    m_position = pos;
    m_radius = radius;
}

Sphere::~Sphere()
{
    m_vertices.clear();
    m_indices.clear();
}

void Sphere::CreateVertices()
{
    if (!m_loadingComplete)
        return;

    m_loadingComplete = false;

    m_vertices.clear();
    m_vertexBuffer.Reset();

    int vertexCount = 0;
    float halfScaling = m_scaling / 2.0f;

    VertexPosition vertex;
    vertex.pos = { 0, (float)m_radius, 0 };

    m_vertices.emplace_back(vertex);

    vertexCount++;

    float phiStep = XM_PI / m_stackCount;
    float thetaStep = 2.0f * XM_PI / m_sliceCount;

    for (int i = 1; i <= m_stackCount - 1; i++) {
        float phi = i * phiStep;
        for (int j = 0; j <= m_sliceCount; j++) {
            float theta = j * thetaStep;
            vertex.pos = {
                ((float)m_radius * sin(phi) * cos(theta)),
                ((float)m_radius * cos(phi)),
                ((float)m_radius * sin(phi) * sin(theta))
            };
            m_vertices.emplace_back(vertex);
            vertexCount++;
        }
    }
    vertex.pos = { 0, -(float)m_radius, 0 };

    m_vertices.emplace_back(vertex);
    m_verticesCount = m_vertices.size();

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = &m_vertices.front();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(m_verticesCount * sizeof(VertexPosition), D3D11_BIND_VERTEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));
    m_loadingComplete = true;
}

void Sphere::CreateIndices()
{
    m_indices.clear();

    int index = 0;
    for (int i = 1; i <= m_sliceCount; i++) {
        m_indices.emplace_back(0);
        m_indices.emplace_back(i + 1);
        m_indices.emplace_back(i);
    }
    
    int baseIndex = 1;
    int ringVertexCount = m_sliceCount + 1;
    for (int i = 0; i < m_stackCount - 2; i++) 
    {
        for (int j = 0; j < m_sliceCount; j++)
        {
            m_indices.emplace_back(baseIndex + i * ringVertexCount + j);
            m_indices.emplace_back(baseIndex + i * ringVertexCount + j + 1);
            m_indices.emplace_back(baseIndex + (i + 1) * ringVertexCount + j);

            m_indices.emplace_back(baseIndex + (i + 1) * ringVertexCount + j);
            m_indices.emplace_back(baseIndex + i * ringVertexCount + j + 1);
            m_indices.emplace_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }
    int southPoleIndex = index - 1;
    baseIndex = southPoleIndex - ringVertexCount;
    for (int i = 0; i < m_sliceCount; i++) 
    {
        m_indices.emplace_back(southPoleIndex);
        m_indices.emplace_back(baseIndex + i);
        m_indices.emplace_back(baseIndex + i + 1);
    }

    m_indexCount = m_indices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_indices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_indices.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
}

void Sphere::setScaling(float scaling)
{
    m_scaling = scaling;
}

void Sphere::ResetBuffers()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
}

void Sphere::Draw()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    UINT stride = sizeof(VertexPosition);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->DrawIndexed(m_indexCount, 0, 0);
}

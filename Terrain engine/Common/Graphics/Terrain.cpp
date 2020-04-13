#include "pch.h"
#include "Terrain.h"
#include <stdint.h>
#include <algorithm>

using namespace Terrain_engine;

using namespace DirectX;
using namespace Windows::Foundation;

Terrain::Terrain(std::shared_ptr<DX::DeviceResources> deviceResources)
{
    m_deviceResources = deviceResources;
    m_scaling = 100;
}

Terrain::Terrain(std::shared_ptr<DX::DeviceResources> deviceResources, int columns, int rows)
{
    m_deviceResources = deviceResources;

    m_scaling = 100;
    m_Columns = columns;
    m_Rows = rows;
}

Terrain::~Terrain()
{
    m_vertices.clear();
    m_indices.clear();
}

void Terrain::CreateVertices()
{
    if (!m_loadingComplete)
        return;

    m_loadingComplete = false;
    m_vertices.clear();
    m_vertexBuffer.Reset();

    XMFLOAT3 gridColor{ 1.0f, 1.0f, 1.0f };

    m_verticesCount = m_Columns * m_Rows;

    float halfScaling = m_scaling / 2.0f;
    const float startX = -halfScaling;
    const float endX = halfScaling;

    const float startZ = -halfScaling;
    const float endZ = halfScaling;

    const float stepX = (endX - startX) / (m_Columns - 1);
    const float stepZ = (endZ - startZ) / (m_Rows - 1);

    float tempX = startX;

    for (size_t i = 0; i < m_Columns; i++)
    {
        float tempZ = startZ;
        for (size_t j = 0; j < m_Rows; j++)
        {
            VertexPositionColor vertex;
            vertex.pos = XMFLOAT3(tempX, 0.0, tempZ);
            vertex.normal = XMFLOAT3(0.0, 1.0, 0.0);
            vertex.color = XMFLOAT3(1.0, 1.0, 1.0);
            m_vertices.emplace_back(vertex);
            tempZ += stepZ;
        }
        tempX += stepX;
    }

    m_verticesCount = m_vertices.size();

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = &m_vertices.front();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(m_verticesCount * sizeof(VertexPositionColor), D3D11_BIND_VERTEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));
    m_loadingComplete = true;
}

void Terrain::CreateIndices()
{
    m_indexBuffer.Reset();
    m_indices.clear();
    for (size_t i = 0; i < m_Columns - 1; i++)
    {
        for (size_t j = 0; j < m_Rows - 1; j++)
        {
            m_indices.emplace_back(i * m_Rows + j);             // 0
            m_indices.emplace_back((i + 1) * m_Rows + j);       // 2
            m_indices.emplace_back(i * m_Rows + j + 1);         // 1
            m_indices.emplace_back((i + 1) * m_Rows + j);       // 2
            m_indices.emplace_back((i + 1) * m_Rows + j + 1);   // 3
            m_indices.emplace_back(i * m_Rows + j + 1);         // 1
        }
        m_indices.emplace_back(-1);
    }

    m_indexCount = m_indices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_indices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_indexCount * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
}

void Terrain::CreateQuadIndices()
{
    m_indexBuffer.Reset();
    m_indices.clear();
    for (size_t i = 0; i < m_Columns - 1; i++)
    {
        for (size_t j = 0; j < m_Rows - 1; j++)
        {
            m_indices.emplace_back(i * m_Rows + j);             // 0
            m_indices.emplace_back((i + 1) * m_Rows + j);       // 2
            m_indices.emplace_back(i * m_Rows + j + 1);         // 1
            m_indices.emplace_back((i + 1) * m_Rows + j + 1);   // 3
        }
        m_indices.emplace_back(-1);
    }

    m_indexCount = m_indices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_indices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_indexCount * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
}

size_t Terrain::getVerticesCount()
{
    return m_verticesCount;
}

size_t Terrain::getIndicesCount()
{
    return m_indexCount;
}

void Terrain::setScaling(double scaling)
{
    m_scaling = scaling;
}

void Terrain::ResetBuffers()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
}

void Terrain::Draw()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->DrawIndexed(m_indexCount, 0, 0);
}
#include "pch.h"
#include "Skybox.h"
#include <stdint.h>
#include <algorithm>
#include <math.h>
#include "Common/DirectXHelpers/BasicLoader.h"

using namespace Terrain_engine;

using namespace DirectX;
using namespace Windows::Foundation;

Skybox::Skybox(std::shared_ptr<DX::DeviceResources> deviceResources)
{
    m_deviceResources = deviceResources;

    auto device = m_deviceResources->GetD3DDevice();
    auto wicFactory = m_deviceResources->GetWicImagingFactory();
    auto tempFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

    BasicLoader^ basicLoader = ref new BasicLoader(device, wicFactory);

    Platform::String^ fileName = tempFolder->Path + "\\Assets\\Textures\\skybox.dds";
    basicLoader->LoadTexture(fileName, &m_d3dSkyboxTexture, &m_d3dSkyboxTextureShaderView);
}

Skybox::~Skybox()
{
    m_vertices.clear();
    m_indices.clear();
    m_d3dSkyboxTexture->Release();
    m_d3dSkyboxTextureShaderView->Release();
}

void Skybox::CreateVertices()
{
    if (!m_loadingComplete)
        return;

    m_loadingComplete = false;

    m_vertices.clear();
    m_vertexBuffer.Reset();

    float halfScaling = m_scaling / 4.0f;

    VertexPositionColor vertex;
    XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };

    vertex = { XMFLOAT3(-halfScaling, -halfScaling, -halfScaling), XMFLOAT3(0.0f, 0.0f, 0.0f), normal };
    m_vertices.emplace_back(vertex);
    vertex = { XMFLOAT3(-halfScaling, -halfScaling, halfScaling), XMFLOAT3(0.0f, 0.0f, 1.0f), normal };
    m_vertices.emplace_back(vertex);
    vertex = { XMFLOAT3(-halfScaling, halfScaling, -halfScaling), XMFLOAT3(0.0f, 1.0f, 0.0f), normal };
    m_vertices.emplace_back(vertex); 
    vertex = { XMFLOAT3(-halfScaling, halfScaling, halfScaling), XMFLOAT3(0.0f, 1.0f, 1.0f), normal };
    m_vertices.emplace_back(vertex);
    vertex = { XMFLOAT3(halfScaling, -halfScaling, -halfScaling), XMFLOAT3(1.0f, 0.0f, 0.0f), normal };
    m_vertices.emplace_back(vertex);
    vertex = { XMFLOAT3(halfScaling, -halfScaling, halfScaling), XMFLOAT3(1.0f, 0.0f, 1.0f), normal };
    m_vertices.emplace_back(vertex);
    vertex = { XMFLOAT3(halfScaling, halfScaling, -halfScaling), XMFLOAT3(1.0f, 1.0f, 0.0f), normal };
    m_vertices.emplace_back(vertex);
    vertex = { XMFLOAT3(halfScaling, halfScaling, halfScaling), XMFLOAT3(1.0f, 1.0f, 1.0f), normal };
    m_vertices.emplace_back(vertex);

    m_verticesCount = m_vertices.size();

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = &m_vertices.front();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(m_verticesCount * sizeof(VertexPositionColor), D3D11_BIND_VERTEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));
    m_loadingComplete = true;
}

void Skybox::CreateIndices()
{
    m_indices.clear();

    static const uint32_t cubeIndices[] =
    {
        0,2,1, // -x
        1,2,3,

        4,5,6, // +x
        5,7,6,

        0,1,5, // -y
        0,5,4,

        2,6,7, // +y
        2,7,3,

        0,4,6, // -z
        0,6,2,

        1,3,7, // +z
        1,7,5,
    };

    m_indices = std::vector<uint32_t>({
        0,2,1, // -x
        1,2,3,

        4,5,6, // +x
        5,7,6,

        0,1,5, // -y
        0,5,4,

        2,6,7, // +y
        2,7,3,

        0,4,6, // -z
        0,6,2,

        1,3,7, // +z
        1,7,5,
        }); 

    m_indexCount = m_indices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_indices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_indices.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
}

void Skybox::setScaling(float scaling)
{
    m_scaling = scaling;
}

void Skybox::ResetBuffers()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
}

void Skybox::Draw()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->DrawIndexed(m_indexCount, 0, 0);
}

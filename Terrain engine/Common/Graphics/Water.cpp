#include "pch.h"
#include "Water.h"
#include <stdint.h>
#include <algorithm>
#include <math.h>
#include "Common/DirectXHelpers/BasicLoader.h"

using namespace Terrain_engine;

using namespace DirectX;
using namespace Windows::Foundation;

Water::Water(std::shared_ptr<DX::DeviceResources> deviceResources)
{
    m_deviceResources = deviceResources;

    auto device = m_deviceResources->GetD3DDevice();
    auto wicFactory = m_deviceResources->GetWicImagingFactory();

    auto tempFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

    BasicLoader^ basicLoader = ref new BasicLoader(device, wicFactory);

    Platform::String^ fileName = tempFolder->Path + "\\Assets\\Textures\\waterDuDV.png";
    basicLoader->LoadTexture(fileName, &m_d3dDUDVTexture, &m_d3dDUDVTextureShaderView);
}

Water::~Water()
{
    m_vertices.clear();
    m_indices.clear();
    m_d3dReflectionTexture.Reset();
    m_d3dRefractionTexture.Reset();
    m_d3dReflectionTextureShaderView.Reset();
    m_d3dRefractionTextureShaderView.Reset();
    m_d3dReflectionRenderTarget.Reset();
    m_d3dRefractionRenderTarget.Reset();
}

void Terrain_engine::Water::CreateReflectionTexture(Windows::Foundation::Size textureSize)
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Width = textureSize.Width;
    textureDesc.Height = textureSize.Height;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;

    DX::ThrowIfFailed(device->CreateTexture2D(&textureDesc, NULL, &m_d3dReflectionTexture));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    DX::ThrowIfFailed(device->CreateShaderResourceView(m_d3dReflectionTexture.Get(), &srvDesc, &m_d3dReflectionTextureShaderView));

    DX::ThrowIfFailed(device->CreateRenderTargetView1(m_d3dReflectionTexture.Get(), nullptr, &m_d3dReflectionRenderTarget));

    CD3D11_TEXTURE2D_DESC1 depthStencilDesc(DXGI_FORMAT_D32_FLOAT,
        lround(textureSize.Width),
        lround(textureSize.Height),
        1, 1, D3D11_BIND_DEPTH_STENCIL
    );

    Microsoft::WRL::ComPtr<ID3D11Texture2D1> depthStencil;
    DX::ThrowIfFailed(device->CreateTexture2D1(&depthStencilDesc, nullptr, &depthStencil));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &m_d3dReflectionDepthStencilView));
}

void Terrain_engine::Water::CreateRefrationTexture(Windows::Foundation::Size textureSize)
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Width = textureSize.Width;
    textureDesc.Height = textureSize.Height;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;

    DX::ThrowIfFailed(device->CreateTexture2D(&textureDesc, NULL, &m_d3dRefractionTexture));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    DX::ThrowIfFailed(device->CreateShaderResourceView(m_d3dRefractionTexture.Get(), &srvDesc, &m_d3dRefractionTextureShaderView));

    DX::ThrowIfFailed(device->CreateRenderTargetView1(m_d3dRefractionTexture.Get(), nullptr, &m_d3dRefractionRenderTarget));

    CD3D11_TEXTURE2D_DESC1 depthStencilDesc(DXGI_FORMAT_D32_FLOAT,
        lround(textureSize.Width),
        lround(textureSize.Height),
        1, 1, D3D11_BIND_DEPTH_STENCIL
    );

    Microsoft::WRL::ComPtr<ID3D11Texture2D1> depthStencil;
    DX::ThrowIfFailed(device->CreateTexture2D1(&depthStencilDesc, nullptr, &depthStencil));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &m_d3dRefractionDepthStencilView));
}

void Water::CreateVertices()
{
    if (!m_loadingComplete)
        return;

    m_loadingComplete = false;

    m_vertices.clear();
    m_vertexBuffer.Reset();

    int vertexCount = 0;
    float halfScaling = m_scaling / 2.0f;
    float heightPos = 10.0f;

    VertexPositionColor vertex;
    XMVECTOR normal;

    vertex.color = { 0.0, 0.0, 1.0 };
    vertex.normal = { 0.0, 1.0, 0.0 };

    vertex.pos = { -halfScaling, heightPos, -halfScaling };
    m_vertices.emplace_back(vertex);

    vertex.pos = { halfScaling, heightPos, -halfScaling };
    m_vertices.emplace_back(vertex);

    vertex.pos = { -halfScaling, heightPos, halfScaling };
    m_vertices.emplace_back(vertex);

    vertex.pos = { halfScaling, heightPos, halfScaling };
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

void Water::CreateIndices()
{
    m_indices.clear();

    m_indices.emplace_back(0);
    m_indices.emplace_back(1);
    m_indices.emplace_back(2);
    m_indices.emplace_back(1);
    m_indices.emplace_back(3);
    m_indices.emplace_back(2);

    m_indexCount = m_indices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_indices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_indices.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
}

void Terrain_engine::Water::UpdateTextureSize(Windows::Foundation::Size textureSize)
{
    CreateReflectionTexture(textureSize);
    CreateReflectionTexture(textureSize);
}

void Water::ResetBuffers()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
}

void Terrain_engine::Water::setScaling(float scaling)
{
    m_scaling = scaling;
}

void Water::Draw()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->DrawIndexed(m_indexCount, 0, 0);
}

#include "pch.h"
#include "..\Common\DirectXHelpers\OBJ_Loader.h"
#include "Common/DirectXHelpers/BasicLoader.h"
#include "Jet.h"

using namespace DirectX;

Terrain_engine::Jet::Jet(std::shared_ptr<DX::DeviceResources> deviceResources)
{
    m_deviceResources = deviceResources;
    m_objLoader = std::make_shared<Terrain_engine::Loader>();
}

Terrain_engine::Jet::~Jet()
{
}

void Terrain_engine::Jet::LoadObject()
{
    auto tempFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
    Platform::String^ fileName = tempFolder->Path + "\\Assets\\Objects\\";
    std::wstring filenameWStr(fileName->Begin());
    std::string filenameStr(filenameWStr.begin(), filenameWStr.end());

    bool loadout = m_objLoader->LoadFile(filenameStr + "F+15.obj", filenameStr);

    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto wicFactory = m_deviceResources->GetWicImagingFactory();

    BasicLoader^ basicLoader = ref new BasicLoader(device, context, wicFactory);

    fileName = tempFolder->Path + "\\Assets\\Objects\\F-15Cmetal.jpg";
    basicLoader->LoadTexture(fileName, &m_d3dTexture, &m_d3dTextureShaderView, false);

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = &m_objLoader->LoadedVertices.front();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(m_objLoader->LoadedVertices.size() * sizeof(VertexPosition), D3D11_BIND_VERTEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_objLoader->LoadedIndices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_objLoader->LoadedIndices.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
}

void Terrain_engine::Jet::Draw()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    ID3D11SamplerState* const sampler[1] = { m_deviceResources->GetSampler() };
    auto dirtTextureShaderResouce = GetTextureShaderResourceView();

    UINT stride = sizeof(VertexPosition);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->PSSetSamplers(0, 1, sampler);
    context->PSSetShaderResources(0, 1, &dirtTextureShaderResouce);

    int indexOffset = 0;
    for (int i = 0; i < m_objLoader->LoadedMeshes.size(); i++)
    {
        context->DrawIndexed(m_objLoader->LoadedMeshes[i].Indices.size(), indexOffset, 0);
        indexOffset += m_objLoader->LoadedMeshes[i].Indices.size();
    }
}

void Terrain_engine::Jet::ResetBuffers()
{
    m_vertexBuffer->Release();
    m_indexBuffer->Release();
}

DirectX::XMMATRIX Terrain_engine::Jet::GetTransformation() const
{
    return XMMatrixRotationY(XM_PI) * XMMatrixTranslation(0.0, -300.0, -1300.0f);
}

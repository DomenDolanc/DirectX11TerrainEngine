#include "pch.h"
#include "SceneRenderer.h"
#include <stdint.h>
#include <algorithm>

using namespace Terrain_engine;

using namespace DirectX;
using namespace Windows::Foundation;

SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_loadingComplete(false),
    m_degreesPerSecond(45),
    m_tracking(false),
    m_deviceResources(deviceResources)
{
    m_Camera = std::make_shared<Camera>();
    m_Terrain = std::make_shared<Terrain>(m_deviceResources);
    m_Terrain->setScaling(m_sceneScaling);

    m_lightPos = { 0.0f, 0.0f, 0.0f};
    m_Light = std::make_shared<Sphere>(m_deviceResources, m_lightPos, 250.0f);
    m_Light->setScaling(100);
    XMFLOAT2 gridSize = m_Terrain->getGridSize();
    m_drawParamsConstantBufferData.terrainParams.amplitude = 2500.0f;
    m_drawParamsConstantBufferData.terrainParams.columns = gridSize.x;
    m_drawParamsConstantBufferData.terrainParams.rows = gridSize.y;

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

Terrain_engine::SceneRenderer::~SceneRenderer()
{
}

void SceneRenderer::CreateWindowSizeDependentResources()
{
    SetProjection(10000.0f);
    XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(m_Camera->GetMatrix()));
    XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationX(0.0)));
}

void SceneRenderer::Update(DX::StepTimer const& timer)
{
    if (!m_tracking)
    {
        float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
        double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
        float radians = static_cast<float>(fmod(totalRotation, XM_2PI));
    }
}

void Terrain_engine::SceneRenderer::UpdateMousePosition(DirectX::XMFLOAT2 mousePoint)
{
    if (m_MousePoint.x == 0.0 && m_MousePoint.y == 0.0)
    {
        m_MousePoint = mousePoint;
        return;
    }
    float deltaX = mousePoint.x - m_MousePoint.x;
    float deltaY = mousePoint.y - m_MousePoint.y;
    m_MousePoint = mousePoint;

    constexpr double PI = 3.141592653589;
    float pitch = m_Camera->getPitch() - (deltaY / 150);
    pitch = fmod(pitch + PI, 2 * PI);
    if (pitch < 0)
        pitch += 2 * PI;
    pitch = pitch - PI;
    m_Camera->setPitch(pitch);

    float yaw = m_Camera->getYaw();
    yaw = yaw - deltaX / 150;
    m_Camera->setYaw(yaw);

    XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationX(0.0)));
}

void Terrain_engine::SceneRenderer::TogglePrimitiveRendering()
{
    m_renderTriangles = !m_renderTriangles;
    m_deviceResources->ChangeDrawMode(m_renderTriangles);
}

void Terrain_engine::SceneRenderer::ToggleShadowsRendering()
{
    m_renderShadows = !m_renderShadows;
}

void Terrain_engine::SceneRenderer::UpdateLightPosition(DirectX::XMFLOAT3 lightPos)
{
    m_lightPos = lightPos;
}

void Terrain_engine::SceneRenderer::UpdateTerrainSettings(TerrainParams params)
{
    if (!m_loadingComplete)
        return;

    m_loadingComplete = false;
    
    m_drawParamsConstantBufferData.terrainParams = params;
    XMFLOAT2 oldGridSize = m_Terrain->getGridSize();
    if (oldGridSize.x == params.columns && oldGridSize.y == params.rows)
    {
        m_loadingComplete = true;
        return;
    }
    m_Terrain->setGridSize(params.columns, params.rows);
    m_Terrain->CreateIndices();
    m_Terrain->CreateVertices();
    if (m_usesTessellation)
        m_Terrain->CreateQuadIndices();
    m_loadingComplete = true;
}

void Terrain_engine::SceneRenderer::UseTessellation(bool useTessellation)
{
    m_usesTessellation = useTessellation;
    m_loadingComplete = false;
    if (m_usesTessellation)
        m_Terrain->CreateQuadIndices();
    else
        m_Terrain->CreateIndices();
    m_loadingComplete = true;
}

void Terrain_engine::SceneRenderer::UseTexture(bool useTexture)
{
    m_drawParamsConstantBufferData.useTexture = (useTexture ? 1.0f : 0.0f);
}

void Terrain_engine::SceneRenderer::DrawLOD(bool drawLOD)
{
    m_drawLOD = drawLOD;
}

void Terrain_engine::SceneRenderer::SetTextureSize(int width, int height)
{
    m_drawParamsConstantBufferData.textureSize.x = (float)width;
    m_drawParamsConstantBufferData.textureSize.y = (float)height;
}

void Terrain_engine::SceneRenderer::UpdateViewDistance(double viewDistance)
{
    SetProjection(viewDistance);
}

bool Terrain_engine::SceneRenderer::IsReadyToRender()
{
    return m_loadingComplete;
}

std::shared_ptr<Camera> Terrain_engine::SceneRenderer::getCamera()
{
    return m_Camera;
}

void SceneRenderer::Render()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto rasterizer = m_deviceResources->GetRasterizerState();

    m_drawParamsConstantBufferData.scaling = m_sceneScaling;
    m_drawParamsConstantBufferData.renderShadows = (float)m_renderShadows;
    m_drawParamsConstantBufferData.lightPos = m_lightPos;
    m_drawParamsConstantBufferData.tessellationParams.usesTessellation = m_usesTessellation ? 1.0f : 0.0f;
    m_drawParamsConstantBufferData.tessellationParams.drawLOD = m_drawLOD ? 1.0f : 0.0f;
    m_drawParamsConstantBufferData.drawTerrain = 1.0f;

    context->UpdateSubresource1(m_drawParamsConstantBuffer.Get(), 0, NULL, &m_drawParamsConstantBufferData, 0, 0, 0);

    auto terrainShaderResouce = m_deviceResources->GetTerrainHeightShaderResourceView();
    ID3D11SamplerState* const sampler[1] = { m_deviceResources->GetSampler() };

    if (m_usesTessellation)
    {
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
        context->HSSetShader(m_hullShader.Get(), nullptr, 0);
        context->DSSetShader(m_domainShader.Get(), nullptr, 0);
        context->DSSetShaderResources(0, 1, &terrainShaderResouce);
        context->DSSetSamplers(0, 1, sampler);
        context->DSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
        context->DSSetConstantBuffers1(1, 1, m_drawParamsConstantBuffer.GetAddressOf(), nullptr, nullptr);
        context->HSSetConstantBuffers1(1, 1, m_drawParamsConstantBuffer.GetAddressOf(), nullptr, nullptr);
    }
    else
    {
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        context->HSSetShader(nullptr, nullptr, 0);
        context->DSSetShader(nullptr, nullptr, 0);

        context->VSSetShaderResources(0, 1, &terrainShaderResouce);
        context->VSSetSamplers(0, 1, sampler);
    }

    auto terrainTextureShaderResouce = m_deviceResources->GetTerrainTextureShaderResourceView();
    context->PSSetSamplers(0, 1, sampler);
    context->PSSetShaderResources(0, 1, &terrainTextureShaderResouce);

    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers1(1, 1, m_drawParamsConstantBuffer.GetAddressOf(), nullptr, nullptr);

    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->PSSetConstantBuffers1(1, 1, m_drawParamsConstantBuffer.GetAddressOf(), nullptr, nullptr);
    context->RSSetState(rasterizer);
    m_Terrain->Draw();


    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->HSSetShader(nullptr, nullptr, 0);
    context->DSSetShader(nullptr, nullptr, 0);

    XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(m_lightPos.x, m_lightPos.y, m_lightPos.z)));
    context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
    context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

    m_drawParamsConstantBufferData.scaling = m_sceneScaling;
    m_drawParamsConstantBufferData.renderShadows = 0.0f;
    m_drawParamsConstantBufferData.lightPos = m_lightPos;
    m_drawParamsConstantBufferData.eyePos = m_Camera->GetEye();
    m_drawParamsConstantBufferData.tessellationParams.usesTessellation = 0.0f;
    m_drawParamsConstantBufferData.tessellationParams.drawLOD = 0.0f;
    m_drawParamsConstantBufferData.drawTerrain = 0.0f;
    context->UpdateSubresource1(m_drawParamsConstantBuffer.Get(), 0, NULL, &m_drawParamsConstantBufferData, 0, 0, 0);
    context->VSSetConstantBuffers1(1, 1, m_drawParamsConstantBuffer.GetAddressOf(), nullptr, nullptr);
    m_Light->Draw();
}

void Terrain_engine::SceneRenderer::SetProjection(double viewDistance)
{
    Size outputSize = m_deviceResources->GetOutputSize();
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;
    float lightFovAngleY = 40.0f * XM_PI / 180.0f;

    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.1f, (float)viewDistance);

    XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

    XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

    XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
}

void Terrain_engine::SceneRenderer::RenderFromCameraView()
{
    if (!m_loadingComplete)
    {
        return;
    }
    auto context = m_deviceResources->GetD3DDeviceContext();

    XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixIdentity());
    XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(m_Camera->GetMatrix()));
    context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
    context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

    Render();
}

void SceneRenderer::CreateDeviceDependentResources()
{
    auto loadVSTask = DX::ReadDataAsync(L"VertexShader.cso");
    auto loadPSTask = DX::ReadDataAsync(L"PixelShader.cso");
    auto loadGSTask = DX::ReadDataAsync(L"GeometryShader.cso");
    auto loadHSTask = DX::ReadDataAsync(L"HullShader.cso");
    auto loadDSTask = DX::ReadDataAsync(L"DomainShader.cso");

    auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

        static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
        });

    auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

        CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));

        CD3D11_BUFFER_DESC drawDataConstantBufferDesc(sizeof(DrawParamsConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&drawDataConstantBufferDesc, nullptr, &m_drawParamsConstantBuffer));
        });

    auto createGSTask = loadGSTask.then([this](const std::vector<byte>& fileData) {
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGeometryShader(&fileData[0], fileData.size(), nullptr, &m_geometryShader));
        });

    auto createHSTask = loadHSTask.then([this](const std::vector<byte>& fileData) {
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateHullShader(&fileData[0], fileData.size(), nullptr, &m_hullShader));
        });

    auto createDSTask = loadDSTask.then([this](const std::vector<byte>& fileData) {
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateDomainShader(&fileData[0], fileData.size(), nullptr, &m_domainShader));
        });

    auto createTerrainTask = (createPSTask && createVSTask).then([this]()
        {
            m_Terrain->CreateIndices();
            m_Terrain->CreateVertices();
        });

    auto createLightTask = (createPSTask && createVSTask).then([this]()
        {
            m_Light->CreateIndices();
            m_Light->CreateVertices();
        });

    auto meshesTask = (createTerrainTask && createLightTask).then([this]() {
        m_loadingComplete = true;
        });
}

void SceneRenderer::ReleaseDeviceDependentResources()
{
    m_loadingComplete = false;
    m_vertexShader.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_geometryShader.Reset();
    m_constantBuffer.Reset();
    m_drawParamsConstantBuffer.Reset();
    m_Terrain->ResetBuffers();
    m_Light->ResetBuffers();
}
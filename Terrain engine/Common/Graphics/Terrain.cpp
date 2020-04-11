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
    m_PerlinNoise = std::make_shared<PerlinNoise>();
    m_scaling = 100;
}

Terrain::Terrain(std::shared_ptr<DX::DeviceResources> deviceResources, int columns, int rows)
{
    m_deviceResources = deviceResources;
    m_PerlinNoise = std::make_shared<PerlinNoise>();

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
    std::unique_ptr<XMFLOAT3[]> pos(new XMFLOAT3[m_Columns * m_Rows]);

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
            double height = m_PerlinNoise->GenerateValue(i, j);
            double colorValue = min(max(height + 0.4, 0), 1);
            XMFLOAT3 heightColor = GetColorFromHeight(height);
            height *= m_scaling / 20;
            VertexPositionColor vertex;
            vertex.pos = XMFLOAT3(tempX, height, tempZ);
            vertex.color = heightColor;
            vertex.normal = heightColor;
            m_vertices.emplace_back(vertex);
            pos.get()[i * m_Rows + j] = m_vertices[i * m_Rows + j].pos;
            tempZ += stepZ;
        }
        tempX += stepX;
    }

    m_verticesCount = m_vertices.size();

    size_t nFaces = m_indexCount / 3;
    size_t nVerts = m_Columns * m_Rows;

    std::unique_ptr<XMFLOAT3[]> normals(new XMFLOAT3[nVerts]);
    ComputeNormals(&m_indices.front(), nFaces, pos.get(), nVerts, CNORM_WIND_CW, normals.get());

    for (size_t j = 0; j < nVerts; j++)
        m_vertices[j].normal = normals.get()[j];

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
    m_indices.clear();
    for (size_t i = 0; i < m_Columns - 1; i++)
    {
        for (size_t j = 0; j < m_Rows - 1; j++)
        {
            //m_indices.emplace_back(i * m_Rows + j);             // 0
            //m_indices.emplace_back((i + 1) * m_Rows + j);       // 2
            //m_indices.emplace_back(i * m_Rows + j + 1);         // 1
            //m_indices.emplace_back((i + 1) * m_Rows + j);       // 2
            //m_indices.emplace_back((i + 1) * m_Rows + j + 1);   // 3
            //m_indices.emplace_back(i * m_Rows + j + 1);         // 1

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

void Terrain::setMode(int index)
{
    m_PerlinNoise->setPrimeIndex(index);
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

std::shared_ptr<PerlinNoise> Terrain_engine::Terrain::getPerlinNoise()
{
    return m_PerlinNoise;
}

DirectX::XMFLOAT3 Terrain::GetColorFromHeight(double height)
{
    height = (height + 1) / 2;
    height = min(max(0.0, height) + 0.01, 1);
    int pos = (int)round(height * 255);
    return m_TerrainColormap[pos];
}

std::map<int, XMFLOAT3> Terrain::m_TerrainColormap{
    {0, XMFLOAT3(0.000000, 0.000000, 0.000000)},
    {1, XMFLOAT3(0.000000, 0.000000, 0.168366)},
    {2, XMFLOAT3(0.000000, 0.000000, 0.221176)},
    {3, XMFLOAT3(0.000000, 0.000000, 0.261961)},
    {4, XMFLOAT3(0.000000, 0.000000, 0.305098)},
    {5, XMFLOAT3(0.000000, 0.000000, 0.346405)},
    {6, XMFLOAT3(0.000000, 0.000000, 0.389020)},
    {7, XMFLOAT3(0.000000, 0.000000, 0.432941)},
    {8, XMFLOAT3(0.000000, 0.005490, 0.454902)},
    {9, XMFLOAT3(0.001569, 0.016471, 0.454902)},
    {10, XMFLOAT3(0.005229, 0.026144, 0.454902)},
    {11, XMFLOAT3(0.007843, 0.034510, 0.454902)},
    {12, XMFLOAT3(0.008627, 0.044706, 0.455686)},
    {13, XMFLOAT3(0.012288, 0.052549, 0.458824)},
    {14, XMFLOAT3(0.015948, 0.063268, 0.458824)},
    {15, XMFLOAT3(0.019608, 0.070588, 0.458824)},
    {16, XMFLOAT3(0.019608, 0.081569, 0.458824)},
    {17, XMFLOAT3(0.023007, 0.089150, 0.458824)},
    {18, XMFLOAT3(0.026667, 0.099608, 0.461961)},
    {19, XMFLOAT3(0.030327, 0.107712, 0.462745)},
    {20, XMFLOAT3(0.031373, 0.117647, 0.462745)},
    {21, XMFLOAT3(0.033725, 0.126275, 0.462745)},
    {22, XMFLOAT3(0.037386, 0.135686, 0.462745)},
    {23, XMFLOAT3(0.041046, 0.144837, 0.464575)},
    {24, XMFLOAT3(0.043137, 0.153725, 0.466667)},
    {25, XMFLOAT3(0.044444, 0.163399, 0.466667)},
    {26, XMFLOAT3(0.048105, 0.170719, 0.466667)},
    {27, XMFLOAT3(0.051765, 0.178824, 0.466667)},
    {28, XMFLOAT3(0.055425, 0.189281, 0.467190)},
    {29, XMFLOAT3(0.058824, 0.196601, 0.470588)},
    {30, XMFLOAT3(0.058824, 0.203922, 0.470588)},
    {31, XMFLOAT3(0.062484, 0.214902, 0.470588)},
    {32, XMFLOAT3(0.066144, 0.222484, 0.470588)},
    {33, XMFLOAT3(0.069804, 0.229804, 0.470588)},
    {34, XMFLOAT3(0.070588, 0.237124, 0.473464)},
    {35, XMFLOAT3(0.073203, 0.247059, 0.474510)},
    {36, XMFLOAT3(0.076863, 0.255686, 0.474510)},
    {37, XMFLOAT3(0.080523, 0.263007, 0.474510)},
    {38, XMFLOAT3(0.084183, 0.270327, 0.474510)},
    {39, XMFLOAT3(0.086275, 0.277647, 0.476078)},
    {40, XMFLOAT3(0.087582, 0.284967, 0.478431)},
    {41, XMFLOAT3(0.091242, 0.293333, 0.478431)},
    {42, XMFLOAT3(0.094902, 0.303529, 0.478431)},
    {43, XMFLOAT3(0.098562, 0.310850, 0.478431)},
    {44, XMFLOAT3(0.101961, 0.318170, 0.478693)},
    {45, XMFLOAT3(0.101961, 0.325490, 0.482353)},
    {46, XMFLOAT3(0.105621, 0.332810, 0.482353)},
    {47, XMFLOAT3(0.109281, 0.340131, 0.482353)},
    {48, XMFLOAT3(0.112941, 0.347451, 0.482353)},
    {49, XMFLOAT3(0.116601, 0.354771, 0.482353)},
    {50, XMFLOAT3(0.120261, 0.362092, 0.484967)},
    {51, XMFLOAT3(0.121569, 0.369412, 0.486275)},
    {52, XMFLOAT3(0.123660, 0.376732, 0.486275)},
    {53, XMFLOAT3(0.127320, 0.384052, 0.486275)},
    {54, XMFLOAT3(0.130980, 0.389804, 0.486275)},
    {55, XMFLOAT3(0.134641, 0.394771, 0.487582)},
    {56, XMFLOAT3(0.138301, 0.402092, 0.490196)},
    {57, XMFLOAT3(0.141176, 0.409412, 0.490196)},
    {58, XMFLOAT3(0.141699, 0.416732, 0.490196)},
    {59, XMFLOAT3(0.145359, 0.423791, 0.490196)},
    {60, XMFLOAT3(0.149020, 0.427451, 0.490196)},
    {61, XMFLOAT3(0.152680, 0.434771, 0.493856)},
    {62, XMFLOAT3(0.156340, 0.442092, 0.494118)},
    {63, XMFLOAT3(0.160000, 0.449412, 0.494118)},
    {64, XMFLOAT3(0.160784, 0.453856, 0.494118)},
    {65, XMFLOAT3(0.163399, 0.460131, 0.494118)},
    {66, XMFLOAT3(0.167059, 0.467451, 0.496471)},
    {67, XMFLOAT3(0.170719, 0.472680, 0.498039)},
    {68, XMFLOAT3(0.174379, 0.478170, 0.498039)},
    {69, XMFLOAT3(0.178039, 0.485490, 0.498039)},
    {70, XMFLOAT3(0.181699, 0.491503, 0.498039)},
    {71, XMFLOAT3(0.185359, 0.496209, 0.499085)},
    {72, XMFLOAT3(0.188235, 0.501961, 0.500392)},
    {73, XMFLOAT3(0.188235, 0.502484, 0.493595)},
    {74, XMFLOAT3(0.188497, 0.505882, 0.489935)},
    {75, XMFLOAT3(0.192157, 0.505882, 0.486275)},
    {76, XMFLOAT3(0.192157, 0.509543, 0.482614)},
    {77, XMFLOAT3(0.195556, 0.513203, 0.478954)},
    {78, XMFLOAT3(0.196078, 0.513726, 0.472157)},
    {79, XMFLOAT3(0.198954, 0.516601, 0.467712)},
    {80, XMFLOAT3(0.200000, 0.520261, 0.464052)},
    {81, XMFLOAT3(0.202353, 0.521569, 0.460392)},
    {82, XMFLOAT3(0.203922, 0.523660, 0.454641)},
    {83, XMFLOAT3(0.205752, 0.525490, 0.449150)},
    {84, XMFLOAT3(0.207843, 0.527059, 0.445490)},
    {85, XMFLOAT3(0.209150, 0.530719, 0.440523)},
    {86, XMFLOAT3(0.211765, 0.533333, 0.434248)},
    {87, XMFLOAT3(0.212549, 0.534118, 0.430588)},
    {88, XMFLOAT3(0.215686, 0.537778, 0.426928)},
    {89, XMFLOAT3(0.215948, 0.541176, 0.423007)},
    {90, XMFLOAT3(0.219608, 0.541176, 0.415686)},
    {91, XMFLOAT3(0.219608, 0.544837, 0.412026)},
    {92, XMFLOAT3(0.223007, 0.548497, 0.408366)},
    {93, XMFLOAT3(0.223529, 0.549020, 0.401569)},
    {94, XMFLOAT3(0.226405, 0.551895, 0.397124)},
    {95, XMFLOAT3(0.227451, 0.552941, 0.393464)},
    {96, XMFLOAT3(0.229804, 0.555294, 0.387451)},
    {97, XMFLOAT3(0.231373, 0.558954, 0.382222)},
    {98, XMFLOAT3(0.233203, 0.560784, 0.378562)},
    {99, XMFLOAT3(0.236863, 0.562353, 0.373333)},
    {100, XMFLOAT3(0.239216, 0.566013, 0.367320)},
    {101, XMFLOAT3(0.240261, 0.568627, 0.363660)},
    {102, XMFLOAT3(0.243137, 0.569412, 0.359216)},
    {103, XMFLOAT3(0.243660, 0.572549, 0.352418)},
    {104, XMFLOAT3(0.247059, 0.572810, 0.348758)},
    {105, XMFLOAT3(0.247059, 0.576471, 0.345098)},
    {106, XMFLOAT3(0.250719, 0.580131, 0.337778)},
    {107, XMFLOAT3(0.250980, 0.580392, 0.333856)},
    {108, XMFLOAT3(0.254118, 0.583529, 0.330196)},
    {109, XMFLOAT3(0.254902, 0.587190, 0.323660)},
    {110, XMFLOAT3(0.257516, 0.588235, 0.318954)},
    {111, XMFLOAT3(0.261176, 0.590588, 0.315294)},
    {112, XMFLOAT3(0.262745, 0.592157, 0.309543)},
    {113, XMFLOAT3(0.264575, 0.593987, 0.304052)},
    {114, XMFLOAT3(0.266667, 0.597647, 0.300392)},
    {115, XMFLOAT3(0.267974, 0.600000, 0.295425)},
    {116, XMFLOAT3(0.270588, 0.601046, 0.289150)},
    {117, XMFLOAT3(0.271373, 0.604706, 0.284706)},
    {118, XMFLOAT3(0.275033, 0.607843, 0.277909)},
    {119, XMFLOAT3(0.278954, 0.608105, 0.274771)},
    {120, XMFLOAT3(0.286275, 0.611765, 0.278431)},
    {121, XMFLOAT3(0.297255, 0.611765, 0.282092)},
    {122, XMFLOAT3(0.304837, 0.615163, 0.282353)},
    {123, XMFLOAT3(0.315294, 0.618824, 0.285490)},
    {124, XMFLOAT3(0.323399, 0.619608, 0.286275)},
    {125, XMFLOAT3(0.333333, 0.622222, 0.288889)},
    {126, XMFLOAT3(0.341961, 0.625882, 0.292549)},
    {127, XMFLOAT3(0.351373, 0.627451, 0.294118)},
    {128, XMFLOAT3(0.362353, 0.629281, 0.295948)},
    {129, XMFLOAT3(0.371765, 0.631373, 0.298039)},
    {130, XMFLOAT3(0.380392, 0.632680, 0.299346)},
    {131, XMFLOAT3(0.390327, 0.636340, 0.301961)},
    {132, XMFLOAT3(0.398431, 0.639216, 0.302745)},
    {133, XMFLOAT3(0.408889, 0.639739, 0.306405)},
    {134, XMFLOAT3(0.416471, 0.643399, 0.309804)},
    {135, XMFLOAT3(0.427451, 0.647059, 0.309804)},
    {136, XMFLOAT3(0.434771, 0.647059, 0.313464)},
    {137, XMFLOAT3(0.445490, 0.650458, 0.313726)},
    {138, XMFLOAT3(0.456471, 0.650980, 0.316863)},
    {139, XMFLOAT3(0.464575, 0.653856, 0.320523)},
    {140, XMFLOAT3(0.471895, 0.657516, 0.321569)},
    {141, XMFLOAT3(0.476863, 0.658824, 0.321569)},
    {142, XMFLOAT3(0.482614, 0.658824, 0.323660)},
    {143, XMFLOAT3(0.489935, 0.660654, 0.325490)},
    {144, XMFLOAT3(0.497255, 0.662745, 0.325490)},
    {145, XMFLOAT3(0.503268, 0.664052, 0.326797)},
    {146, XMFLOAT3(0.507974, 0.666667, 0.329412)},
    {147, XMFLOAT3(0.515294, 0.667451, 0.329412)},
    {148, XMFLOAT3(0.522614, 0.670588, 0.329935)},
    {149, XMFLOAT3(0.529673, 0.670850, 0.333333)},
    {150, XMFLOAT3(0.533333, 0.674510, 0.333333)},
    {151, XMFLOAT3(0.540654, 0.674510, 0.333333)},
    {152, XMFLOAT3(0.547974, 0.674510, 0.336732)},
    {153, XMFLOAT3(0.552157, 0.677647, 0.337255)},
    {154, XMFLOAT3(0.558693, 0.678431, 0.337255)},
    {155, XMFLOAT3(0.566013, 0.681046, 0.339869)},
    {156, XMFLOAT3(0.573333, 0.682353, 0.341176)},
    {157, XMFLOAT3(0.580654, 0.684444, 0.341176)},
    {158, XMFLOAT3(0.586144, 0.686275, 0.343007)},
    {159, XMFLOAT3(0.591373, 0.686275, 0.345098)},
    {160, XMFLOAT3(0.598693, 0.687582, 0.345098)},
    {161, XMFLOAT3(0.606013, 0.690196, 0.346144)},
    {162, XMFLOAT3(0.612549, 0.690980, 0.349020)},
    {163, XMFLOAT3(0.616732, 0.694118, 0.349020)},
    {164, XMFLOAT3(0.624052, 0.694379, 0.349281)},
    {165, XMFLOAT3(0.631373, 0.698039, 0.352941)},
    {166, XMFLOAT3(0.638693, 0.698039, 0.352941)},
    {167, XMFLOAT3(0.646013, 0.701438, 0.352941)},
    {168, XMFLOAT3(0.650196, 0.701961, 0.356078)},
    {169, XMFLOAT3(0.656732, 0.701961, 0.356863)},
    {170, XMFLOAT3(0.664052, 0.704575, 0.356863)},
    {171, XMFLOAT3(0.671373, 0.705882, 0.359216)},
    {172, XMFLOAT3(0.678693, 0.707974, 0.360784)},
    {173, XMFLOAT3(0.684183, 0.709804, 0.360784)},
    {174, XMFLOAT3(0.689412, 0.711373, 0.362353)},
    {175, XMFLOAT3(0.696732, 0.713726, 0.364706)},
    {176, XMFLOAT3(0.704052, 0.714771, 0.364706)},
    {177, XMFLOAT3(0.711373, 0.717647, 0.365490)},
    {178, XMFLOAT3(0.717647, 0.717124, 0.368627)},
    {179, XMFLOAT3(0.717909, 0.713464, 0.368627)},
    {180, XMFLOAT3(0.721569, 0.709804, 0.368627)},
    {181, XMFLOAT3(0.721569, 0.709804, 0.372288)},
    {182, XMFLOAT3(0.724967, 0.706405, 0.372549)},
    {183, XMFLOAT3(0.725490, 0.702745, 0.372549)},
    {184, XMFLOAT3(0.728366, 0.699085, 0.375425)},
    {185, XMFLOAT3(0.729412, 0.695425, 0.376471)},
    {186, XMFLOAT3(0.731765, 0.691765, 0.378824)},
    {187, XMFLOAT3(0.733333, 0.688105, 0.380392)},
    {188, XMFLOAT3(0.733333, 0.684444, 0.380392)},
    {189, XMFLOAT3(0.734902, 0.680784, 0.381961)},
    {190, XMFLOAT3(0.737255, 0.677124, 0.384314)},
    {191, XMFLOAT3(0.738301, 0.673464, 0.384314)},
    {192, XMFLOAT3(0.741176, 0.669804, 0.385098)},
    {193, XMFLOAT3(0.741699, 0.666144, 0.388235)},
    {194, XMFLOAT3(0.745098, 0.662484, 0.388235)},
    {195, XMFLOAT3(0.745098, 0.658824, 0.388235)},
    {196, XMFLOAT3(0.745098, 0.655163, 0.391895)},
    {197, XMFLOAT3(0.748497, 0.651503, 0.392157)},
    {198, XMFLOAT3(0.749020, 0.647843, 0.392157)},
    {199, XMFLOAT3(0.751895, 0.644183, 0.395033)},
    {200, XMFLOAT3(0.752941, 0.640523, 0.396078)},
    {201, XMFLOAT3(0.755294, 0.639216, 0.403137)},
    {202, XMFLOAT3(0.761046, 0.641307, 0.412026)},
    {203, XMFLOAT3(0.766536, 0.643137, 0.419346)},
    {204, XMFLOAT3(0.770196, 0.644706, 0.428235)},
    {205, XMFLOAT3(0.773856, 0.647059, 0.437909)},
    {206, XMFLOAT3(0.777516, 0.648105, 0.446275)},
    {207, XMFLOAT3(0.781961, 0.651765, 0.456471)},
    {208, XMFLOAT3(0.788758, 0.654902, 0.464314)},
    {209, XMFLOAT3(0.792418, 0.655163, 0.475033)},
    {210, XMFLOAT3(0.796078, 0.658824, 0.482353)},
    {211, XMFLOAT3(0.799739, 0.662484, 0.493333)},
    {212, XMFLOAT3(0.803399, 0.666144, 0.504314)},
    {213, XMFLOAT3(0.810196, 0.669804, 0.512157)},
    {214, XMFLOAT3(0.814641, 0.673464, 0.522353)},
    {215, XMFLOAT3(0.818301, 0.677124, 0.533333)},
    {216, XMFLOAT3(0.821961, 0.680784, 0.541961)},
    {217, XMFLOAT3(0.825621, 0.684444, 0.551373)},
    {218, XMFLOAT3(0.831111, 0.688105, 0.562353)},
    {219, XMFLOAT3(0.836863, 0.691765, 0.573333)},
    {220, XMFLOAT3(0.840523, 0.695425, 0.583007)},
    {221, XMFLOAT3(0.844183, 0.699085, 0.591373)},
    {222, XMFLOAT3(0.847843, 0.703529, 0.602353)},
    {223, XMFLOAT3(0.852026, 0.710327, 0.613333)},
    {224, XMFLOAT3(0.859085, 0.714248, 0.624314)},
    {225, XMFLOAT3(0.862745, 0.721569, 0.635294)},
    {226, XMFLOAT3(0.866405, 0.725229, 0.646275)},
    {227, XMFLOAT3(0.870065, 0.732288, 0.657255)},
    {228, XMFLOAT3(0.873726, 0.736471, 0.665098)},
    {229, XMFLOAT3(0.880261, 0.743007, 0.675294)},
    {230, XMFLOAT3(0.884967, 0.750327, 0.686275)},
    {231, XMFLOAT3(0.888627, 0.757647, 0.697255)},
    {232, XMFLOAT3(0.892288, 0.764967, 0.708235)},
    {233, XMFLOAT3(0.895948, 0.772288, 0.719216)},
    {234, XMFLOAT3(0.901176, 0.779608, 0.731765)},
    {235, XMFLOAT3(0.907190, 0.786928, 0.745098)},
    {236, XMFLOAT3(0.910850, 0.794248, 0.756078)},
    {237, XMFLOAT3(0.914510, 0.801569, 0.767059)},
    {238, XMFLOAT3(0.918170, 0.808889, 0.778039)},
    {239, XMFLOAT3(0.922092, 0.816471, 0.789020)},
    {240, XMFLOAT3(0.929412, 0.827451, 0.800000)},
    {241, XMFLOAT3(0.933072, 0.834771, 0.810980)},
    {242, XMFLOAT3(0.936732, 0.842092, 0.825359)},
    {243, XMFLOAT3(0.940392, 0.852549, 0.836863)},
    {244, XMFLOAT3(0.944052, 0.863529, 0.847843)},
    {245, XMFLOAT3(0.950327, 0.871895, 0.858824)},
    {246, XMFLOAT3(0.955294, 0.881569, 0.872157)},
    {247, XMFLOAT3(0.958954, 0.892549, 0.884706)},
    {248, XMFLOAT3(0.962614, 0.903529, 0.895686)},
    {249, XMFLOAT3(0.966275, 0.914510, 0.908235)},
    {250, XMFLOAT3(0.971242, 0.925490, 0.921569)},
    {251, XMFLOAT3(0.977516, 0.936471, 0.933595)},
    {252, XMFLOAT3(0.981176, 0.947451, 0.947451)},
    {253, XMFLOAT3(0.984837, 0.958954, 0.958954)},
    {254, XMFLOAT3(0.988497, 0.973333, 0.973333)},
    {255, XMFLOAT3(0.992157, 0.984314, 0.984314)},
};

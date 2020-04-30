#include "pch.h"
#include "BasicLoader.h"

//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "DDSTextureLoader.h"
#include <memory>

using namespace Microsoft::WRL;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;
using namespace std;
using namespace concurrency;

BasicLoader::BasicLoader(
    _In_ ID3D11Device* d3dDevice,
    _In_ ID3D11DeviceContext3* d3dContext,
    _In_opt_ IWICImagingFactory2* wicFactory
) :
    m_d3dDevice(d3dDevice),
    m_d3dContext(d3dContext),
    m_wicFactory(wicFactory)
{
    // Create a new BasicReaderWriter to do raw file I/O.
    m_basicReaderWriter = ref new BasicReaderWriter();
}

template <class DeviceChildType>
inline void BasicLoader::SetDebugName(
    _In_ DeviceChildType* object,
    _In_ Platform::String^ name
)
{
#if defined(_DEBUG)
    // Only assign debug names in debug builds.

    char nameString[1024];
    int nameStringLength = WideCharToMultiByte(
        CP_ACP,
        0,
        name->Data(),
        -1,
        nameString,
        1024,
        nullptr,
        nullptr
    );

    if (nameStringLength == 0)
    {
        char defaultNameString[] = "BasicLoaderObject";
        DX::ThrowIfFailed(
            object->SetPrivateData(
                WKPDID_D3DDebugObjectName,
                sizeof(defaultNameString) - 1,
                defaultNameString
            )
        );
    }
    else
    {
        DX::ThrowIfFailed(
            object->SetPrivateData(
                WKPDID_D3DDebugObjectName,
                nameStringLength - 1,
                nameString
            )
        );
    }
#endif
}

Platform::String^ BasicLoader::GetExtension(
    _In_ Platform::String^ filename
)
{
    int lastDotIndex = -1;
    for (int i = filename->Length() - 1; i >= 0 && lastDotIndex == -1; i--)
    {
        if (*(filename->Data() + i) == '.')
        {
            lastDotIndex = i;
        }
    }
    if (lastDotIndex != -1)
    {
        std::unique_ptr<wchar_t[]> extension(new wchar_t[filename->Length() - lastDotIndex]);
        for (unsigned int i = 0; i < filename->Length() - lastDotIndex; i++)
        {
            extension[i] = tolower(*(filename->Data() + lastDotIndex + 1 + i));
        }
        return ref new Platform::String(extension.get());
    }
    return "";
}

void BasicLoader::CreateTexture(
    _In_ bool decodeAsDDS,
    _In_reads_bytes_(dataSize) byte* data,
    _In_ uint32 dataSize,
    _Out_opt_ ID3D11Texture2D** texture,
    _Out_opt_ ID3D11ShaderResourceView** textureView,
    _In_ bool generateMipMaps,
    _In_opt_ Platform::String^ debugName
)
{
    ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    ComPtr<ID3D11Texture2D> texture2D;

    if (decodeAsDDS)
    {
        ComPtr<ID3D11Resource> resource;

        if (textureView == nullptr)
        {
            CreateDDSTextureFromMemory(
                m_d3dDevice.Get(),
                data,
                dataSize,
                &resource,
                nullptr
            );
        }
        else
        {
            CreateDDSTextureFromMemory(
                m_d3dDevice.Get(),
                data,
                dataSize,
                &resource,
                &shaderResourceView
            );
        }

        DX::ThrowIfFailed(
            resource.As(&texture2D)
        );

        if (textureView != nullptr)
        {
            *textureView = shaderResourceView.Detach();
        }
    }
    else
    {
        if (m_wicFactory.Get() == nullptr)
        {
            // A WIC factory object is required in order to load texture
            // assets stored in non-DDS formats.  If BasicLoader was not
            // initialized with one, create one as needed.
            DX::ThrowIfFailed(
                CoCreateInstance(
                    CLSID_WICImagingFactory,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&m_wicFactory)
                )
            );
        }

        ComPtr<IWICStream> stream;
        DX::ThrowIfFailed(
            m_wicFactory->CreateStream(&stream)
        );

        DX::ThrowIfFailed(
            stream->InitializeFromMemory(
                data,
                dataSize
            )
        );

        ComPtr<IWICBitmapDecoder> bitmapDecoder;
        DX::ThrowIfFailed(
            m_wicFactory->CreateDecoderFromStream(
                stream.Get(),
                nullptr,
                WICDecodeMetadataCacheOnDemand,
                &bitmapDecoder
            )
        );

        ComPtr<IWICBitmapFrameDecode> bitmapFrame;
        DX::ThrowIfFailed(
            bitmapDecoder->GetFrame(0, &bitmapFrame)
        );

        ComPtr<IWICFormatConverter> formatConverter;
        DX::ThrowIfFailed(
            m_wicFactory->CreateFormatConverter(&formatConverter)
        );

        DX::ThrowIfFailed(
            formatConverter->Initialize(
                bitmapFrame.Get(),
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom
            )
        );

        uint32 width;
        uint32 height;
        DX::ThrowIfFailed(
            bitmapFrame->GetSize(&width, &height)
        );

        std::unique_ptr<byte[]> bitmapPixels(new byte[width * height * 4]);
        DX::ThrowIfFailed(
            formatConverter->CopyPixels(
                nullptr,
                width * 4,
                width * height * 4,
                bitmapPixels.get()
            )
        );




        HRESULT hr;

        // Create texture
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = (generateMipMaps) ? 0u : 1u;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;

        if (generateMipMaps)
        {
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }
        else
        {
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags = 0;
        }

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = bitmapPixels.get();
        initData.SysMemPitch = width * 4;
        initData.SysMemSlicePitch = 0;

        hr = m_d3dDevice->CreateTexture2D(&desc, (generateMipMaps) ? nullptr : &initData, &texture2D);
        if (SUCCEEDED(hr) && texture2D)
        {
            if (textureView)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
                SRVDesc.Format = desc.Format;

                SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                SRVDesc.Texture2D.MipLevels = (generateMipMaps) ? unsigned(-1) : 1u;

                hr = m_d3dDevice->CreateShaderResourceView(texture2D.Get(), &SRVDesc, textureView);
                if (FAILED(hr))
                {
                    texture2D->Release();
                }

                if (generateMipMaps)
                {
                    m_d3dContext->UpdateSubresource(texture2D.Get(), 0, nullptr, bitmapPixels.get(), width * 4, 0);
                    m_d3dContext->GenerateMips(*textureView);
                }
            }
        }
    }

    SetDebugName(texture2D.Get(), debugName);

    if (texture != nullptr)
    {
        *texture = texture2D.Detach();
    }
}

void BasicLoader::CreateInputLayout(
    _In_reads_bytes_(bytecodeSize) byte* bytecode,
    _In_ uint32 bytecodeSize,
    _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    _In_ uint32 layoutDescNumElements,
    _Out_ ID3D11InputLayout** layout
)
{
    if (layoutDesc == nullptr)
    {
        // If no input layout is specified, use the BasicVertex layout.
        const D3D11_INPUT_ELEMENT_DESC basicVertexLayoutDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        DX::ThrowIfFailed(
            m_d3dDevice->CreateInputLayout(
                basicVertexLayoutDesc,
                ARRAYSIZE(basicVertexLayoutDesc),
                bytecode,
                bytecodeSize,
                layout
            )
        );
    }
    else
    {
        DX::ThrowIfFailed(
            m_d3dDevice->CreateInputLayout(
                layoutDesc,
                layoutDescNumElements,
                bytecode,
                bytecodeSize,
                layout
            )
        );
    }
}

void BasicLoader::LoadTexture(
    _In_ Platform::String^ filename,
    _Out_opt_ ID3D11Texture2D** texture,
    _Out_opt_ ID3D11ShaderResourceView** textureView,
    _In_ bool generateMipMaps
)
{
    Platform::Array<byte>^ textureData = m_basicReaderWriter->ReadData(filename);

    CreateTexture(
        GetExtension(filename) == "dds",
        textureData->Data,
        textureData->Length,
        texture,
        textureView,
        generateMipMaps,
        filename
    );
}

task<void> BasicLoader::LoadTextureAsync(
    _In_ Platform::String^ filename,
    _Out_opt_ ID3D11Texture2D** texture,
    _Out_opt_ ID3D11ShaderResourceView** textureView,
    _In_ bool generateMipMaps
)
{
    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ textureData)
        {
            CreateTexture(
                GetExtension(filename) == "dds",
                textureData->Data,
                textureData->Length,
                texture,
                textureView,
                generateMipMaps,
                filename
            );
        });
}

void BasicLoader::LoadShader(
    _In_ Platform::String^ filename,
    _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC layoutDesc[],
    _In_ uint32 layoutDescNumElements,
    _Out_ ID3D11VertexShader** shader,
    _Out_opt_ ID3D11InputLayout** layout
)
{
    Platform::Array<byte>^ bytecode = m_basicReaderWriter->ReadData(filename);

    DX::ThrowIfFailed(
        m_d3dDevice->CreateVertexShader(
            bytecode->Data,
            bytecode->Length,
            nullptr,
            shader
        )
    );

    SetDebugName(*shader, filename);

    if (layout != nullptr)
    {
        CreateInputLayout(
            bytecode->Data,
            bytecode->Length,
            layoutDesc,
            layoutDescNumElements,
            layout
        );

        SetDebugName(*layout, filename);
    }
}

task<void> BasicLoader::LoadShaderAsync(
    _In_ Platform::String^ filename,
    _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC layoutDesc[],
    _In_ uint32 layoutDescNumElements,
    _Out_ ID3D11VertexShader** shader,
    _Out_opt_ ID3D11InputLayout** layout
)
{
    // This method assumes that the lifetime of input arguments may be shorter
    // than the duration of this task.  In order to ensure accurate results, a
    // copy of all arguments passed by pointer must be made.  The method then
    // ensures that the lifetime of the copied data exceeds that of the task.

    // Create copies of the layoutDesc array as well as the SemanticName strings,
    // both of which are pointers to data whose lifetimes may be shorter than that
    // of this method's task.
    shared_ptr<vector<D3D11_INPUT_ELEMENT_DESC>> layoutDescCopy;
    shared_ptr<vector<string>> layoutDescSemanticNamesCopy;
    if (layoutDesc != nullptr)
    {
        layoutDescCopy.reset(
            new vector<D3D11_INPUT_ELEMENT_DESC>(
                layoutDesc,
                layoutDesc + layoutDescNumElements
                )
        );

        layoutDescSemanticNamesCopy.reset(
            new vector<string>(layoutDescNumElements)
        );

        for (uint32 i = 0; i < layoutDescNumElements; i++)
        {
            layoutDescSemanticNamesCopy->at(i).assign(layoutDesc[i].SemanticName);
        }
    }

    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ bytecode)
        {
            DX::ThrowIfFailed(
                m_d3dDevice->CreateVertexShader(
                    bytecode->Data,
                    bytecode->Length,
                    nullptr,
                    shader
                )
            );

            SetDebugName(*shader, filename);

            if (layout != nullptr)
            {
                if (layoutDesc != nullptr)
                {
                    // Reassign the SemanticName elements of the layoutDesc array copy to point
                    // to the corresponding copied strings. Performing the assignment inside the
                    // lambda body ensures that the lambda will take a reference to the shared_ptr
                    // that holds the data.  This will guarantee that the data is still valid when
                    // CreateInputLayout is called.
                    for (uint32 i = 0; i < layoutDescNumElements; i++)
                    {
                        layoutDescCopy->at(i).SemanticName = layoutDescSemanticNamesCopy->at(i).c_str();
                    }
                }

                CreateInputLayout(
                    bytecode->Data,
                    bytecode->Length,
                    layoutDesc == nullptr ? nullptr : layoutDescCopy->data(),
                    layoutDescNumElements,
                    layout
                );

                SetDebugName(*layout, filename);
            }
        });
}

void BasicLoader::LoadShader(
    _In_ Platform::String^ filename,
    _Out_ ID3D11PixelShader** shader
)
{
    Platform::Array<byte>^ bytecode = m_basicReaderWriter->ReadData(filename);

    DX::ThrowIfFailed(
        m_d3dDevice->CreatePixelShader(
            bytecode->Data,
            bytecode->Length,
            nullptr,
            shader
        )
    );

    SetDebugName(*shader, filename);
}

task<void> BasicLoader::LoadShaderAsync(
    _In_ Platform::String^ filename,
    _Out_ ID3D11PixelShader** shader
)
{
    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ bytecode)
        {
            DX::ThrowIfFailed(
                m_d3dDevice->CreatePixelShader(
                    bytecode->Data,
                    bytecode->Length,
                    nullptr,
                    shader
                )
            );

            SetDebugName(*shader, filename);
        });
}

void BasicLoader::LoadShader(
    _In_ Platform::String^ filename,
    _Out_ ID3D11ComputeShader** shader
)
{
    Platform::Array<byte>^ bytecode = m_basicReaderWriter->ReadData(filename);

    DX::ThrowIfFailed(
        m_d3dDevice->CreateComputeShader(
            bytecode->Data,
            bytecode->Length,
            nullptr,
            shader
        )
    );

    SetDebugName(*shader, filename);
}

task<void> BasicLoader::LoadShaderAsync(
    _In_ Platform::String^ filename,
    _Out_ ID3D11ComputeShader** shader
)
{
    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ bytecode)
        {
            DX::ThrowIfFailed(
                m_d3dDevice->CreateComputeShader(
                    bytecode->Data,
                    bytecode->Length,
                    nullptr,
                    shader
                )
            );

            SetDebugName(*shader, filename);
        });
}

void BasicLoader::LoadShader(
    _In_ Platform::String^ filename,
    _Out_ ID3D11GeometryShader** shader
)
{
    Platform::Array<byte>^ bytecode = m_basicReaderWriter->ReadData(filename);

    DX::ThrowIfFailed(
        m_d3dDevice->CreateGeometryShader(
            bytecode->Data,
            bytecode->Length,
            nullptr,
            shader
        )
    );

    SetDebugName(*shader, filename);
}

task<void> BasicLoader::LoadShaderAsync(
    _In_ Platform::String^ filename,
    _Out_ ID3D11GeometryShader** shader
)
{
    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ bytecode)
        {
            DX::ThrowIfFailed(
                m_d3dDevice->CreateGeometryShader(
                    bytecode->Data,
                    bytecode->Length,
                    nullptr,
                    shader
                )
            );

            SetDebugName(*shader, filename);
        });
}

void BasicLoader::LoadShader(
    _In_ Platform::String^ filename,
    _In_reads_opt_(numEntries) const D3D11_SO_DECLARATION_ENTRY* streamOutDeclaration,
    _In_ uint32 numEntries,
    _In_reads_opt_(numStrides) const uint32* bufferStrides,
    _In_ uint32 numStrides,
    _In_ uint32 rasterizedStream,
    _Out_ ID3D11GeometryShader** shader
)
{
    Platform::Array<byte>^ bytecode = m_basicReaderWriter->ReadData(filename);

    DX::ThrowIfFailed(
        m_d3dDevice->CreateGeometryShaderWithStreamOutput(
            bytecode->Data,
            bytecode->Length,
            streamOutDeclaration,
            numEntries,
            bufferStrides,
            numStrides,
            rasterizedStream,
            nullptr,
            shader
        )
    );

    SetDebugName(*shader, filename);
}

task<void> BasicLoader::LoadShaderAsync(
    _In_ Platform::String^ filename,
    _In_reads_opt_(numEntries) const D3D11_SO_DECLARATION_ENTRY* streamOutDeclaration,
    _In_ uint32 numEntries,
    _In_reads_opt_(numStrides) const uint32* bufferStrides,
    _In_ uint32 numStrides,
    _In_ uint32 rasterizedStream,
    _Out_ ID3D11GeometryShader** shader
)
{
    // This method assumes that the lifetime of input arguments may be shorter
    // than the duration of this task.  In order to ensure accurate results, a
    // copy of all arguments passed by pointer must be made.  The method then
    // ensures that the lifetime of the copied data exceeds that of the task.

    // Create copies of the streamOutDeclaration array as well as the SemanticName
    // strings, both of which are pointers to data whose lifetimes may be shorter
    // than that of this method's task.
    shared_ptr<vector<D3D11_SO_DECLARATION_ENTRY>> streamOutDeclarationCopy;
    shared_ptr<vector<string>> streamOutDeclarationSemanticNamesCopy;
    if (streamOutDeclaration != nullptr)
    {
        streamOutDeclarationCopy.reset(
            new vector<D3D11_SO_DECLARATION_ENTRY>(
                streamOutDeclaration,
                streamOutDeclaration + numEntries
                )
        );

        streamOutDeclarationSemanticNamesCopy.reset(
            new vector<string>(numEntries)
        );

        for (uint32 i = 0; i < numEntries; i++)
        {
            streamOutDeclarationSemanticNamesCopy->at(i).assign(streamOutDeclaration[i].SemanticName);
        }
    }

    // Create a copy of the bufferStrides array, which is a pointer to data
    // whose lifetime may be shorter than that of this method's task.
    shared_ptr<vector<uint32>> bufferStridesCopy;
    if (bufferStrides != nullptr)
    {
        bufferStridesCopy.reset(
            new vector<uint32>(
                bufferStrides,
                bufferStrides + numStrides
                )
        );
    }

    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ bytecode)
        {
            if (streamOutDeclaration != nullptr)
            {
                // Reassign the SemanticName elements of the streamOutDeclaration array copy to
                // point to the corresponding copied strings. Performing the assignment inside the
                // lambda body ensures that the lambda will take a reference to the shared_ptr
                // that holds the data.  This will guarantee that the data is still valid when
                // CreateGeometryShaderWithStreamOutput is called.
                for (uint32 i = 0; i < numEntries; i++)
                {
                    streamOutDeclarationCopy->at(i).SemanticName = streamOutDeclarationSemanticNamesCopy->at(i).c_str();
                }
            }

            DX::ThrowIfFailed(
                m_d3dDevice->CreateGeometryShaderWithStreamOutput(
                    bytecode->Data,
                    bytecode->Length,
                    streamOutDeclaration == nullptr ? nullptr : streamOutDeclarationCopy->data(),
                    numEntries,
                    bufferStrides == nullptr ? nullptr : bufferStridesCopy->data(),
                    numStrides,
                    rasterizedStream,
                    nullptr,
                    shader
                )
            );

            SetDebugName(*shader, filename);
        });
}

void BasicLoader::LoadShader(
    _In_ Platform::String^ filename,
    _Out_ ID3D11HullShader** shader
)
{
    Platform::Array<byte>^ bytecode = m_basicReaderWriter->ReadData(filename);

    DX::ThrowIfFailed(
        m_d3dDevice->CreateHullShader(
            bytecode->Data,
            bytecode->Length,
            nullptr,
            shader
        )
    );

    SetDebugName(*shader, filename);
}

task<void> BasicLoader::LoadShaderAsync(
    _In_ Platform::String^ filename,
    _Out_ ID3D11HullShader** shader
)
{
    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ bytecode)
        {
            DX::ThrowIfFailed(
                m_d3dDevice->CreateHullShader(
                    bytecode->Data,
                    bytecode->Length,
                    nullptr,
                    shader
                )
            );

            SetDebugName(*shader, filename);
        });
}

void BasicLoader::LoadShader(
    _In_ Platform::String^ filename,
    _Out_ ID3D11DomainShader** shader
)
{
    Platform::Array<byte>^ bytecode = m_basicReaderWriter->ReadData(filename);

    DX::ThrowIfFailed(
        m_d3dDevice->CreateDomainShader(
            bytecode->Data,
            bytecode->Length,
            nullptr,
            shader
        )
    );

    SetDebugName(*shader, filename);
}

task<void> BasicLoader::LoadShaderAsync(
    _In_ Platform::String^ filename,
    _Out_ ID3D11DomainShader** shader
)
{
    return m_basicReaderWriter->ReadDataAsync(filename).then([=](const Platform::Array<byte>^ bytecode)
        {
            DX::ThrowIfFailed(
                m_d3dDevice->CreateDomainShader(
                    bytecode->Data,
                    bytecode->Length,
                    nullptr,
                    shader
                )
            );

            SetDebugName(*shader, filename);
        });
}

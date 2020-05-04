#include "stdafx.h"
#include "D3D12HelloWindow.h"

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

D3D12HelloWindow::D3D12HelloWindow(UINT width, UINT height, std::wstring name,UINT frameCount):
    DXSample(width,height,name,frameCount)
{

}

void D3D12HelloWindow::OnInit()
{
    assert(DXSample::Initialize());

    // Reset the command list to prepare for initialization
    // commands.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
    
    BuildConstantDescriptorHeaps();
    BuildConstantBuffers();
    BuildRootSignature();
    BuildShaderAndInputLayout();
    BuildOwnGeometry();
    BuildPSO();

    // Execute the initialization commands.
    ThrowIfFailed(m_commandList->Close());
}


// Update frame-based values.
void D3D12HelloWindow::OnUpdate()
{
    // Convert Spherical to Cartesian coordinates.
    float x = m_radius * sinf(m_phi) * cosf(m_theta);
    float y = m_radius * cosf(m_phi);
    float z = m_radius * sinf(m_phi) * sinf(m_theta);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_viewMatrix = XMMatrixLookAtLH(pos, target, up);
    XMMATRIX worldViewProj = m_worldMatrix * m_viewMatrix * m_projMatrix;

    // Update the constant buffer with the latest worldViewProj
    ObjectConstants objConstants;
    objConstants.worldViewProj = XMMatrixTranspose(worldViewProj);
    m_objectConstantBuffer->CopyData(0, objConstants);
}

// Render the scene.
void D3D12HelloWindow::OnRender()
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists 
    // have finished execution on the GPU.
    WaitForGPU();
    ThrowIfFailed(m_commandAllocator->Reset());

    // A command list can be reset after it has been added to
    // the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    m_commandList->RSSetViewports(1, &m_screenViewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate a state transition on resource usage.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    m_commandList->ClearRenderTargetView(
        GetCurrentBackBufferView(),
        Colors::SteelBlue,
        0,
        nullptr
    );
    m_commandList->ClearDepthStencilView(
        GetDepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,0,0,nullptr
    );

    // Specify the buffers we are going to render to.
    m_commandList->OMSetRenderTargets(1, 
        &GetCurrentBackBufferView(), false, 
        &GetDepthStencilView()
    );

    ID3D12DescriptorHeap* descriptorHeap[] = { m_cbvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);

    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    m_commandList->IASetVertexBuffers(0, 1, &m_geometry->VertexBufferView());
    m_commandList->IASetIndexBuffer(&m_geometry->IndexBufferView());
    m_commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

    m_commandList->DrawIndexedInstanced(
        m_geometry->DrawArgs["box"].IndexCount,
        1, 0, 0, 0
    );

    // Indicate a state transition on the resource usage.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(m_commandList->Close());

    // Add command list to the queue for execution.
    ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    // Swap the back and front buffers.
    ThrowIfFailed(m_swapChain->Present(1, 0));
    MoveToNextFrame();
}

void D3D12HelloWindow::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources
    // that are about to be cleaned up by the destructor.
    WaitForGPU();
    CloseHandle(m_fenceEvent);
}

void D3D12HelloWindow::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should
    // use fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular
    // command list, that command list can then be reset at any time
    // and must be before re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),m_frameIndex,m_rtvDescriptorSize);

    // Record commands.
    const float clearColor[] = { 0.0f,0.2f,0.4f,1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    
    ThrowIfFailed(m_commandList->Close());
}



void D3D12HelloWindow::BuildConstantDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void D3D12HelloWindow::BuildConstantBuffers()
{
    m_objectConstantBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(m_device.Get(), 1, true);

    UINT objectConstantBufferByteSize = CalculateConstantBufferByteSize(sizeof(ObjectConstants));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_objectConstantBuffer->Resource()->GetGPUVirtualAddress();

    // Offset to the ith object constant buffer in the buffer.
    int constantBufferIndex = 0;
    cbAddress += constantBufferIndex * objectConstantBufferByteSize;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbAddress;
    cbvDesc.SizeInBytes = CalculateConstantBufferByteSize(sizeof(ObjectConstants));

    // Bind constant buffer view to a subregion of the buffer
    m_device->CreateConstantBufferView(
        &cbvDesc,
        m_cbvHeap->GetCPUDescriptorHandleForHeapStart()
    );
}

void D3D12HelloWindow::BuildRootSignature()
{
    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[1];

    // Create a single descriptor table of CBVs.
    CD3DX12_DESCRIPTOR_RANGE cbvTable;
    cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // Create a root signature with a single slot which points to a descriptor range consisting 
    // of a single constant buffer.
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)
    ));
}

void D3D12HelloWindow::BuildShaderAndInputLayout()
{
#if defined (_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif // (_DEBUG)
    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &m_vsByteCode, nullptr));
    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &m_psByteCode, nullptr));
    m_inputLayout.push_back(InitInputLayoutDescription((LPSTR)"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0));
    m_inputLayout.push_back(InitInputLayoutDescription((LPSTR)"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0));
}

void D3D12HelloWindow::BuildOwnGeometry()
{
    std::array<Vertex, 8> vertices =
    {
        Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
        Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
    };
    std::array<std::uint16_t, 36> indices =
    {
        // front face
        0,1,2,
        0,2,3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    m_geometry = std::make_unique<MeshGeometry>();
    m_geometry->Name = "boxGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_geometry->VertexBufferCPU));
    CopyMemory(m_geometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_geometry->IndexBufferCPU));
    CopyMemory(m_geometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    m_geometry->VertexBufferGPU = CreateDefaultBuffer(m_device.Get(), m_commandList.Get(), vertices.data(),
        vbByteSize, m_geometry->VertexBufferUploadBuffer);

    m_geometry->IndexBufferGPU = CreateDefaultBuffer(m_device.Get(), m_commandList.Get(), indices.data(), ibByteSize,
        m_geometry->IndexBufferUploadBuffer);

    m_geometry->VertexByteStride = sizeof(Vertex);
    m_geometry->VertexBufferByteSize = vbByteSize;
    m_geometry->IndexFormat = DXGI_FORMAT_R16_UINT;
    m_geometry->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.IndexCount = (UINT)indices.size();
    submesh.StartIndexLoacation = 0;
    submesh.BaseVertexLoction = 0;
    
    m_geometry->DrawArgs["box"] = submesh;
}

void D3D12HelloWindow::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { m_inputLayout.data(),(UINT)m_inputLayout.size() };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
        m_vsByteCode->GetBufferSize()
    };
    psoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
        m_psByteCode->GetBufferSize()
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_backBufferFormat;
    psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = m_depthStencilFormat;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void D3D12HelloWindow::OnResize()
{
    DXSample::OnResize();

    // The window resized, so update the aspect ratio and 
    // recompute the projection matrix.
    m_projMatrix = XMMatrixPerspectiveFovLH(0.25f * XM_PI, m_aspectRatio, 1.0f, 1000.0f);
}

void D3D12HelloWindow::OnMouseDown(WPARAM btnState, int x, int y)
{
    m_lastMousePos.x = x;
    m_lastMousePos.y = y;
}

void D3D12HelloWindow::OnMouseUp(WPARAM btnState, int x, int y)
{

}

void D3D12HelloWindow::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_lastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_lastMousePos.y));

        // Update angles base on input to orbit camera around object.
        m_theta += dx;
        m_phi += dy;

        // Restrict the angle m_phi
        if (m_phi <= 0.1f)
        {
            m_phi = 0.1f;
        }
        if (m_phi >= XM_PI - 0.1f)
        {
            m_phi = XM_PI - 0.1f;
        }
    }
    else if((btnState&MK_RBUTTON)!=0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.005f * static_cast<float>(x - m_lastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - m_lastMousePos.y);

        // Update the camera radius based on input.
        m_radius += dx - dy;

        // Restrict the radius.
        if (m_radius <= 3.0f)
        {
            m_radius = 3.0f;
        }
        if (m_radius >= 15.0f)
        {
            m_radius = 15.0f;
        }
    }
    m_lastMousePos.x = x;
    m_lastMousePos.y = y;
}
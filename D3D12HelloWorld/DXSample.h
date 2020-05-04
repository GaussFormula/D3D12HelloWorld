#pragma once
#include "DXSampleHelper.h"
#include "Win32Application.h"
#include "GameTimer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class DXSample
{
public:
    DXSample(UINT width, UINT height, std::wstring name,UINT frameCount=2);
    virtual ~DXSample();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnDestroy() = 0;

    DXSample(const DXSample& rhs) = delete;
    DXSample& operator=(const DXSample& rhs) = delete;

    //Samples override the event handlers to handle specific messages
    virtual void OnKeyDown(UINT8) {}
    virtual void OnKeyUp(UINT8 wParam) 
    {
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        else if(wParam==VK_F2)
        {
            m_4xMsaaState = !m_4xMsaaState;
        }
    }
    virtual void OnResize();

    // Convenience overrides for handling mouse input.
    virtual void OnMouseDown(WPARAM btnState,int x,int y){}
    virtual void OnMouseUp(WPARAM btnState,int x,int y){}
    virtual void OnMouseMove(WPARAM btnState,int x,int y){}

    //Accessors
    UINT GetWidth()const { return m_width; }
    UINT GetHeight()const { return m_height; }
    const WCHAR* GetTitle() const { return m_title.c_str(); }
    ID3D12Device* GetDevice()const;
    bool GetWindowMinimized()const;
    bool GetWindowMaximized()const;
    bool GetWindowResizing()const;
    bool GetProgramPauseState()const;

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);
    void StopTimer();
    void StartTimer();
    void TickTimer();
    void ResetTimer();
    void SetProgramPauseState(bool);
    void SetWindowMinimizedState(bool);
    void SetWindowMaximizedState(bool);
    void SetWindowResizingState(bool);
    void SetWindowFullscreenState(bool);
    void SetWindowWidth(int);
    void SetWindowHeight(int);

    void CalculateFrameStats();

    int Run();

protected:
    std::wstring GetAssetFullPath(LPCWSTR assetName);
    void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
    void SetCustomWindowText(LPCWSTR text);
    virtual void CreateFactoryDeviceAdapter();
    virtual void CreateFenceObjects();
    void InitDescriptorSize();
    virtual void CheckFeatureSupport();
    virtual void CreateCommandQueue();
    virtual void CreateCommandAllocator();
    virtual void CreateCommandList();
    void CreateCommandObjects();
    virtual void CreateSwapChain();
    virtual void CreateRtvAndDsvDescriptorHeaps();
    bool Get4xMsaaState()const;
    virtual void Set4xMsaaState(bool);
    D3D12_INPUT_ELEMENT_DESC InitInputLayoutDescription(
        LPSTR SemanticName,
        UINT SemanticIndex,
        DXGI_FORMAT Format,
        UINT InputSlot,
        UINT AlignedByteOffset,
        D3D12_INPUT_CLASSIFICATION InputSlotClass,
        UINT InstanceDataStepRate
        );
    bool InitializeDirect3D();
    bool Initialize();
    ID3D12Resource* GetCurrentBackBuffer()const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView()const;

    void WaitForGPU();
    void MoveToNextFrame();

    virtual void BuildConstantDescriptorHeaps() = 0;
    virtual void BuildConstantBuffers() = 0;
    virtual void BuildRootSignature() = 0;
    virtual void BuildShaderAndInputLayout() = 0;
    virtual void BuildOwnGeometry() = 0;
    virtual void BuildPSO() = 0;
    
    bool m_programPaused = false;   // is the application paused?
    bool m_windowMinimized = false; // is the application minimized?
    bool m_windowMaximized = false; // is the application maximized?
    bool m_windowResizing = false;  // are the resize bars being dragged?
    bool m_windowFullscreenState = false;   // fullscreen enabled

    //Adapter info.
    bool m_useWarpDevice;

    // Set True to use 4X MSAA. The default is false.
    bool m_4xMsaaState = false;  // Is 4xMsaa Enabled ?
    UINT m_4xMsaaQuality = 0;    // Quality level of 4X MSAA

    // Used to keep track of the delta-time and game time
    GameTimer mTimer;

    // Derived class should set these in derived constructor to customize starting values.
    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE;
    D3D12_COMMAND_LIST_TYPE m_commandListType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    
    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;


    // Pipeline Objects.
    ComPtr<ID3D12CommandQueue>              m_commandQueue;
    ComPtr<ID3D12CommandAllocator>          m_commandAllocator;
    ComPtr<ID3D12GraphicsCommandList>       m_commandList;
    ComPtr<IDXGISwapChain3>                 m_swapChain;
    ComPtr<IDXGIFactory4>                   m_factory;
    ComPtr<ID3D12RootSignature>             m_rootSignature;
    ComPtr<ID3D12DescriptorHeap>            m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap>            m_srvHeap;
    ComPtr<ID3D12DescriptorHeap>            m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap>            m_cbvHeap;
    ComPtr<ID3D12Device>                    m_device;
    ComPtr<IDXGIAdapter1>                   m_adapter;
    UINT                                    m_rtvDescriptorSize = 0;
    UINT                                    m_dsvDescriptorSize = 0;
    UINT                                    m_cbvSrvUavDescriptorSize = 0;

    // App resources.
    ComPtr<ID3D12Resource>                  m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                m_vertexBufferView;

    UINT                                    m_frameCount = 2;
    //ComPtr<ID3D12Resource>                m_renderTarget[FrameCount];
    std::vector<ComPtr<ID3D12Resource>>     m_renderTargets;
    ComPtr<ID3D12Resource>                  m_depthStencilBuffer;

    // Synchronization objects.
    ComPtr<ID3D12Fence>                     m_fence;
    UINT                                    m_frameIndex = 0;
    HANDLE                                  m_fenceEvent;
    std::vector<UINT64>                     m_fenceValue;

    D3D12_VIEWPORT                          m_screenViewport;
    D3D12_RECT                              m_scissorRect;
protected:
    //Root assets path.
    std::wstring m_assetsPath;

    //Window title.
    std::wstring m_title;
};

struct SubmeshGeometry
{
    UINT IndexCount = 0;
    UINT StartIndexLoacation = 0;
    int BaseVertexLoction = 0;

    // Bounding box of the geometry defined by the submesh.
    DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
    // Give it a name so we can look it up by name.
    std::string Name;

    // System memory copies.
    // Use blob because the vertex/index format can be generic.
    // It is up to the client to cast appropriately.
    ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
    ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

    ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

    ComPtr<ID3D12Resource> VertexBufferUploadBuffer = nullptr;
    ComPtr<ID3D12Resource> IndexBufferUploadBuffer = nullptr;

    // Data about the buffers.
    UINT VertexByteStride = 0;
    UINT VertexBufferByteSize = 0;
    DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
    UINT IndexBufferByteSize = 0;

    // A MeshGeometry may store multiple geometries in one vertex/index buffer.
    // Use this container to define the Submesh geometries so we can draw
    // the Submeshes individually.
    std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
        vbv.StrideInBytes = VertexByteStride;
        vbv.SizeInBytes = VertexBufferByteSize;

        return vbv;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
        ibv.Format = IndexFormat;
        ibv.SizeInBytes = IndexBufferByteSize;

        return ibv;
    }

    // We can free this memory after we finish upload to the GPU
    void DisposeUploadBuffer()
    {
        VertexBufferUploadBuffer = nullptr;
        IndexBufferUploadBuffer = nullptr;
    }
};
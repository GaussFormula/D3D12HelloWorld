#pragma once
#include "DXSampleHelper.h"
#include "Win32Application.h"
#include "GameTimer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class DXSample
{
public:
    DXSample(UINT width, UINT height, std::wstring name);
    virtual ~DXSample();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnDestroy() = 0;

    DXSample(const DXSample& rhs) = delete;
    DXSample& operator=(const DXSample& rhs) = delete;

    //Samples override the event handlers to handle specific messages
    virtual void OnKeyDown(UINT8) {}
    virtual void OnKeyUp(UINT8) {}

    //Accessors
    UINT GetWidth()const { return m_width; }
    UINT GetHeight()const { return m_height; }
    const WCHAR* GetTitle() const { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
    std::wstring GetAssetFullPath(LPCWSTR assetName);
    void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
    void SetCustomWindowText(LPCWSTR text);
    virtual void CreateFactoryDeviceAdapter();
    virtual void CreateFence();
    void InitDescriptorSize();
    virtual void CheckFeatureSupport();
    virtual void CreateCommandQueue();
    virtual void CreateCommandAllocator();
    virtual void CreateCommandList();
    void CreateCommandObjects();
    virtual void CreateSwapChain();
    

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
    
    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;


    // Pipeline Objects.
    ComPtr<ID3D12CommandQueue>              m_commandQueue;
    ComPtr<ID3D12CommandAllocator>          m_commandAllcator;
    ComPtr<ID3D12GraphicsCommandList>       m_commandList;
    ComPtr<IDXGISwapChain3>                 m_swapChain;
    ComPtr<IDXGIFactory4>                   m_factory;
    ComPtr<ID3D12RootSignature>             m_rootSignature;
    ComPtr<ID3D12DescriptorHeap>            m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap>            m_srvHeap;
    ComPtr<ID3D12DescriptorHeap>            m_dsvHeap;
    ComPtr<ID3D12PipelineState>             m_pipelineState;
    UINT                                    m_rtvDescriptorSize = 0;
    UINT                                    m_dsvDescriptorSize = 0;
    UINT                                    m_cbvSrvUavDescriptorSize = 0;
    ComPtr<ID3D12Device>                    m_device;
    ComPtr<IDXGIAdapter1>                   m_adapter;

    // App resources.
    ComPtr<ID3D12Resource>                  m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                m_vertexBufferView;

    static const UINT                       FrameCount = 2;
    ComPtr<ID3D12Resource>                  m_renderTarget[FrameCount];
    ComPtr<ID3D12Resource>                  m_depthStencilBuffer;

    // Synchronization objects.
    ComPtr<ID3D12Fence>                     m_fence;
    UINT                                    m_frameIndex = 0;
    HANDLE                                  m_fenceEvent;
    UINT64                                  m_fenceValue = 0;

    D3D12_VIEWPORT                          m_screenViewport;
    D3D12_RECT                              m_scissorRect;
protected:
    //Root assets path.
    std::wstring m_assetsPath;

    //Window title.
    std::wstring m_title;
};
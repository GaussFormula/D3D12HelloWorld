#pragma once
#include "DXSample.h"
#include "UploadBuffer.h"

using Microsoft::WRL::ComPtr;

struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj=
        DirectX::XMFLOAT4X4 (
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
};

class D3D12HelloWindow :public DXSample
{
public:
    D3D12HelloWindow(UINT width, UINT height, std::wstring name,UINT frameCount=2);
    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    

private:
    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;
    virtual void BuildConstantDescriptorHeaps() override;
    virtual void BuildConstantBuffers()override;
    virtual void BuildRootSignature()override;
    virtual void BuildShaderAndInputLayout()override;
    virtual void BuildOwnGeometry()override;
    virtual void BuildPSO()override;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();

    virtual void OnResize() override;

    std::unique_ptr <UploadBuffer<ObjectConstants>> m_objectConstantBuffer = nullptr;
    ComPtr<ID3DBlob> m_vsByteCode = nullptr;
    ComPtr<ID3DBlob> m_psByteCode = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    std::unique_ptr<MeshGeometry> m_geometry = nullptr;

    ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;

    DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX m_viewMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX m_projMatrix = DirectX::XMMatrixIdentity();

    float m_theta = 1.5f * XM_PI;
    float m_phi = XM_PIDIV4;
    float m_radius = 5.0f;

    POINT m_lastMousePos;
};
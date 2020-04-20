#pragma once
#include "DXSample.h"

using Microsoft::WRL::ComPtr;

class D3D12HelloWindow :public DXSample
{
public:
    D3D12HelloWindow(UINT width, UINT height, std::wstring name,UINT frameCount=2);

    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;
    virtual void BuildConstantDescriptorHeaps() override;
    virtual void BuildConstantBuffers()override;
    virtual void BuildRootSignature()override;
    virtual void BuildShaderAndInputLayoutoverride();
    virtual void BuildOwnGeometry()override;
    virtual void BuildPS0()override;

private:
    

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};
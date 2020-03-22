#pragma once
#include "DXSample.h"

using Microsoft::WRL::ComPtr;

class D3D12HelloWindow :public DXSample
{
public:
    D3D12HelloWindow(UINT width, UINT height, std::wstring name,UINT frameCount=2);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    virtual void BuildDescriptorHeaps();
    virtual void BuildConstantBuffers();
    virtual void BuildRootSignature();
    virtual void BuildShaderAndInputLayout();
    virtual void BuildOwnGeometry();
    virtual void BuildPS0();

private:
    

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};
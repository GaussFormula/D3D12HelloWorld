//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "Win32Application.h"
#include <windowsx.h>

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow)
{
    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pSample->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXSampleClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        pSample->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        pSample);

    // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
    pSample->OnInit();

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            pSample->TickTimer();
            if (!pSample->GetProgramPauseState())
            {
                pSample->CalculateFrameStats();

            }
        }
    }

    pSample->OnDestroy();

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        if (LODWORD(wParam) == WA_INACTIVE)
        {
            pSample->SetProgramPauseState(true);
            pSample->StopTimer();
        }
        else
        {
            pSample->SetProgramPauseState(false);
            pSample->StartTimer();
        }
    }
    return 0;

    // WM_SIZE is sent when the user resizes the window.
    case WM_SIZE:
        pSample->SetWindowWidth(LOWORD(lParam));
        pSample->SetWindowHeight(HIWORD(lParam));
        if (pSample->GetDevice())
        {
            if (wParam == SIZE_MINIMIZED)
            {
                pSample->SetProgramPauseState(true);
                pSample->SetWindowMinimizedState(true);
                pSample->SetWindowMaximizedState(false);
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                pSample->SetProgramPauseState(false);
                pSample->SetWindowMinimizedState(false);
                pSample->SetWindowMaximizedState(true);
                pSample->OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {
                // Is Restoring from minimized state?
                if (pSample->GetWindowMinimized())
                {
                    pSample->SetProgramPauseState(false);
                    pSample->SetWindowMinimizedState(false);
                    pSample->OnResize();
                }

                // Is Restoring from maximized state?
                else if (pSample->GetWindowMaximized())
                {
                    pSample->SetProgramPauseState(false);
                    pSample->SetWindowMaximizedState(false);
                    pSample->OnResize();
                }

                else if (pSample->GetWindowResizing())
                {
                    // If user is dragging the resize bars, we do not resize 
                    // the buffers here because as the user continuously
                    // drags the resize bars, a stream of WM_SIZE messages are 
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars. So instead, we reset after the user is
                    // done resizing the window and releases the resize bars, which
                    // sends a WM_EXITSIZEMOVE message.
                }
                else // API call such as SetWindowPos or m_swapChain->SetFullscreenState.
                {
                    pSample->OnResize();
                }
            }
        }
        return 0;


        // WM_ENTERSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        pSample->SetProgramPauseState(true);
        pSample->SetWindowResizingState(true);
        pSample->StopTimer();
        return 0;

        //  WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        //  Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        pSample->SetProgramPauseState(false);
        pSample->SetWindowResizingState(false);
        pSample->StartTimer();
        return 0;

        // The WM_MENUCHAR message is sent when a menu is active and the user presses
        // a key that does not correspond to any mnemonic or accelerator key.
    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        pSample->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        SetCapture(hWnd);
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        pSample->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        ReleaseCapture();
        return 0;
    case WM_KEYDOWN:
        if (pSample)
        {
            pSample->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:
        if (pSample)
        {
            pSample->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_PAINT:
        if (pSample)
        {
            pSample->OnUpdate();
            pSample->OnRender();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

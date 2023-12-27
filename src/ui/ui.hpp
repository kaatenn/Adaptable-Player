//
// Created by 86137 on 2023/12/22.
//

#ifndef ADAPTABLE_UPLOADER_UI_HPP
#define ADAPTABLE_UPLOADER_UI_HPP

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include "vector"
#include "string"
#include "Data/DataWrapper.hpp"
#include "kcp/Connection.hpp"

using std::vector, std::string;

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER

#include <dxgidebug.h>

#include <utility>

#pragma comment(lib, "dxguid.lib")
#endif

struct FrameContext {
    ID3D12CommandAllocator *command_allocator;
    UINT64 fence_value;
};

struct GuiData {
    vector<string> file_list;
    int selected_file_index = -1;
    bool could_play = false;
    enum playing_state {
        playing,
        paused,
        stopped
    } state = stopped;
};

static GuiData gui_data;

// DX 12 global variables
static int const NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext g_frame_context[NUM_FRAMES_IN_FLIGHT] = {};
static UINT g_frame_index = 0;

static int const NUM_BACK_BUFFERS = 3;
static ID3D12Device *g_pd3d_device = nullptr;
static ID3D12DescriptorHeap *g_pd3d_rtv_desc_heap = nullptr;
static ID3D12DescriptorHeap *g_pd3d_srv_desc_heap = nullptr;
static ID3D12CommandQueue *g_pd3d_command_queue = nullptr;
static ID3D12GraphicsCommandList *g_pd3d_command_list = nullptr;
static ID3D12Fence *g_fence = nullptr;
static HANDLE g_fence_event = nullptr;
static UINT64 g_fence_last_signaled_value = 0;
static IDXGISwapChain3 *g_pswap_chain = nullptr;
static HANDLE g_hswap_chain_waitable_object = nullptr;
static ID3D12Resource *g_main_render_target_resource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE g_main_render_target_descriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of helper functions
bool create_deviceD3D(HWND hWnd);

void cleanup_device_D3D();

void create_render_target();

void cleanup_render_target();

void wait_for_last_submitted_frame();

FrameContext *wait_for_next_frame_resources();

LRESULT WINAPI wnd_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param);

// The key is the url, the value is the callback function
static map<string, std::function<void(const std::string&)>> receive_buffer_map;

void send_request(const Connection& connection, DataWrapper *data_wrapper, std::function<void(const std::string&)> callback = [](const std::string&){}) {
    data_wrapper->send_queue.push(connection.to_json());
    receive_buffer_map[connection.get_url()] = std::move(callback);
}

// child window
void render_player_window(const char *title, GuiData::playing_state &state, DataWrapper *data_wrapper) {
    ImGui::Begin(title);
    switch (state) {
        case GuiData::stopped:
            if (ImGui::Button("Play")) {
                // send play request
                string url = "file";
                Connection request(url, {title});
                state = GuiData::playing;
                send_request(request, data_wrapper, [](const string& json) {
                    // TODO: define the callback function
                    gui_data.could_play = true;
                });
                // Disable playing until the file received
                gui_data.could_play = false;
            }
            break;
        case GuiData::playing:
            if (ImGui::Button("Pause")) {
                // send pause request
                // TODO: Pause the music
                state = GuiData::paused;
            }
            // TODO: Add a progress bar
            // We do not need to add "stop" button, which will lead the ui and the logic to a mess.
            break;
        case GuiData::paused:
            if (ImGui::Button("Resume")) {
                // resume the music
                state = GuiData::playing;
            }
            break;
    }
    ImGui::End();

    // TODO: manage playing state
}

// Main code
int render(DataWrapper *data_wrapper) {
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr,
                      nullptr, L"ImGui Example", nullptr};
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX12 Example", WS_OVERLAPPEDWINDOW, 100, 100,
                                1280,
                                800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!create_deviceD3D(hwnd)) {
        cleanup_device_D3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // We do not need gamepad controls

    // Setup Dear ImGui
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_pd3d_device, NUM_FRAMES_IN_FLIGHT,
                        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3d_srv_desc_heap,
                        g_pd3d_srv_desc_heap->GetCPUDescriptorHandleForHeapStart(),
                        g_pd3d_srv_desc_heap->GetGPUDescriptorHandleForHeapStart());

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // data_async init
    DataWrapper *data_async = data_wrapper;

    // init music list
    // do not open the ui before init so use method wait
    string list_json = data_async->recv_queue.wait_and_pop().value();
    Connection connection_music_list = Connection::from_json(list_json);
    while (connection_music_list.get_url() != "music_list") {}
    // I can't image what will happen if the server send a res without url "music_list"
    // If this happens, the client will crash.
    // If this happens, I will add a check in the future.
    gui_data.file_list = connection_music_list.get_params();

    // Main loop
    bool done = false;
    while (true) {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Manage the data in the DataWrapper
        {
            optional<string> res_str = data_async->recv_queue.try_pop();
            while ((res_str = data_async->recv_queue.try_pop()) != nullopt){
                // if is waiting for a file, then check if the file is received(The procedure of receiving a file is
                // finished in asio thread)
                std::cout << "receive: " << res_str.value() << std::endl;
                Connection res = Connection::from_json(res_str.value());
                if (receive_buffer_map.count(res.get_url()) != 0) {
                    receive_buffer_map[res.get_url()](res_str.value());
                }
            }
        }

        // Show the main ui -- selector
        {
            ImGui::Begin("Selector");
            for (int i = 0; i < gui_data.file_list.size(); ++i) {
                if (ImGui::Selectable(gui_data.file_list[i].c_str(), gui_data.selected_file_index == i)) {
                    gui_data.selected_file_index = i;
                    gui_data.state = GuiData::stopped;
                }
            }
            ImGui::End();
        }

        // Show the main ui -- player
        {
            if (gui_data.selected_file_index != -1) {
                string url = gui_data.file_list[gui_data.selected_file_index];
                render_player_window(url.c_str(), gui_data.state, data_async);
            }
        }

        // Rendering
        ImGui::Render();

        FrameContext *frame_ctx = wait_for_next_frame_resources();
        UINT back_buffer_idx = g_pswap_chain->GetCurrentBackBufferIndex();
        frame_ctx->command_allocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_main_render_target_resource[back_buffer_idx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pd3d_command_list->Reset(frame_ctx->command_allocator, nullptr);
        g_pd3d_command_list->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                                                 clear_color.z * clear_color.w, clear_color.w};
        g_pd3d_command_list->ClearRenderTargetView(g_main_render_target_descriptor[back_buffer_idx],
                                                   clear_color_with_alpha, 0, nullptr);
        g_pd3d_command_list->OMSetRenderTargets(1, &g_main_render_target_descriptor[back_buffer_idx], FALSE,
                                                nullptr);
        g_pd3d_command_list->SetDescriptorHeaps(1, &g_pd3d_srv_desc_heap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3d_command_list);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3d_command_list->ResourceBarrier(1, &barrier);
        g_pd3d_command_list->Close();

        g_pd3d_command_queue->ExecuteCommandLists(1, (ID3D12CommandList *const *) &g_pd3d_command_list);

        g_pswap_chain->Present(1, 0); // Present with vsync

        UINT64 fence_value = g_fence_last_signaled_value + 1;
        g_pd3d_command_queue->Signal(g_fence, fence_value);
        g_fence_last_signaled_value = fence_value;
        frame_ctx->fence_value = fence_value;
    }

    wait_for_last_submitted_frame();

    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_device_D3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    // data_async setting
    data_async->should_close.set(true);

    return 0;
}

// Helper functions

bool create_deviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug *pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3d_device)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr) {
        ID3D12InfoQueue *pInfoQueue = nullptr;
        g_pd3d_device->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3d_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3d_rtv_desc_heap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = g_pd3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3d_rtv_desc_heap->GetCPUDescriptorHandleForHeapStart();
        for (auto &i: g_main_render_target_descriptor) {
            i = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_pd3d_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3d_srv_desc_heap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3d_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3d_command_queue)) != S_OK)
            return false;
    }

    for (auto &i: g_frame_context)
        if (g_pd3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                  IID_PPV_ARGS(&i.command_allocator)) != S_OK)
            return false;

    if (g_pd3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frame_context[0].command_allocator,
                                         nullptr, IID_PPV_ARGS(&g_pd3d_command_list)) != S_OK ||
        g_pd3d_command_list->Close() != S_OK)
        return false;

    if (g_pd3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (g_fence_event == nullptr)
        return false;

    {
        IDXGIFactory4 *dxgiFactory = nullptr;
        IDXGISwapChain1 *swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(g_pd3d_command_queue, hWnd, &sd, nullptr, nullptr, &swapChain1) !=
            S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pswap_chain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        g_pswap_chain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hswap_chain_waitable_object = g_pswap_chain->GetFrameLatencyWaitableObject();
    }

    create_render_target();
    return true;
}

void cleanup_device_D3D() {
    cleanup_render_target();
    if (g_pswap_chain) {
        g_pswap_chain->SetFullscreenState(false, nullptr);
        g_pswap_chain->Release();
        g_pswap_chain = nullptr;
    }
    if (g_hswap_chain_waitable_object != nullptr) { CloseHandle(g_hswap_chain_waitable_object); }
    for (auto &i: g_frame_context)
        if (i.command_allocator) {
            i.command_allocator->Release();
            i.command_allocator = nullptr;
        }
    if (g_pd3d_command_queue) {
        g_pd3d_command_queue->Release();
        g_pd3d_command_queue = nullptr;
    }
    if (g_pd3d_command_list) {
        g_pd3d_command_list->Release();
        g_pd3d_command_list = nullptr;
    }
    if (g_pd3d_rtv_desc_heap) {
        g_pd3d_rtv_desc_heap->Release();
        g_pd3d_rtv_desc_heap = nullptr;
    }
    if (g_pd3d_srv_desc_heap) {
        g_pd3d_srv_desc_heap->Release();
        g_pd3d_srv_desc_heap = nullptr;
    }
    if (g_fence) {
        g_fence->Release();
        g_fence = nullptr;
    }
    if (g_fence_event) {
        CloseHandle(g_fence_event);
        g_fence_event = nullptr;
    }
    if (g_pd3d_device) {
        g_pd3d_device->Release();
        g_pd3d_device = nullptr;
    }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1 *pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug)))) {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void create_render_target() {
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        ID3D12Resource *pBackBuffer = nullptr;
        g_pswap_chain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3d_device->CreateRenderTargetView(pBackBuffer, nullptr, g_main_render_target_descriptor[i]);
        g_main_render_target_resource[i] = pBackBuffer;
    }
}

void cleanup_render_target() {
    wait_for_last_submitted_frame();

    for (auto &i: g_main_render_target_resource)
        if (i) {
            i->Release();
            i = nullptr;
        }
}

void wait_for_last_submitted_frame() {
    FrameContext *frameCtx = &g_frame_context[g_frame_index % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->fence_value;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->fence_value = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fence_event);
    WaitForSingleObject(g_fence_event, INFINITE);
}

FrameContext *wait_for_next_frame_resources() {
    UINT nextFrameIndex = g_frame_index + 1;
    g_frame_index = nextFrameIndex;

    HANDLE waitableObjects[] = {g_hswap_chain_waitable_object, nullptr};
    DWORD numWaitableObjects = 1;

    FrameContext *frameCtx = &g_frame_context[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->fence_value;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->fence_value = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fence_event);
        waitableObjects[1] = g_fence_event;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI wnd_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    if (ImGui_ImplWin32_WndProcHandler(h_wnd, msg, w_param, l_param))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (g_pd3d_device != nullptr && w_param != SIZE_MINIMIZED) {
                wait_for_last_submitted_frame();
                cleanup_render_target();
                HRESULT result = g_pswap_chain->ResizeBuffers(0, (UINT) LOWORD(l_param), (UINT) HIWORD(l_param),
                                                              DXGI_FORMAT_UNKNOWN,
                                                              DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
                assert(SUCCEEDED(result) && "Failed to resize swapchain.");
                create_render_target();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((w_param & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return ::DefWindowProcW(h_wnd, msg, w_param, l_param);
}

#endif //ADAPTABLE_UPLOADER_UI_HPP

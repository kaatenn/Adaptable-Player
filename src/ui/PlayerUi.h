//
// Created by 86137 on 2023/12/21.
//

#ifndef ADAPTABLE_UPLOADER_PLAYERUI_H
#define ADAPTABLE_UPLOADER_PLAYERUI_H

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif


class PlayerUi {
private:
    struct FrameContext
    {
        ID3D12CommandAllocator* CommandAllocator;
        UINT64                  FenceValue;
    };

};


#endif //ADAPTABLE_UPLOADER_PLAYERUI_H

#pragma once

#include "framework_vulkan\framework_vulkan.h"

struct sdf_input
{
    f32 Time;
    f32 RenderWidth;
    f32 RenderHeight;
};

struct demo_state
{
    linear_arena Arena;
    linear_arena TempArena;

    f32 TotalProgramTime;
    
    // NOTE: Samplers
    VkSampler PointSampler;
    VkSampler LinearSampler;
    
    camera Camera;

    // NOTE: Render Target Entries
    render_target_entry SwapChainEntry;
    render_target SdfRenderTarget;

    VkDescriptorSetLayout SdfDescLayout;
    VkDescriptorSet SdfDescriptor;
    VkBuffer SdfInputBuffer;
    render_fullscreen_pass SdfPass;
};

global demo_state* DemoState;


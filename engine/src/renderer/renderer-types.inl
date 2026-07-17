#pragma once

#include "defines.hpp"

struct PlatformState;

enum class RendererBackendType { Vulkan, OpenGL, DirectX };

struct RendererBackend {
    PlatformState* platformState;
    u64 FrameNumber;

    bool (*Initialise)(RendererBackend& backend, const char* applicationName,
                       PlatformState& platform);
    void (*Shutdown)(RendererBackend& backend);

    void (*Resized)(RendererBackend& backend, u16 width, u16 height);

    bool (*BeginFrame)(RendererBackend& backend, f32 deltaTime);
    bool (*EndFrame)(RendererBackend& backend, f32 deltaTime);
};

struct RenderPacket {
    f32 deltaTime;
};
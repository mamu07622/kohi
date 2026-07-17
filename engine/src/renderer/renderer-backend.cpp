#include "renderer-backend.hpp"

#include "vulkan/vulkan-backend.h"

bool RendererBackendCreate(RendererBackendType type, PlatformState& state,
                           RendererBackend& rendererBackend) {
    rendererBackend.platformState = &state;

    switch (type) {
        case RendererBackendType::Vulkan: {
            rendererBackend.Initialise = VulkanRendererBackendInitialise;
            rendererBackend.Shutdown = VulkanRendererBackendShutdown;
            rendererBackend.BeginFrame = VulkanRendererBackendBeginFrame;
            rendererBackend.EndFrame = VulkanRendererBackendEndFrame;
            rendererBackend.Resized = VulkanRendererBackendOnResize;

            return true;
        }

        default: {
            return false;
        }
    }

    return false;
}

void RendererBackendDestroy(RendererBackend& rendererBackend) {
    rendererBackend.Initialise = nullptr;
    rendererBackend.Shutdown = nullptr;
    rendererBackend.BeginFrame = nullptr;
    rendererBackend.EndFrame = nullptr;
    rendererBackend.Resized = nullptr;

    return;
}
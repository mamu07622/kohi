#pragma once

#include "renderer/renderer-backend.hpp"

struct PlatformState;

bool VulkanRendererBackendInitialise(RendererBackend& backend,
                                     const char* applicationName,
                                     PlatformState& platform);
void VulkanRendererBackendShutdown(RendererBackend& backend);

void VulkanRendererBackendOnResize(RendererBackend& backend, u16 width,
                                   u16 height);

bool VulkanRendererBackendBeginFrame(RendererBackend& backend, f32 deltaTime);
bool VulkanRendererBackendEndFrame(RendererBackend& backend, f32 deltaTime);

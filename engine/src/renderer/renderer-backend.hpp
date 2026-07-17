#pragma once

#include "renderer-types.inl"

struct PlatformState;

bool RendererBackendCreate(RendererBackendType type, PlatformState& state,
                           RendererBackend& rendererBackend);
void RendererBackendDestroy(RendererBackend& rendererBackend);
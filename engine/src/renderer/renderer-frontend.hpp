#pragma once

#include "renderer-types.inl"

struct StaticMeshData;
struct PlatformState;

bool RendererInitialise(const char* applicationName, PlatformState& platformState);
void RendererShutdown();

void RendererOnResize(u16 width, u16 height);

bool RendererDrawFrame(RenderPacket& packet);
#include "renderer-frontend.hpp"

#include "core/kmemory.hpp"
#include "core/logger.hpp"
#include "renderer-backend.hpp"

struct PlatformState;

// Backend render context.
static RendererBackend* sBackendContext {};

bool RendererInitialise(const char* applicationName,
                        PlatformState& platformState) {
    sBackendContext = reinterpret_cast<RendererBackend*>(
        KAllocate(sizeof(RendererBackend), MemoryTag::RENDERER));

    // TODO: Make this configurable.
    RendererBackendCreate(RendererBackendType::Vulkan, platformState,
                          *sBackendContext);
    sBackendContext->FrameNumber = 0;

    if (!sBackendContext->Initialise(*sBackendContext, applicationName,
                                     platformState)) {
        KFATAL("Renderer backend failed to initialise. Shutting down.");
        return false;
    }

    return true;
}
void RendererShutdown() {
    sBackendContext->Shutdown(*sBackendContext);

    KFree(sBackendContext, sizeof(RendererBackend), MemoryTag::RENDERER);

    return;
}

bool RendererBeginFrame(f32 deltaTime) {
    return sBackendContext->BeginFrame(*sBackendContext, deltaTime);
}

bool RendererEndFrame(f32 deltaTime) {
    return sBackendContext->EndFrame(*sBackendContext, deltaTime);
}

void RendererOnResize(u16 width, u16 height) { return; }

bool RendererDrawFrame(RenderPacket& packet) {
    // If the begin frame returned successfully, mid-frame operations may
    // continue.
    if (RendererBeginFrame(packet.deltaTime)) {
        // End the frame. If this fails, it is likely unrecoverable.
        if (!RendererEndFrame(packet.deltaTime)) {
            KERROR("RendererEndFrame failed. Application shutting down...");
            return false;
        }
    }

    return true;
}
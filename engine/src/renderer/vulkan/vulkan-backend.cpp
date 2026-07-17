#include "vulkan-backend.h"

#include <core/logger.hpp>
#include <vector>

#include "vulkan-types.inl"

// Static Vulkan context.
static VulkanContext sVulkanContext {};

static u32 SelectVulkanApiVersion() {
    // Vulkan 1.0 loaders do not expose vkEnumerateInstanceVersion.
    u32 supportedVersion {VK_API_VERSION_1_0};
    const VkResult versionResult {
        vkEnumerateInstanceVersion(&supportedVersion)};
    if (versionResult != VK_SUCCESS) {
        return VK_API_VERSION_1_0;
    }

    // Target Vulkan 1.3 at most for broad compatibility (including MoltenVK).
    const u32 preferredVersion {VK_API_VERSION_1_1};
    if (supportedVersion < preferredVersion) {
        return supportedVersion;
    }

    return preferredVersion;
}

bool VulkanRendererBackendInitialise(RendererBackend& backend,
                                     const char* applicationName,
                                     PlatformState& platform) {
    // TODO: Custom allocator.
    sVulkanContext.Allocator = nullptr;

    // Setup Vulkan Instance.
    const u32 apiVersion = SelectVulkanApiVersion();
    VkApplicationInfo appInfo {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                               .pApplicationName = applicationName,
                               .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                               .pEngineName = "Kohi Engine",
                               .apiVersion = apiVersion};

    std::vector<const char*> requiredExtensions;
#if defined(KPLATFORM_APPLE)
    // MoltenVK is a portability driver; the loader requires this extension
    // (and the matching create-info flag) to enumerate it.
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if defined(KPLATFORM_APPLE)
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<u32>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()};

    VkResult result {vkCreateInstance(&createInfo, sVulkanContext.Allocator,
                                      &sVulkanContext.Instance)};

    if (result != VK_SUCCESS) {
        KERROR("vkCreateInstance failed with result: %d", result);
        return false;
    }

    KINFO("Vulkan instance created with API version %u.%u.%u",
          VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion),
          VK_VERSION_PATCH(apiVersion));

    KINFO("VUlkan renderer intialised successfully.");

    return true;
}

void VulkanRendererBackendShutdown(RendererBackend& backend) {}

void VulkanRendererBackendOnResize(RendererBackend& backend, u16 width,
                                   u16 height) {}

bool VulkanRendererBackendBeginFrame(RendererBackend& backend, f32 deltaTime) {
    return true;
}
bool VulkanRendererBackendEndFrame(RendererBackend& backend, f32 deltaTime) {
    return true;
}

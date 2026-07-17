#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

struct VulkanContext {
    VkInstance Instance;
    VkAllocationCallbacks* Allocator;
};
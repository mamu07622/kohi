#pragma once

#include "defines.hpp"

enum class MemoryTag : u8 {
    // For temporary use. Should be assigned one of the below or have a new tag
    // created.
    UNKNOWN,
    ARRAY,
    DARRAY,
    DICT,
    RING_QUEUE,
    BST,
    STRING,
    APPLICATION,
    JOB,
    TEXTURE,
    MATERIAL_INSTANCE,
    RENDERER,
    GAME,
    TRANSFORM,
    ENTITY,
    ENTITY_NODE,
    SCENE,

    MAX_TAGS
};

KAPI void InitialiseMemory();
KAPI void ShutdownMemory();

KAPI void* KAllocate(u64 size, MemoryTag tag);

KAPI void KFree(void* block, u64 size, MemoryTag tag);

KAPI void* KZeroMemory(void* block, u64 size);

KAPI void* KCopyMemory(void* destination, const void* source, u64 size);

KAPI void* KSetMemory(void* destination, i32 value, u64 size);

KAPI char* GetMemoryUsageString();
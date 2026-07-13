#include "kmemory.hpp"

#include "core/logger.hpp"
#include "platform/platform.hpp"

// TODO: Custom string lib
#include <stdio.h>

#include <iomanip>
#include <sstream>
#include <string>

constexpr u64 GetMaximumMemoryTagCount() {
    return static_cast<u64>(MemoryTag::MAX_TAGS);
}

struct MemoryStatistics {
    u64 TotalAllocated;
    u64 TaggedAllocations[GetMaximumMemoryTagCount()];
};

static MemoryStatistics sMemoryStatistics {};

static const char* MemoryTagStrings[GetMaximumMemoryTagCount()] {
    "UNKNOWN    ", "ARRAY      ", "DARRAY     ", "DICT       ", "RING_QUEUE ",
    "BST        ", "STRING     ", "APPLICATION", "JOB        ", "TEXTURE    ",
    "MAT_INST   ", "RENDERER   ", "GAME       ", "TRANSFORM  ", "ENTITY     ",
    "ENTITY_NODE", "SCENE      "};

struct MemorySizeView {
    f64 Amount;
    const char* Unit;
};

constexpr MemorySizeView ToMemorySizeView(u64 bytes) {
    constexpr u64 gib {1024ULL * 1024ULL * 1024ULL};
    constexpr u64 mib {1024ULL * 1024ULL};
    constexpr u64 kib {1024ULL};

    if (bytes >= gib) {
        return {static_cast<f64>(bytes) / static_cast<f64>(gib), "GiB"};
    }

    if (bytes >= mib) {
        return {static_cast<f64>(bytes) / static_cast<f64>(mib), "MiB"};
    }

    if (bytes >= kib) {
        return {static_cast<f64>(bytes) / static_cast<f64>(kib), "KiB"};
    }

    return {static_cast<f64>(bytes), "B"};
}

static void RecordDeallocation(MemoryTag tag, u64 size) {
    sMemoryStatistics.TotalAllocated -= size;
    sMemoryStatistics.TaggedAllocations[static_cast<u64>(tag)] -= size;

    return;
}

static void RecordAllocation(MemoryTag tag, u64 size) {
    sMemoryStatistics.TotalAllocated += size;
    sMemoryStatistics.TaggedAllocations[static_cast<u64>(tag)] += size;

    return;
}

void InitialiseMemory() { return; }

void ShutdownMemory() { return; }

KAPI void* KAllocate(u64 size, MemoryTag tag) {
    if (tag == MemoryTag::UNKNOWN) {
        KWARN(
            "KAllocate called using MemoryTag::Unknown. Please re-class this "
            "allocation.");
    }

    RecordAllocation(tag, size);

    // TODO: Memory alignment
    void* block {PlatformAllocate(size)};

    PlatformZeroMemory(block, size);

    return block;
}

KAPI void KFree(void* block, u64 size, MemoryTag tag) {
    if (tag == MemoryTag::UNKNOWN) {
        KWARN(
            "KFree called using MemoryTag::Unknown. Please re-class this "
            "allocation.");
    }

    RecordDeallocation(tag, size);

    // TODO: Memory alignment
    PlatformFree(block);

    return;
}

KAPI void* KZeroMemory(void* block, u64 size) {
    return PlatformZeroMemory(block, size);
}

KAPI void* KCopyMemory(void* destination, const void* source, u64 size) {
    return PlatformCopyMemory(destination, source, size);
}

KAPI void* KSetMemory(void* destination, i32 value, u64 size);

KAPI char* GetMemoryUsageString() {
    std::ostringstream stream {};

    stream << "System memory use (tagged):\n";
    stream << std::fixed << std::setprecision(2);

    for (u64 i = 0; i < GetMaximumMemoryTagCount(); ++i) {
        const MemorySizeView sizeView {
            ToMemorySizeView(sMemoryStatistics.TaggedAllocations[i])};

        stream << "  " << MemoryTagStrings[i] << ": " << sizeView.Amount
               << sizeView.Unit << "\n";
    }

    const std::string output {stream.str()};

    auto* memoryString {static_cast<char*>(
        PlatformAllocate(static_cast<u64>(output.size() + 1)))};

    if (!memoryString) {
        return nullptr;
    }

    PlatformCopyMemory(memoryString, output.c_str(),
                       static_cast<u64>(output.size() + 1));

    return memoryString;
}
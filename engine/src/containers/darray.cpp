#include "darray.h"

#include <cstring>

#include "core/kmemory.hpp"

namespace detail {

u64* DArrayHeader(void* array) noexcept {
    return reinterpret_cast<u64*>(array) - DArrayHeaderFieldCount;
}

const u64* DArrayHeader(const void* array) noexcept {
    return reinterpret_cast<const u64*>(array) - DArrayHeaderFieldCount;
}

u64 DArrayFieldGet(const void* array, const DArrayField field) noexcept {
    return DArrayHeader(array)[ToUnderlying(field)];
}

void DArrayFieldSet(void* array, const DArrayField field,
                    const u64 value) noexcept {
    DArrayHeader(array)[ToUnderlying(field)] = value;

    return;
}

u64 DArrayAllocationSize(const u64 capacity, const u64 stride) noexcept {
    return DArrayHeaderSize + (capacity * stride);
}

void* DArrayCreateRaw(const u64 capacity, const u64 stride) {
    auto* block {static_cast<u64*>(
        KAllocate(DArrayAllocationSize(capacity, stride), MemoryTag::DARRAY))};

    if (!block) {
        return nullptr;
    }

    block[ToUnderlying(DArrayField::Capacity)] = capacity;
    block[ToUnderlying(DArrayField::Length)] = 0;
    block[ToUnderlying(DArrayField::Stride)] = stride;

    return block + DArrayHeaderFieldCount;
}

void DArrayDestroyRaw(void* array) {
    if (!array) {
        return;
    }

    auto* header {DArrayHeader(array)};

    KFree(header,
          DArrayAllocationSize(header[ToUnderlying(DArrayField::Capacity)],
                               header[ToUnderlying(DArrayField::Stride)]),
          MemoryTag::DARRAY);
}

void* DArrayResizeRaw(void* array) {
    const u64 length {DArrayFieldGet(array, DArrayField::Length)};
    const u64 oldCapacity {DArrayFieldGet(array, DArrayField::Capacity)};
    const u64 stride {DArrayFieldGet(array, DArrayField::Stride)};
    const u64 newCapacity {oldCapacity == 0 ? DArrayDefaultCapacity
                                            : oldCapacity * DArrayResizeFactor};

    auto* newArray {DArrayCreateRaw(newCapacity, stride)};

    if (!newArray) {
        return array;
    }

    KCopyMemory(newArray, array, length * stride);
    DArrayFieldSet(newArray, DArrayField::Length, length);
    DArrayDestroyRaw(array);

    return newArray;
}

void* DArrayPushRaw(void* array, const void* valuePtr) {
    const u64 length {DArrayFieldGet(array, DArrayField::Length)};

    if (length >= DArrayFieldGet(array, DArrayField::Capacity)) {
        array = DArrayResizeRaw(array);
    }

    const u64 stride {DArrayFieldGet(array, DArrayField::Stride)};
    auto* destination {static_cast<u8*>(array) + (length * stride)};

    KCopyMemory(destination, valuePtr, stride);
    DArrayFieldSet(array, DArrayField::Length, length + 1);

    return array;
}

void DArrayPopRaw(void* array, void* dest) {
    const u64 length {DArrayFieldGet(array, DArrayField::Length)};

    if (length == 0) {
        return;
    }

    const u64 stride {DArrayFieldGet(array, DArrayField::Stride)};
    auto* source {static_cast<u8*>(array) + ((length - 1) * stride)};

    if (dest) {
        KCopyMemory(dest, source, stride);
    }

    DArrayFieldSet(array, DArrayField::Length, length - 1);
}

void* DArrayPopAtRaw(void* array, const u64 index, void* dest) {
    const u64 length {DArrayFieldGet(array, DArrayField::Length)};

    if (index >= length) {
        return array;
    }

    const u64 stride {DArrayFieldGet(array, DArrayField::Stride)};
    auto* source {static_cast<u8*>(array) + (index * stride)};

    if (dest) {
        KCopyMemory(dest, source, stride);
    }

    const u64 trailingCount {length - index - 1};

    if (trailingCount > 0) {
        std::memmove(source, source + stride, trailingCount * stride);
    }

    DArrayFieldSet(array, DArrayField::Length, length - 1);

    return array;
}

void* DArrayInsertAtRaw(void* array, const u64 index, const void* valuePtr) {
    const u64 length {DArrayFieldGet(array, DArrayField::Length)};

    if (index >= length) {
        return DArrayPushRaw(array, valuePtr);
    }

    if (length >= DArrayFieldGet(array, DArrayField::Capacity)) {
        array = DArrayResizeRaw(array);
    }

    const u64 stride {DArrayFieldGet(array, DArrayField::Stride)};
    auto* insertionPoint {static_cast<u8*>(array) + (index * stride)};

    std::memmove(insertionPoint + stride, insertionPoint,
                 (length - index) * stride);
                 
    KCopyMemory(insertionPoint, valuePtr, stride);
    DArrayFieldSet(array, DArrayField::Length, length + 1);

    return array;
}

}  // namespace detail

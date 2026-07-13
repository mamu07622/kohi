#pragma once

#include "defines.hpp"

/*
Memory layout
u64 capacity = number elements that can be held
u64 length = number of elements currently contained
u64 stride = size of each element in bytes
void* elements
*/

enum class DArrayField : u64 { Capacity, Length, Stride, FieldLength };

constexpr u64 DArrayDefaultCapacity {1};
constexpr u64 DArrayResizeFactor {2};

namespace detail {

constexpr u64 ToUnderlying(const DArrayField field) noexcept {
    return static_cast<u64>(field);
}

constexpr u64 DArrayHeaderFieldCount {ToUnderlying(DArrayField::FieldLength)};
constexpr u64 DArrayHeaderSize {DArrayHeaderFieldCount * sizeof(u64)};

u64* DArrayHeader(void* array) noexcept;
const u64* DArrayHeader(const void* array) noexcept;
u64 DArrayFieldGet(const void* array, DArrayField field) noexcept;
void DArrayFieldSet(void* array, DArrayField field, u64 value) noexcept;
u64 DArrayAllocationSize(u64 capacity, u64 stride) noexcept;
void* DArrayCreateRaw(u64 capacity, u64 stride);
void DArrayDestroyRaw(void* array);
void* DArrayResizeRaw(void* array);
void* DArrayPushRaw(void* array, const void* valuePtr);
void DArrayPopRaw(void* array, void* dest);
void* DArrayPopAtRaw(void* array, u64 index, void* dest);
void* DArrayInsertAtRaw(void* array, u64 index, const void* valuePtr);

}  // namespace detail

template <typename T>
[[nodiscard]] inline T* DArrayCreate() {
    return static_cast<T*>(
        detail::DArrayCreateRaw(DArrayDefaultCapacity, sizeof(T)));
}

template <typename T>
[[nodiscard]] inline T* DArrayReserve(u64 capacity) {
    return static_cast<T*>(detail::DArrayCreateRaw(capacity, sizeof(T)));
}

template <typename T>
inline void DArrayDestroy(T* array) {
    detail::DArrayDestroyRaw(array);
}

template <typename T>
inline void DArrayPush(T*& array, const T& value) {
    array = static_cast<T*>(detail::DArrayPushRaw(array, &value));
}

template <typename T>
inline void DArrayPop(T* array, T* valuePtr) {
    detail::DArrayPopRaw(array, valuePtr);
}

template <typename T>
inline void DArrayInsertAt(T*& array, u64 index, const T& value) {
    array = static_cast<T*>(detail::DArrayInsertAtRaw(array, index, &value));
}

template <typename T>
[[nodiscard]] inline T* DArrayPopAt(T* array, u64 index, T* valuePtr) {
    return static_cast<T*>(detail::DArrayPopAtRaw(array, index, valuePtr));
}

template <typename T>
inline void DArrayClear(T* array) {
    detail::DArrayFieldSet(array, DArrayField::Length, 0);
}

template <typename T>
[[nodiscard]] inline u64 DArrayCapacity(T* array) {
    return detail::DArrayFieldGet(array, DArrayField::Capacity);
}

template <typename T>
[[nodiscard]] inline u64 DArrayLength(T* array) {
    return detail::DArrayFieldGet(array, DArrayField::Length);
}

template <typename T>
[[nodiscard]] inline u64 DArrayStride(T* array) {
    return detail::DArrayFieldGet(array, DArrayField::Stride);
}

template <typename T>
inline void DArrayLengthSet(T* array, u64 value) {
    detail::DArrayFieldSet(array, DArrayField::Length, value);
}

#pragma once

#include "RefCounted.h"
#include <cassert>
#include <cstddef>
#include <utility>

namespace utils {

inline void addRef_(RefCounted* ptr)
{
    ptr->addRef();
}

/// Release the object reference and delete it if necessary.
inline void releaseRef_(RefCounted* ptr)
{
    ptr->releaseRef();
}

/// Shared pointer template class with intrusive reference counting.
template <class T>
class SharedPtr {
public:
    /// Construct a null shared pointer.
    SharedPtr() noexcept
        : ptr_(nullptr)
    {
    }

    /// Construct a null shared pointer.
    SharedPtr(std::nullptr_t) noexcept
        : // NOLINT(google-explicit-constructor)
        ptr_(nullptr)
    {
    }

    /// Copy-construct from another shared pointer.
    SharedPtr(const SharedPtr<T>& rhs) noexcept
        : ptr_(rhs.ptr_)
    {
        addRef();
    }

    /// Move-construct from another shared pointer.
    SharedPtr(SharedPtr<T>&& rhs) noexcept
        : ptr_(rhs.ptr_)
    {
        rhs.ptr_ = nullptr;
    }

    /// Copy-construct from another shared pointer allowing implicit upcasting.
    template <class U>
    SharedPtr(const SharedPtr<U>& rhs) noexcept
        : // NOLINT(google-explicit-constructor)
        ptr_(rhs.ptr_)
    {
        addRef();
    }

    /// Construct from a raw pointer.
    explicit SharedPtr(T* ptr) noexcept
        : ptr_(ptr)
    {
        addRef();
    }

    /// Construct from a raw pointer.
    SharedPtr(T& ptr) noexcept
        : ptr_(&ptr)
    {
        addRef();
    }

    /// Destruct. Release the object reference.
    ~SharedPtr() noexcept
    {
        releaseRef();
    }

    /// Assign from another shared pointer.
    SharedPtr<T>& operator=(const SharedPtr<T>& rhs)
    {
        if (ptr_ == rhs.ptr_)
            return *this;

        SharedPtr<T> copy(rhs);
        swap(copy);

        return *this;
    }

    /// Move-assign from another shared pointer.
    SharedPtr<T>& operator=(SharedPtr<T>&& rhs)
    {
        SharedPtr<T> copy(std::move(rhs));
        swap(copy);

        return *this;
    }

    /// Assign from another shared pointer allowing implicit upcasting.
    template <class U>
    SharedPtr<T>& operator=(const SharedPtr<U>& rhs)
    {
        if (ptr_ == rhs.ptr_)
            return *this;

        SharedPtr<T> copy(rhs);
        swap(copy);

        return *this;
    }

    /// Assign from a raw pointer.
    SharedPtr<T>& operator=(T* ptr)
    {
        if (ptr_ == ptr)
            return *this;

        SharedPtr<T> copy(ptr);
        swap(copy);

        return *this;
    }

    /// Assign from a raw pointer.
    SharedPtr<T>& operator=(T& ptr)
    {
        if (ptr_ == &ptr)
            return *this;

        SharedPtr<T> copy(ptr);
        swap(copy);

        return *this;
    }

    /// Point to the object.
    T* operator->() const
    {
        assert(ptr_);
        return ptr_;
    }

    /// Dereference the object.
    T& operator*() const
    {
        assert(ptr_);
        return *ptr_;
    }

    /// Subscript the object if applicable.
    T& operator[](int index)
    {
        assert(ptr_);
        return ptr_[index];
    }

    /// Test for less than with another shared pointer.
    template <class U>
    bool operator<(const SharedPtr<U>& rhs) const
    {
        return ptr_ < rhs.ptr_;
    }

    /// Test for equality with another shared pointer.
    template <class U>
    bool operator==(const SharedPtr<U>& rhs) const
    {
        return ptr_ == rhs.ptr_;
    }

    /// Test for inequality with another shared pointer.
    template <class U>
    bool operator!=(const SharedPtr<U>& rhs) const
    {
        return ptr_ != rhs.ptr_;
    }

    /// Convert to a raw pointer.
    operator T*() const
    {
        return ptr_;
    }

    T** getAddr()
    {
        return &ptr_;
    }

    /// swap with another SharedPtr.
    void swap(SharedPtr<T>& rhs)
    {
        std::swap(ptr_, rhs.ptr_);
    }

    /// Reset with another pointer.
    void reset(T* ptr = nullptr)
    {
        SharedPtr<T> copy(ptr);
        swap(copy);
    }

    /// Perform a static cast from a shared pointer of another type.
    template <class U>
    void staticCast(const SharedPtr<U>& rhs)
    {
        SharedPtr<T> copy(static_cast<T*>(rhs.get()));
        swap(copy);
    }

    /// Perform a dynamic cast from a shared pointer of another type.
    template <class U>
    void dynamicCast(const SharedPtr<U>& rhs)
    {
        SharedPtr<T> copy(dynamic_cast<T*>(rhs.get()));
        swap(copy);
    }

    /// Check if the pointer is null.
    bool null() const
    {
        return ptr_ == 0;
    }

    /// Check if the pointer is not null.
    bool notNull() const
    {
        return ptr_ != nullptr;
    }

    /// Return the raw pointer.
    T* get() const
    {
        return ptr_;
    }

    /// Return the object's reference count, or 0 if the pointer is null.
    int refs() const
    {
        return ptr_ ? ptr_->refs() : 0;
    }

protected:
    template <class U>
    friend class SharedPtr;

    /// Add a reference to the object pointed to.
    void addRef()
    {
        if (ptr_) {
            addRef_((RefCounted*)ptr_);
        }
    }

    /// Release the object reference and delete it if necessary.
    void releaseRef()
    {

        if (ptr_) {
            releaseRef_((RefCounted*)ptr_);
            ptr_ = nullptr;
        }
    }

    /// Pointer to the object.
    T* ptr_;
};

/// Perform a static cast from one shared pointer type to another.
template <class T, class U>
SharedPtr<T> staticCast(const SharedPtr<U>& ptr)
{
    SharedPtr<T> ret;
    ret.staticCast(ptr);
    return ret;
}

/// Perform a dynamic cast from one weak pointer type to another.
template <class T, class U>
SharedPtr<T> dynamicCast(const SharedPtr<U>& ptr)
{
    SharedPtr<T> ret;
    ret.dynamicCast(ptr);
    return ret;
}

/// Delete object of type T. T must be complete. See boost::checked_delete.
template <class T>
inline void checkedDelete(T* x)
{
    // intentionally complex - simplification causes regressions
    using type_must_be_complete = char[sizeof(T) ? 1 : -1];
    (void)sizeof(type_must_be_complete);
    delete x;
}

/// Construct SharedPtr.
template <class T, class... Args>
SharedPtr<T> makeShared(Args&&... args)
{
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
using Ref = SharedPtr<T>;

template <class T, class... Args>
T& makeRef(Args&&... args)
{
    return *new T(std::forward<Args>(args)...);
}

}

namespace std {
template <typename T>
struct hash<utils::SharedPtr<T>> {
    size_t operator()(const utils::SharedPtr<T>& value) const
    {
        return (size_t)value.get();
    }
};

}
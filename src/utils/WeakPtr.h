#pragma once
#include "RefCounted.h"

namespace utils {

/// Weak pointer template class with intrusive reference counting. Does not keep the object pointed to alive.
template <class T>
class WeakPtr {
public:
    /// Construct a null weak pointer.
    WeakPtr() noexcept
        : ptr_(nullptr)
        , refCount_(nullptr)
    {
    }

    /// Construct a null weak pointer.
    WeakPtr(std::nullptr_t) noexcept
        : // NOLINT(google-explicit-constructor)
        ptr_(nullptr)
        , refCount_(nullptr)
    {
    }

    /// Copy-construct from another weak pointer.
    WeakPtr(const WeakPtr<T>& rhs) noexcept
        : ptr_(rhs.ptr_)
        , refCount_(rhs.refCount_)
    {
        addRef();
    }

    /// Copy-construct from another weak pointer allowing implicit upcasting.
    template <class U>
    WeakPtr(const WeakPtr<U>& rhs) noexcept
        : // NOLINT(google-explicit-constructor)
        ptr_(rhs.ptr_)
        , refCount_(rhs.refCount_)
    {
        addRef();
    }

    /// Construct from a shared pointer.
    WeakPtr(const SharedPtr<T>& rhs) noexcept
        : // NOLINT(google-explicit-constructor)
        ptr_(rhs.get())
        , refCount_(rhs->getWeakRefCount())
    {
        addRef();
    }

    /// Construct from a raw pointer.
    explicit WeakPtr(T* ptr) noexcept
        : ptr_(ptr)
        , refCount_(ptr ? ptr->getWeakRefCount() : nullptr)
    {
        addRef();
    }

    /// Destruct. Release the weak reference to the object.
    ~WeakPtr() noexcept
    {
        releaseRef();
    }

    /// Assign from a shared pointer.
    WeakPtr<T>& operator=(const SharedPtr<T>& rhs)
    {
        if (ptr_ == rhs.get() && refCount_ == rhs->getWeakRefCount())
            return *this;

        releaseRef();
        ptr_ = rhs.get();
        refCount_ = rhs->getWeakRefCount();
        addRef();

        return *this;
    }

    /// Assign from a weak pointer.
    WeakPtr<T>& operator=(const WeakPtr<T>& rhs)
    {
        if (ptr_ == rhs.ptr_ && refCount_ == rhs.refCount_)
            return *this;

        releaseRef();
        ptr_ = rhs.ptr_;
        refCount_ = rhs.refCount_;
        addRef();

        return *this;
    }

    /// Assign from another weak pointer allowing implicit upcasting.
    template <class U>
    WeakPtr<T>& operator=(const WeakPtr<U>& rhs)
    {
        if (ptr_ == rhs.ptr_ && refCount_ == rhs.refCount_)
            return *this;

        releaseRef();
        ptr_ = rhs.ptr_;
        refCount_ = rhs.refCount_;
        addRef();

        return *this;
    }

    /// Assign from a raw pointer.
    WeakPtr<T>& operator=(T* ptr)
    {
        WeakRefCount* refCount = ptr ? ptr->getWeakRefCount() : nullptr;

        if (ptr_ == ptr && refCount_ == refCount)
            return *this;

        releaseRef();
        ptr_ = ptr;
        refCount_ = refCount;
        addRef();

        return *this;
    }

    /// Convert to a shared pointer. If expired, return a null shared pointer.
    SharedPtr<T> lock() const
    {
        if (expired())
            return SharedPtr<T>();
        else
            return SharedPtr<T>(ptr_);
    }

    /// Return raw pointer. If expired, return null.
    T* get() const
    {
        return expired() ? nullptr : ptr_;
    }

    /// Point to the object.
    T* operator->() const
    {
        T* rawPtr = get();
        assert(rawPtr);
        return rawPtr;
    }

    /// Dereference the object.
    T& operator*() const
    {
        T* rawPtr = get();
        assert(rawPtr);
        return *rawPtr;
    }

    /// Subscript the object if applicable.
    T& operator[](int index)
    {
        T* rawPtr = get();
        assert(rawPtr);
        return (*rawPtr)[index];
    }

    /// Test for equality with another weak pointer.
    template <class U>
    bool operator==(const WeakPtr<U>& rhs) const { return ptr_ == rhs.ptr_ && refCount_ == rhs.refCount_; }

    /// Test for inequality with another weak pointer.
    template <class U>
    bool operator!=(const WeakPtr<U>& rhs) const { return ptr_ != rhs.ptr_ || refCount_ != rhs.refCount_; }

    /// Test for less than with another weak pointer.
    template <class U>
    bool operator<(const WeakPtr<U>& rhs) const { return ptr_ < rhs.ptr_; }

    /// Convert to a raw pointer, null if the object is expired.
    operator T*() const { return get(); } // NOLINT(google-explicit-constructor)

    /// Reset to null and release the weak reference.
    void reset() { releaseRef(); }

    /// Perform a static cast from a weak pointer of another type.
    template <class U>
    void staticCast(const WeakPtr<U>& rhs)
    {
        releaseRef();
        ptr_ = static_cast<T*>(rhs.get());
        refCount_ = rhs.refCount_;
        addRef();
    }

    /// Perform a dynamic cast from a weak pointer of another type.
    template <class U>
    void dynamicCast(const WeakPtr<U>& rhs)
    {
        releaseRef();
        ptr_ = dynamic_cast<T*>(rhs.get());

        if (ptr_) {
            refCount_ = rhs.refCount_;
            addRef();
        } else
            refCount_ = 0;
    }

    /// Check if the pointer is null.
    bool isNull() const { return refCount_ == nullptr; }

    /// Return the object's reference count, or 0 if null pointer or if object has expired.
    int refs() const { return (refCount_ && refCount_->weakRefs >= 0) ? refCount_->weakRefs : 0; }

    /// Return the object's weak reference count.
    int weakRefs() const
    {
        if (!expired())
            return ptr_->weakRefs();
        else
            return refCount_ ? refCount_->weakRefs : 0;
    }

    /// Return whether the object has expired. If null pointer, always return true.
    bool expired() const { return refCount_ ? refCount_->expired.load() : true; }

    /// Return pointer to the RefCount structure.
    WeakRefCount* refCountPtr() const { return refCount_; }

private:
    template <class U>
    friend class WeakPtr;

    /// Add a weak reference to the object pointed to.
    void addRef()
    {
        if (refCount_) {
            refCount_->addRef();
        }
    }

    /// Release the weak reference. Delete the Refcount structure if necessary.
    void releaseRef()
    {
        if (refCount_) {
            refCount_->releaseRef();
        }

        ptr_ = nullptr;
        refCount_ = nullptr;
    }

    /// Pointer to the object.
    T* ptr_;
    /// Pointer to the RefCount structure.
    WeakRefCount* refCount_;
};

/// Perform a static cast from one weak pointer type to another.
template <class T, class U>
WeakPtr<T> staticCast(const WeakPtr<U>& ptr)
{
    WeakPtr<T> ret;
    ret.staticCast(ptr);
    return ret;
}

/// Perform a dynamic cast from one weak pointer type to another.
template <class T, class U>
WeakPtr<T> dynamicCast(const WeakPtr<U>& ptr)
{
    WeakPtr<T> ret;
    ret.dynamicCast(ptr);
    return ret;
}

}
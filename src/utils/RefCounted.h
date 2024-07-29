#pragma once
#include <atomic>

namespace utils {

	struct WeakRefCount {
		std::atomic<bool> expired = false;
		std::atomic<int> weakRefs = 0;
	};

    /// Base class for intrusively reference-counted objects. These are noncopyable and non-assignable.
    class RefCounted
    {
    public:
        RefCounted();
        virtual ~RefCounted();

        /// Prevent copy construction.
        RefCounted(const RefCounted& rhs) = delete;
        /// Prevent assignment.
        RefCounted& operator=(const RefCounted& rhs) = delete;
        
        void addRef();
        void releaseRef();
		int refs() const;
		//WeakRefCount* getWeakRefCount() const;
    protected:
        virtual void deleteThis();
    private:
		std::atomic<int> mRefCount = 0;
		//mutable Handle<WeakRefCount> mWeakRefCount;
    };

	static_assert(sizeof(RefCounted) == 16);

}


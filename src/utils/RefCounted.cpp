#include "RefCounted.h"

namespace utils {
	
    RefCounted::RefCounted()
    {
    }
    
    RefCounted::~RefCounted()
	{/*
		if (mWeakRefCount) {
			mWeakRefCount->expired = true;
			(mWeakRefCount->weakRefs)--;
			if (!mWeakRefCount->weakRefs)
				mWeakRefCount.free();
		}*/
    }
	
    void RefCounted::addRef()
    {
        ++mRefCount;
    }

    void RefCounted::releaseRef()
    {
		if (--mRefCount == 0) {
			deleteThis();
		}
    }

	void RefCounted::deleteThis() {
		delete this;
	}

    int RefCounted::refs() const
    {
        return mRefCount;
    }
	/*
	WeakRefCount* RefCounted::getWeakRefCount() const {
		if (mWeakRefCount.isNull()) {
			mWeakRefCount.create();
			(mWeakRefCount->weakRefs)++;
		}

		return mWeakRefCount.get();
	}*/


}

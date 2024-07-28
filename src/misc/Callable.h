#pragma once
#include <list>

namespace mygfx {

	template<typename T>
	class TCallable1 {
	public:
		TCallable1()
		{
			s_Callables.push_back((T*)this);
		}

		~TCallable1()
		{
			s_Callables.remove((T*)this);
		}

		static void callAll1()
		{
			for (auto obj : s_Callables) {
				obj->onCall1();
			}
		}

		virtual void onCall1() {}

	protected:
		inline static std::list<T*> s_Callables;
	};

	template<typename T>
	class TCallable2 : public TCallable1<T> {
	public:

		static void callAll2()
		{
			for (auto obj : TCallable1<T>::s_Callables) {
				obj->onCall2();
			}
		}

		virtual void onCall2() {}
	};

}
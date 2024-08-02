#pragma once
#include "utils/RefCounted.h"
#include "utils/SharedPtr.h"
#include <vector>
#include <string>
#include "MathTypes.h"

namespace mygfx {

	using namespace utils;

	using String = std::string;

	template<typename T>
	using Vector = std::vector<T>;

	class Node : public utils::RefCounted {
	public:
		Node();
		
		const vec3& position() const { return mPosition; }
		const quat& rotation() const { return mRotation; }
		const vec3& scale() const { return mScale; }

		Node& position(const vec3& p);
		Node& rotation(const quat& r);
		Node& scale(const vec3& s);

		const mat4& getWorldTransform() const;
		void updateTransform() const;

		String name;
		Vector<utils::Ref<Node>> children;
		Node* parent = nullptr;

	protected:
		vec3 mPosition{ 0.0f };
		quat mRotation = identity<quat>();
		vec3 mScale{ 1.0f };
		mutable mat4 worldTransform;
		mutable bool worldTransformDirty : 1 = true;
		mutable bool viewDirty : 1 = true;
		mutable bool projDirty : 1 = true;
	};

}
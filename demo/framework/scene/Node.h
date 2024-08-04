#pragma once
#include "Fwd.h"
#include "core/MathTypes.h"

namespace mygfx {

	
	class Scene;

	class Node : public utils::RefCounted {
	public:
		Node();
		Node(const String& name, const vec3& pos = vec3 {0}, const quat& rot = identity<quat>(), const vec3& s = one<vec3>());
		Node* createChild(const String& name, const vec3& pos = vec3 {0}, const quat& rot = identity<quat>(), const vec3& s = one<vec3>());
        void addChild(Node* child);
        void removeChild(Node* child);
		void remove();
        void setParent(Node* parent);
		
		inline const String& getName() const { return mName; }
		virtual void setName(const char* name);
        Node* getParent() const { return mParent; }
        Scene* getScene() const { return mScene; }
		const vec3& position() const { return mPosition; }
		const quat& rotation() const { return mRotation; }
		const vec3& scale() const { return mScale; }

		Node& position(const vec3& p);
		Node& rotation(const quat& r);
		Node& scale(const vec3& s);
		void setTRS(const vec3& p, const quat& r, const vec3& s, bool notifyChange = true);
		void setTransform(const mat4& m);
		void lookAt(vec3 const& eye, vec3 const& center, vec3 const& up) noexcept;
		
		const vec3& getWorldPosition() const;
		quat getWorldRotation() const;
		const mat4& getWorldTransform() const;
		void updateTransform() const;

	protected:
        void setScene(Scene* scene);
		virtual void addToScene();
		virtual void removeFromScene();
        virtual void onAddChild(Node* child);
        virtual void onRemoveChild(Node* child);
		void hierarchyChanged();
		void transformChanged();
		virtual void onHierarchyChanged();
		virtual void onTransformChanged();
		
		String mName;
		Vector<utils::Ref<Node>> mChildren;
		Node* mParent = nullptr;
		Scene* mScene = nullptr;
		vec3 mPosition{ 0.0f };
		quat mRotation = identity<quat>();
		vec3 mScale{ 1.0f };
		mutable quat mWorldRotation;
		mutable mat4 mWorldTransform;
		bool mActive : 1 = true;
		mutable bool mWorldTransformDirty : 1 = true;
		mutable bool mViewDirty : 1 = true;
		mutable bool mProjDirty : 1 = true;
		mutable bool mSkinning : 1 = false;
		mutable bool mMorphing : 1 = false;
	};

}
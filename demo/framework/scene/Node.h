#pragma once
#include "Fwd.h"
#include "core/Maths.h"

namespace mygfx {

	
	class Scene;

	enum class TransformSpace {
		LOCAL = 0,
		PARENT,
		WORLD
	};

	class Node : public utils::RefCounted {
	public:
		Node();
		Node(const String& name, const vec3& pos = vec3 {0}, const quat& rot = identity<quat>(), const vec3& s = one<vec3>());
		virtual Ref<Node> clone();

		template<typename T, typename... Args>
		T* createChild(Args... args) {

			T* so = new T(std::forward<Args>(args)...);
			addChild(so);
			return so;			
		}

		template<typename T>
		T* createChild() {

			T* so = new T();
			addChild(so);
			return so;
		}

        void addChild(Node* child);
        void removeChild(Node* child);
		void remove();
        void setParent(Node* parent);
		
		inline const String& getName() const { return mName; }
		virtual void setName(const char* name);

        Node* getParent() const { return mParent; }
        Scene* getScene() const { return mScene; }

		const vec3& getPosition() const { return mPosition; }
		const quat& getRotation() const { return mRotation; }
		const vec3& getScale() const { return mScale; }

		Node& setPosition(const vec3& p);
		Node& setRotation(const quat& r);
		Node& setScale(const vec3& s);
		void setTRS(const vec3& p, const quat& r, const vec3& s, bool notifyChange = true);
		void setTransform(const mat4& m);
		void translate(const vec3& delta, TransformSpace space = TransformSpace::LOCAL);
		void lookAt(vec3 const& eye, vec3 const& center, vec3 const& up = {0.0f, 1.0f, 0.0f}) noexcept;
		
		const vec3& getWorldPosition() const;
		quat getWorldRotation() const;
		const mat4& getWorldTransform() const;
		void updateTransform() const;

	protected:
		virtual Node* createNode();
		virtual void cloneProcess(Node* destNode);
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
#include "Node.h"
#include "Scene.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace mygfx {
	
	Node::Node() = default;
	
    Node::Node(const String& n, const vec3& pos, const quat& rot, const vec3& s) {
        mName = n;
        mPosition = pos;
        mRotation = rot;
        mScale = s;
    }
    
    void Node::setName(const char* name) {
        mName = name;
    }

    Node* Node::createChild(const String& name, const vec3& pos, const quat& rot, const vec3& s) {
        Node* so = new Node(name, pos, rot, s);
        addChild(so);
        return so;
    }

    void Node::addChild(Node* child) {
        child->setParent(this);
    }

    void Node::removeChild(Node* child) {
        if (child->mParent == this) {
            child->setParent(nullptr);
        }
    }

    void Node::remove() {
        if (mParent) {
            mParent->removeChild(this);
        }
    }

    void Node::setParent(Node* parent) {
        if (mParent == parent) {
            return;
        }

        if (mParent != nullptr) {
            mParent->onRemoveChild(this);
        }

        mParent = parent;
        
        if (mParent != nullptr) {
            mParent->onAddChild(this);
		}

		hierarchyChanged();
    }
	
    void Node::onAddChild(Node* child) {
        mChildren.emplace_back(child);

        if (mScene != nullptr) {
            child->setScene(mScene);
        }        
    }

    void Node::onRemoveChild(Node* child) {
        auto it = std::find(mChildren.begin(), mChildren.end(), child);
        if (it != mChildren.end()) {
            mChildren.erase(it);
            child->setScene(nullptr);
        } else {
            assert(false);
        }
    }
	
    void Node::setScene(Scene* scene) {
        if (mScene == scene) {
            return;
        }

        if (mScene != nullptr) {            
            removeFromScene();
        }

        mScene = scene;

        if (mScene != nullptr) {
            
            addToScene();
        }

        for (auto& child: mChildren) {
            child->setScene(scene);
        }
    }

    void Node::addToScene()
    {
    }

    void Node::removeFromScene()
	{
    }
            
	void Node::hierarchyChanged()
	{
		onHierarchyChanged();

        transformChanged();
	}

    void Node::transformChanged() {
        
		mWorldTransformDirty = true;

        onTransformChanged();

		for (auto& child : mChildren) {
			child->transformChanged();
		}
    }
	
    void Node::onHierarchyChanged() {

    }

    void Node::onTransformChanged() {        
        
    }
    
	Node& Node::position(const vec3& p) {
		mPosition = p;
		transformChanged();
		return *this;
	}
		
	Node& Node::rotation(const quat& r) {
		mRotation = r;
		transformChanged();
		return *this;
	}

	Node& Node::scale(const vec3& s) {
		mScale = s;
		transformChanged();
		return *this;
	}
	
	void Node::setTRS(const vec3& p, const quat& r, const vec3& s, bool notifyChange)
	{
		mPosition = p;
		mRotation = r;
		mScale = s;
		
		if (notifyChange) {
			transformChanged();
		}
	}

	void Node::setTransform(const mat4& m) {
		vec3 tr;
		quat r;
		vec3 s;
        vec3 skew;
        vec4 perspective;
		glm::decompose(m, s, r, tr, skew, perspective);

		setTRS(tr, r, s);
	}

	void Node::lookAt(vec3 const& eye, vec3 const& center, vec3 const& up) noexcept
	{
		mPosition = eye;
		vec3 dir = normalize(center - eye);
		mRotation = quatLookAtRH(dir, up);
		mScale = one<vec3>();
		transformChanged();
	}
	
    const vec3& Node::getWorldPosition() const {
        return *(const vec3*)&mWorldTransform[3][0];
    }
       
    quat Node::getWorldRotation() const {
        if (mWorldTransformDirty) {
			updateTransform();
		}

        return mWorldRotation;
    }

	const mat4& Node::getWorldTransform() const {
		if (mWorldTransformDirty) {
			updateTransform();
		}

		return mWorldTransform;
	}

	void Node::updateTransform() const {

		mat4 r = glm::scale_slow(glm::mat4_cast(mRotation), mScale);
		mat4 localTransform = glm::translate(r, mPosition);

		if (mParent) {
            mWorldRotation = mParent->getWorldRotation() * mRotation;
			mWorldTransform = mParent->getWorldTransform() * localTransform;
		} else {
            mWorldRotation = mRotation;
			mWorldTransform = localTransform;
		}

        mWorldTransformDirty = false;
	}
}
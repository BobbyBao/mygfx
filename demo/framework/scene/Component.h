#pragma once
#include "core/Fwd.h"
#include "core/Object.h"


namespace mygfx {
	
    class Node;
    class Scene;

	class Component : public Object {
	public:
		Component();
        
        Scene* getScene() const;

		Node* getOwner() const { return mOwner; }
        bool enabled() const { return mEnable; }
        bool isEnabledEffective() const { return mEnabledEffective; }

        void setEnabled(bool enable);
	protected:
        void setOwner(Node* node);
        virtual void onActive();
        virtual void onDeactive();
        virtual void onSetOwner(Node* node);
        virtual void onAddToScene(Scene* scene);
        virtual void onRemoveFromScene(Scene* scene);
        virtual void onActiveChanged();
        virtual void onParentChanged(Node* parent);
        virtual void onTransformChanged();
                        
        Node* mOwner = nullptr;
    private:
        void checkActivateState();

        bool mEnable = true;
        bool mEnabledEffective = true;

        friend class Node;
	};

}
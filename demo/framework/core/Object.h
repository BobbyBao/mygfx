#pragma once
#include "core/Fwd.h"
#include "utils/SharedPtr.h"

namespace mygfx {
	
	class Object : public utils::RefCounted {
	public:
		Object();
        		
        virtual Ref<Object> clone();
		  
	protected:
		virtual Object* createObject();
		virtual void cloneProcess(Object* destNode);
	};
	
#define PROPERTY_GET_SET(type, prop)\
    type get##prop() const { return m##prop; }\
    void set##prop(type v) { m##prop = v; }

#define PROPERTY_GET(type, prop)\
    type get##prop() const { return m##prop; }\


#define PROPERTY_GET_SET_BOOL(type, prop)\
    type is##prop() const { return m##prop; }\
    void set##prop(type v) { m##prop = v; }

#define PROPERTY_GET_BOOL(type, prop)\
    type is##prop() const { return m##prop; }\

#define PROPERTY_GET_WITH_BOOL(type, prop)\
    type is##prop() const { return m##prop; }

#define PROPERTY_GET_SET_1(type, prop)\
    const type& get##prop() const { return m##prop; }\
    void set##prop(const type& v) { m##prop = v; }

#define PROPERTY_GET_1(type, prop)\
    const type& get##prop() const { return m##prop; }\

}
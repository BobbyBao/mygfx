#pragma once
#include "GraphicsDefs.h"
#include "utils/RefCounted.h"

#include <map>
#include <vector>

namespace mygfx {

	
	enum class UniformType : int8_t
	{
		UNKNOWN = -1,
		BOOL,
		INT8,
		UINT8,
		INT16,
		UINT16,
		INT32,
		UINT32,
		INT64,
		UINT64,
		HALF,
		FLOAT,
		DOUBLE,
		STRUCT,

		VEC2,
		VEC3,
		VEC4,
		MAT4,
		COLOR,

		OBJECT,

		TEXTURE,
		SAMPLER,
		BUFFER,
	};

	class DefineList : public std::map<const String, String>
	{
	public:
		bool has(const std::string& str) const;

		size_t hash(size_t result) const;

		DefineList& add(const String& key, size_t val);
		DefineList& add(const String& key, int val);
		DefineList& add(const String& key);

		DefineList operator+(const DefineList& def2);
		DefineList& operator+=(const DefineList& def2);
	};

	class ShaderStruct
	{
	public:
		const ShaderStruct* getMember(const String& name) const;
		//const ShaderStruct* getMember(const String& name) const;
		bool getMemberOffset(const String& name, uint32_t& offset) const;
		uint32_t getMemberSize() const;
		void clear();

		operator bool() const { return size != 0; }

		bool operator == (const ShaderStruct& other) const;

		String name{};
		UniformType type = UniformType::UNKNOWN;
		uint32_t offset{ 0 };
		uint32_t size{ 0 };
		std::vector<ShaderStruct> members;
	};

	class PushConstant : public ShaderStruct {
	public:
		ShaderStage stageFlags;
	};

	enum class ConstantType : uint8_t {
		INT,
		FLOAT,
		BOOL
	};

	struct SpecializationConst
	{
		uint32_t id;
		UniformType type;
		uint32_t size;

		inline bool operator <(const SpecializationConst& other)
		{
			return id < other.id;
		}
	};
	
	class ShaderResourceInfo : public RefCounted, public ShaderStruct {
	public:
		uint32_t set{ 0 };
		DescriptorSetLayoutBinding dsLayoutBinding;
		bool bindless = false;
		uint32_t binding() const { return dsLayoutBinding.binding; }
	};

}
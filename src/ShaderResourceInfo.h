#pragma once
#include "GraphicsDefs.h"
#include "utils/RefCounted.h"

#include <map>
#include <vector>

namespace mygfx {

	
	enum class UniformType : int8_t
	{
		Unknown = -1,
		Bool,
		Int8,
		Uint8,
		Int16,
		Uint16,
		Int32,
		Uint32,
		Int64,
		Uint64,
		Half,
		Float,
		Double,
		Struct,

		Vec2,
		Vec3,
		Vec4,
		Mat4,
		Color,

		Object,

		Texture,
		Sampler,
		Buffer,
	};

	class DefineList : public std::map<const String, String>
	{
	public:
		bool has(const std::string& str) const;

		size_t hash(size_t result) const;

		DefineList& operator+(const DefineList& def2);
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
		UniformType type = UniformType::Unknown;
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
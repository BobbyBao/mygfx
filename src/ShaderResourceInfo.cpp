#include "ShaderResourceInfo.h"
#include "utils/algorithm.h"

namespace mygfx {

	bool DefineList::has(const std::string& str) const {
		return find(str) != end();
	}

	size_t DefineList::hash(size_t result) const {
		for (auto it = begin(); it != end(); it++) {
			utils::hash_combine(result, it->first);
			utils::hash_combine(result, it->second);
		}
		return result;
	}

	DefineList& DefineList::add(const String& key, size_t val) {
		(*this)[key] = std::to_string(val);
		return *this;
	}

	DefineList& DefineList::add(const String& key, int val) {
		(*this)[key] = std::to_string(val);
		return *this;
	}

	DefineList& DefineList::add(const String& key) {
		(*this)[key] = "";
		return *this;
	}

	DefineList& DefineList::operator+(const DefineList& def2) {
		for (auto it = def2.begin(); it != def2.end(); it++)
			(*this)[it->first] = it->second;
		return *this;
	}

	const ShaderStruct* ShaderStruct::getMember(const String& name) const {
		for (int i = 0; i < members.size(); i++) {
			const ShaderStruct& member = members[i];
			if (member.name == name) {
				return &member;
			}
		}

		return nullptr;
	}

	/*
	const ShaderStruct* ShaderStruct::getMember(const String& name) const
	{
		auto it = StringUtil::split(name, ".");
		const ShaderStruct* st = this;
		for (auto& str : it) {
			const ShaderStruct* st1 = st->getMember(str);
			if (st1 == nullptr) {
				return nullptr;
			}

			st = st1;
		}

		return st;

	}*/

	bool ShaderStruct::getMemberOffset(const String& name, uint32_t& offset) const
	{
		for (int i = 0; i < members.size(); i++) {
			const ShaderStruct& member = members[i];
			if (member.name == name) {
				offset = member.offset;
				return true;
			}
		}

		offset = -1;
		return false;
	}

	uint32_t ShaderStruct::getMemberSize() const
	{
		uint32_t sz = 0;
		for (int i = 0; i < members.size(); i++) {
			const ShaderStruct& member = members[i];
			sz += member.size;
		}
		return sz;
	}

	bool ShaderStruct::operator == (const ShaderStruct& other) const
	{
		return type == other.type && offset == other.offset && size == other.size;
	}

	void ShaderStruct::clear()
	{
		name = {};
		type = UniformType::UNKNOWN;
		offset = 0;
		size = 0;
		members.clear();
	}


}
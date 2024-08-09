#pragma once
#include "GraphicsFwd.h"
#include "utils/BitmaskEnum.h"

namespace utils {

	enum class FileMode : uint8_t
	{
		Read = (1 << 0),
		Write = (1 << 1),
		ReadWrite = Read | Write,
		Append = (1 << 2),
		Truncate = (1 << 3)
	};

	class IFile
	{
	public:

	public:
		IFile() = default;
		virtual ~IFile() = default;

		virtual const String& getName() const = 0;
		virtual uint64_t size() = 0;
		virtual bool isReadOnly() const = 0;
		virtual void open(FileMode mode) = 0;
		virtual void close() = 0;
		virtual bool isOpened() const = 0;

		virtual uint64_t seek(uint64_t offset) = 0;
		virtual uint64_t tell() = 0;
		virtual uint64_t read(uint8_t* buffer, uint64_t size) = 0;
		virtual uint64_t write(const uint8_t* buffer, uint64_t size) = 0;

		template<typename T>
		bool read(T& value)
		{
			return (read(reinterpret_cast<uint8_t*>(&value), sizeof(value)));
		}

		template<typename T>
		uint64_t write(const T& value)
		{
			return (write(reinterpret_cast<const uint8_t*>(&value), sizeof(value)));
		}

	};

	class IFileSystem {
	public:

	};

} // namespace utils

template<> struct utils::EnableBitMaskOperators<utils::FileMode>
: public std::true_type {};


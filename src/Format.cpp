#include "Format.h"


#include <tiny_imageformat/tinyimageformat.h>

namespace mygfx {


	struct FormatSizeHelper {
		FormatInfo formatSize[(int)Format::COUNT];

		FormatSizeHelper()
		{
			for (int i = 0; i < (int)Format::COUNT; i++) {
				auto fmt = (Format)i;
				formatSize[i] = FormatInfo 
				{
					.blockSizeInBits = getBitSizeOfBlock(fmt),
					.blockWidth = (uint8_t)getWidthOfBlock(fmt),
					.blockHeight = (uint8_t)getHeightOfBlock(fmt),
					.blockDepth = (uint8_t)getDepthOfBlock(fmt),
					.compressed = TinyImageFormat_IsCompressed((TinyImageFormat)fmt),
					.depth = TinyImageFormat_IsDepthOnly((TinyImageFormat)fmt) || TinyImageFormat_IsDepthAndStencil((TinyImageFormat)fmt),
					.stencil = TinyImageFormat_IsStencilOnly((TinyImageFormat)fmt) || TinyImageFormat_IsDepthAndStencil((TinyImageFormat)fmt),
				};
			}
		}
	};
	
	static FormatSizeHelper helper;

	uint32_t getWidthOfBlock(Format const fmt) {
		return TinyImageFormat_WidthOfBlock((TinyImageFormat)fmt);
	}
	
	uint32_t getHeightOfBlock(Format const fmt) {
		return TinyImageFormat_HeightOfBlock((TinyImageFormat)fmt);
	}

	uint32_t getDepthOfBlock(Format const fmt) {
		return TinyImageFormat_DepthOfBlock((TinyImageFormat)fmt);
	}
		
	uint32_t getBitSizeOfBlock(Format const fmt) {
		return TinyImageFormat_BitSizeOfBlock((TinyImageFormat)fmt);
	}
		
	uint32_t getChannelCount(Format const fmt) {
		return TinyImageFormat_ChannelCount((TinyImageFormat)fmt);
	}
	
	const FormatInfo& getFormatInfo(Format const fmt) {
		return helper.formatSize[(int)fmt];
	}

	char const* const getName(Format const fmt) {
		return TinyImageFormat_Name((TinyImageFormat)fmt);
	}
	
	Format fromName(char const* p) {
		return (Format)TinyImageFormat_FromName(p);
	}
}
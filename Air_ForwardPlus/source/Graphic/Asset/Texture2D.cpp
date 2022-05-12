#include "Graphic/Asset/Texture2D.h"
#include "core/LoadThread.h"
#include "FreeImage/FreeImage.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/CommandBuffer.h"
#include <iostream>
#include "Graphic/Manager/MemoryManager.h"
#include "Graphic/Instance/Buffer.h"
#include "utils/Log.h"
#include "Graphic/Instance/Semaphore.h"
#include "Graphic/Instance/Memory.h"
#include "Graphic/Instance/Image.h"
Graphic::Asset::Texture2D::Texture2DInstance::Texture2DInstance(std::string path)
	: IAssetInstance(path)
	, _textureInfoBuffer(nullptr)
	, _image(nullptr)
	, _vkSampler(VK_NULL_HANDLE)
	, _textureInfo()
	, _byteData()
	, _settings()
{
}

Graphic::Asset::Texture2D::Texture2DInstance::Texture2DInstance::~Texture2DInstance()
{
	delete _textureInfoBuffer;

	vkDestroySampler(Graphic::GlobalInstance::device, _vkSampler, nullptr);

	delete _image;
}

void Graphic::Asset::Texture2D::Texture2DInstance::_LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)
{
	_settings = Graphic::Asset::Texture2D::Texture2DSetting(path.c_str());;
	Graphic::Asset::Texture2D::Texture2DSetting& config = _settings;

	Instance::Semaphore semaphore = Instance::Semaphore();

	//Load bitmap
	{
		auto p = _settings.path.c_str();
		auto fileType = FreeImage_GetFileType(p);
		if (fileType == FREE_IMAGE_FORMAT::FIF_UNKNOWN) fileType = FreeImage_GetFIFFromFilename(p);
		if ((fileType != FREE_IMAGE_FORMAT::FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fileType))
		{
			FIBITMAP* bitmap = FreeImage_Load(fileType, p);
			uint32_t pixelDepth = FreeImage_GetBPP(bitmap);
			if (pixelDepth != 32)
			{
				FIBITMAP* t = bitmap;
				bitmap = FreeImage_ConvertTo32Bits(t);
				FreeImage_Unload(t);
			}
			uint32_t pitch = FreeImage_GetPitch(bitmap);
			_extent = VkExtent2D{ FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap) };
			_textureInfo.size = glm::vec4(1.0 / _extent.width, 1.0 / _extent.height, _extent.width, _extent.height);
			_textureInfo.tilingScale = glm::vec4(0, 0, 1, 1);
			_byteData.resize(static_cast<size_t>(_extent.width) * _extent.height * 4);
			FreeImage_ConvertToRawBits(_byteData.data(), bitmap, pitch, pixelDepth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0L);
			FreeImage_Unload(bitmap);
		}
	}
	//Create image
	{
		_image = new Instance::Image(
			_extent,
			_settings.format,
			VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
			static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT),
			1,
			_settings.sampleCount,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
			VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	//Create buffer
	Instance::Buffer textureStagingBuffer = Instance::Buffer(static_cast<uint64_t>(_extent.width) * _extent.height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	textureStagingBuffer.WriteBuffer(_byteData.data(), textureStagingBuffer.Size());

	_textureInfoBuffer = new Instance::Buffer(sizeof(_textureInfo), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Instance::Buffer infoStagingBuffer = Instance::Buffer(sizeof(_textureInfo), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	infoStagingBuffer.WriteBuffer(&_textureInfo, infoStagingBuffer.Size());

	transferCommandBuffer->Reset();
	transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	//Copy buffer to image
	{
		VkImageMemoryBarrier imageTransferBarrier{};
		imageTransferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageTransferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageTransferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageTransferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageTransferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageTransferBarrier.image = _image->VkImage_();
		imageTransferBarrier.subresourceRange = _image->VkImageSubresourceRange_();
		imageTransferBarrier.srcAccessMask = 0;
		imageTransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		transferCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			{},
			{},
			{ imageTransferBarrier }
		);
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource = _image->VkImageSubresourceLayers_();
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = _image->VkExtent3D_();

		transferCommandBuffer->CopyBufferToImage(textureStagingBuffer.VkBuffer(), _image->VkImage_(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { region });
	}
	//Copy buffer to buffer
	{
		transferCommandBuffer->CopyBuffer(infoStagingBuffer.VkBuffer(), _textureInfoBuffer->VkBuffer(), infoStagingBuffer.Size());
	}
	//Release buffer and image
	{
		VkImageMemoryBarrier releaseImageBarrier{};
		releaseImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		releaseImageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		releaseImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		releaseImageBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		releaseImageBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		releaseImageBarrier.subresourceRange = _image->VkImageSubresourceRange_();
		releaseImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		releaseImageBarrier.dstAccessMask = 0;
		releaseImageBarrier.image = _image->VkImage_();

		VkBufferMemoryBarrier releaseBufferBarrier = {};
		releaseBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		releaseBufferBarrier.pNext = nullptr;
		releaseBufferBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		releaseBufferBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		releaseBufferBarrier.offset = 0;
		releaseBufferBarrier.size = _textureInfoBuffer->Size();
		releaseBufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		releaseBufferBarrier.dstAccessMask = 0;
		releaseBufferBarrier.buffer = _textureInfoBuffer->VkBuffer();
		transferCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			{},
			{ releaseBufferBarrier },
			{ releaseImageBarrier }
		);
	}
	transferCommandBuffer->EndRecord();
	transferCommandBuffer->Submit({}, {}, { semaphore.VkSemphore() });

	renderCommandBuffer->Reset();
	renderCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		VkImageMemoryBarrier acquireImageBarrier{};
		acquireImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		acquireImageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		acquireImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		acquireImageBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		acquireImageBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		acquireImageBarrier.subresourceRange = _image->VkImageSubresourceRange_();
		acquireImageBarrier.srcAccessMask = 0;
		acquireImageBarrier.dstAccessMask = 0;
		acquireImageBarrier.image = _image->VkImage_();
		VkBufferMemoryBarrier acquireBufferBarrier = {};
		acquireBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		acquireBufferBarrier.pNext = nullptr;
		acquireBufferBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		acquireBufferBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		acquireBufferBarrier.offset = 0;
		acquireBufferBarrier.size = _textureInfoBuffer->Size();
		acquireBufferBarrier.srcAccessMask = 0;
		acquireBufferBarrier.dstAccessMask = 0;
		acquireBufferBarrier.buffer = _textureInfoBuffer->VkBuffer();
		renderCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			{},
			{ acquireBufferBarrier },
			{ acquireImageBarrier }
		);
	}
	renderCommandBuffer->EndRecord();
	renderCommandBuffer->Submit({ semaphore.VkSemphore() }, { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT }, {});

	renderCommandBuffer->WaitForFinish();

	renderCommandBuffer->Reset();
	transferCommandBuffer->Reset();

	_CreateTextureSampler(config, *this);
}

void Graphic::Asset::Texture2D::Texture2DInstance::_CreateTextureSampler(Texture2DSetting& config, Texture2DInstance& texture)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = config.magFilter;
	samplerInfo.minFilter = config.minFilter;
	samplerInfo.addressModeU = config.addressMode;
	samplerInfo.addressModeV = config.addressMode;
	samplerInfo.addressModeW = config.addressMode;
	samplerInfo.anisotropyEnable = config.anisotropy < 1.0f ? VK_FALSE : VK_TRUE;
	samplerInfo.maxAnisotropy = config.anisotropy < 1.0f ? 1.0f : config.anisotropy;
	samplerInfo.borderColor = config.borderColor;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.mipLodBias = 0.0f;

	Log::Exception("Failed to create sampler.", vkCreateSampler(Graphic::GlobalInstance::device, &samplerInfo, nullptr, &texture._vkSampler));
}

Graphic::Asset::Texture2D::Texture2D()
	: IAsset()
{

}

Graphic::Asset::Texture2D::~Texture2D()
{
}

std::future<Graphic::Asset::Texture2D*> Graphic::Asset::Texture2D::LoadAsync(const char* path)
{
	return _LoadAsync<Graphic::Asset::Texture2D, Graphic::Asset::Texture2D::Texture2DInstance>(path);
}

Graphic::Asset::Texture2D* Graphic::Asset::Texture2D::Load(const char* path)
{
	return _Load<Graphic::Asset::Texture2D, Graphic::Asset::Texture2D::Texture2DInstance>(path);
}

void Graphic::Asset::Texture2D::Unload(Texture2D* texture2D)
{
	_Unload< Graphic::Asset::Texture2D, Graphic::Asset::Texture2D::Texture2DInstance>(texture2D);
}

VkExtent2D Graphic::Asset::Texture2D::Extent()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->_extent;
}

Graphic::Instance::Image& Graphic::Asset::Texture2D::Image()
{
	return *dynamic_cast<Texture2DInstance*>(_assetInstance)->_image;
}

VkSampler Graphic::Asset::Texture2D::VkSampler()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->_vkSampler;
}

const Graphic::Asset::Texture2D::Texture2DSetting& Graphic::Asset::Texture2D::Settings()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->_settings;
}

Graphic::Instance::Buffer& Graphic::Asset::Texture2D::TextureInfoBuffer()
{
	return *dynamic_cast<Texture2DInstance*>(_assetInstance)->_textureInfoBuffer;
}
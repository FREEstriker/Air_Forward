#pragma once
#include <glm/ext/vector_int2.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
#include <glm/vec4.hpp>
#include <future>
#include "core/AssetUtils.h"
namespace Graphic
{
	class CommandBuffer;
	class MemoryManager;
	class MemoryBlock;
	struct Texture2DAssetConfig
	{
		std::string path;
		VkSampleCountFlagBits sampleCount;
		VkFormat format;
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerAddressMode addressMode;
		float anisotropy;
		VkBorderColor borderColor;

		Texture2DAssetConfig(const char* path)
			: path(path)
			, sampleCount(VK_SAMPLE_COUNT_1_BIT)
			, format(VK_FORMAT_R8G8B8A8_SRGB)
			, magFilter(VK_FILTER_LINEAR)
			, minFilter(VK_FILTER_LINEAR)
			, addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			, anisotropy(0.0f)
			, borderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
		{

		}
	};
	class Texture2DInstance;
	class Texture2D : IAsset
	{
		friend class IAsset;
	public:
		struct TextureInfo
		{
			alignas(16) glm::vec4 size;
			alignas(16) glm::vec4 tilingScale;
		};

		Texture2D(const Texture2D& source);
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&) = delete;
		Texture2D& operator=(Texture2D&&) = delete;
		~Texture2D();

		static std::future<Texture2D*>LoadAsync(const char* path);
		static Texture2D* Load(const char* path);

		VkExtent2D Size();
		VkImage TextureImage();
		VkFormat TextureFormat();
		VkImageView TextureImageView();
		VkSampler TextureSampler();

		VkBuffer TextureInfoBuffer();
		TextureInfo GetTextureInfo();

	private:
		Texture2D(Texture2DInstance* assetInstance);
	};
	class Texture2DInstance: public IAssetInstance
	{
		friend class Texture2D;
	public:
		Texture2DInstance(std::string path);
		virtual ~Texture2DInstance();
		VkExtent2D size;
		VkImage textureImage;
		VkFormat textureFormat;
		std::unique_ptr<MemoryBlock> imageMemory;
		std::unique_ptr<MemoryBlock> bufferMemory;
		VkBuffer buffer;
		VkImageView textureImageView;
		VkSampler sampler;
		Texture2D::TextureInfo textureInfo;
		std::vector<unsigned char> data;
	private:
		void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)override;
		static void _LoadTexture2D(Graphic::CommandBuffer* const commandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer, Texture2DAssetConfig config, Texture2DInstance& texture);
		static void _LoadBitmap(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);

		static void _CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, MemoryBlock* bufferMemory);
		static uint32_t _FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void _TransitionToTransferLayout(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void _CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, Graphic::CommandBuffer& commandBuffer);

		static void _CreateImage(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);
		static void _CreateImageView(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);
		static void _CreateTextureSampler(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);
	};
}

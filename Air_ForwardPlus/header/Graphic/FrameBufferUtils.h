#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <map>
#include <vector>

namespace Graphic
{
	class MemoryBlock;
	namespace Render
	{
		class RenderPass;
		typedef RenderPass* RenderPassHandle;
	}
	namespace Manager
	{
		class Attachment
		{
			friend class FrameBufferManager;
		public:
			std::string name;
			VkImage image;
			VkImageView imageView;
			MemoryBlock* memoryBlock;
			VkExtent2D size;
		private:
			Attachment();
			~Attachment();
		};
		class FrameBuffer
		{
			friend class FrameBufferManager;
		private:
			VkFramebuffer _frameBuffer;
		public:
			VkFramebuffer VulkanFrameBuffer();
		};
		class FrameBufferManager
		{
		private:
			std::map<std::string, Attachment*> _attachments;
			std::map<std::string, FrameBuffer> _frameBuffers;
		public:
			void AddAttachment(std::string name, VkExtent2D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);
			void AddFrameBuffer(std::string name, Render::RenderPassHandle renderPass, std::vector<std::string> attachments);
			const FrameBuffer GetFrameBuffer(std::string name);

			FrameBufferManager();
			~FrameBufferManager();
		};
	}
}
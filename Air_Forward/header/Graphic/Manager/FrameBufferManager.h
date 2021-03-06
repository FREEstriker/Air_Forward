#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <map>
#include <vector>
#include <shared_mutex>

namespace Graphic
{
	namespace RenderPass
	{
		class RenderPass;
		typedef RenderPass* RenderPassHandle;
	}
	namespace Instance
	{
		class Attachment;
		typedef Attachment* AttachmentHandle;
		class FrameBuffer;
		typedef FrameBuffer* FrameBufferHandle;
	}
	namespace Manager
	{
		class FrameBufferManager
		{
		private:
			std::shared_mutex _managerMutex;
			std::map<std::string, Instance::Attachment*> _attachments;
			std::map<std::string, uint32_t> _attachmentRefCounts;

			std::map<std::string, Instance::FrameBuffer*> _frameBuffers;
		public:
			void AddColorAttachment(std::string name, VkExtent2D extent, VkFormat format, VkImageUsageFlagBits extraUsage, VkMemoryPropertyFlagBits properties);
			void AddDepthAttachment(std::string name, VkExtent2D extent, VkFormat format, VkImageUsageFlagBits extraUsage, VkMemoryPropertyFlagBits properties);
			void AddDepthAttachment(std::string name, VkExtent2D extent, VkFormat format, VkImageUsageFlagBits extraUsage, VkMemoryPropertyFlagBits properties, VkImageAspectFlags extraAspect);
			void AddFrameBuffer(std::string name, RenderPass::RenderPassHandle renderPass, std::vector<std::string> attachments);
			Instance::FrameBufferHandle FrameBuffer(std::string name);
			Instance::AttachmentHandle Attachment(std::string name);
			void DeleteFrameBuffer(std::string name);
			void DeleteAttachment(std::string name);
			FrameBufferManager();
			~FrameBufferManager();
		};
	}
}
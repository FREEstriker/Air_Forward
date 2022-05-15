#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
namespace Graphic
{
	namespace Instance
	{
		class RenderPass;
		typedef RenderPass* RenderPassHandle;
		class Attachment;
		typedef Attachment* AttachmentHandle;
	}
	namespace Manager
	{
		class RenderPassManager
		{
		public:
			class RenderPassCreator final
			{
				friend class Graphic::Manager::RenderPassManager;
			public:
				class AttachmentDescriptor
				{
					friend class Graphic::Manager::RenderPassManager;
					std::string name;
					Instance::AttachmentHandle attachment;
					VkAttachmentLoadOp loadOp;
					VkAttachmentStoreOp storeOp;
					VkAttachmentLoadOp stencilLoadOp;
					VkAttachmentStoreOp stencilStoreOp;
					VkImageLayout initialLayout;
					VkImageLayout layout;
					VkImageLayout finalLayout;
					bool useStencil;
				public:
					AttachmentDescriptor(std::string name, Instance::AttachmentHandle attachment, VkImageLayout layout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout, bool isStencil, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp);
				};
				class SubpassDescriptor
				{
					friend class Graphic::Manager::RenderPassManager;
					std::string name;
					VkPipelineBindPoint pipelineBindPoint;
					std::vector<std::string> colorAttachmentNames;
					std::string depthStencilAttachmentName;
					bool useDepthStencilAttachment;

				public:
					SubpassDescriptor(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames, bool useDepthStencilAttachment, std::string depthStencilAttachmentName);
				};
				class DependencyDescriptor
				{
					friend class Graphic::Manager::RenderPassManager;
					std::string srcSubpassName;
					std::string dstSubpassName;
					VkPipelineStageFlags srcStageMask;
					VkPipelineStageFlags dstStageMask;
					VkAccessFlags srcAccessMask;
					VkAccessFlags dstAccessMask;
				public:
					DependencyDescriptor(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
				};
				std::string _name;
				std::map <std::string, AttachmentDescriptor> _attchments;
				std::map <std::string, SubpassDescriptor> _subpasss;
				std::vector <DependencyDescriptor> _dependencys;
				RenderPassCreator(const char* name);
				~RenderPassCreator();
				void AddColorAttachment(std::string name, Instance::AttachmentHandle attachment, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout);
				void AddSubpass(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames);
				void AddDependency(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
			};

		private:
			std::shared_mutex _managerMutex;
			std::map<std::string, Instance::RenderPass*> _renderPasss;
		public:
			RenderPassManager();
			~RenderPassManager();
			void CreateRenderPass(RenderPassCreator& creator);
			void DeleteRenderPass(const char* renderPassName);
			Instance::RenderPassHandle RenderPass(const char* renderPassName);
			Graphic::Instance::RenderPassHandle RenderPass(std::string renderPassName);
		};

	}
}
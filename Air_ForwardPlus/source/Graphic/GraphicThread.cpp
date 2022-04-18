#include "Graphic/GraphicThread.h"
#include "Graphic/Creator/GlfwWindowCreator.h"
#include "Graphic/Creator/VulkanInstanceCreator.h"
#include "Graphic/Creator/VulkanDeviceCreator.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/Creator/RenderPassCreator.h"
#include "Graphic/GlobalSetting.h"
#include "Graphic/DescriptorSetUtils.h"

Graphic::GraphicThread* const Graphic::GraphicThread::instance = new Graphic::GraphicThread();

Graphic::GraphicThread::GraphicThread()
	: _stopped(true)
{

}

Graphic::GraphicThread::~GraphicThread()
{

}

void Graphic::GraphicThread::Init()
{
	Graphic::GlfwWindowCreator glfwWindowCreator = Graphic::GlfwWindowCreator();
	Graphic::GlobalInstance::CreateGlfwWindow(&glfwWindowCreator);
	Graphic::VulkanInstanceCreator vulkanInstanceCreator = Graphic::VulkanInstanceCreator();
	Graphic::GlobalInstance::CreateVulkanInstance(&vulkanInstanceCreator);
	Graphic::VulkanDeviceCreator vulkanDeviceCreator = Graphic::VulkanDeviceCreator();
	vulkanDeviceCreator.SetDeviceFeature([](VkPhysicalDeviceFeatures& features)
		{
			features.geometryShader = VK_TRUE;
		});
	vulkanDeviceCreator.AddQueue("TransferQueue", VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT, 1.0);
	vulkanDeviceCreator.AddQueue("RenderQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
	vulkanDeviceCreator.AddQueue("ComputeQueue", VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT, 1.0);
	Graphic::GlobalInstance::CreateVulkanDevice(&vulkanDeviceCreator);

	RenderPassCreator renderPassCreator = RenderPassCreator("TestRenderPass");
	renderPassCreator.AddColorAttachment(
		"ColorAttachment",
		GlobalSetting::windowImageFormat,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);
	renderPassCreator.AddSubpassWithColorAttachment(
		"TestSubpass",
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		{ "ColorAttachment" }
	);
	renderPassCreator.AddDependency(
		"VK_SUBPASS_EXTERNAL",
		"TestSubpass",
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	);
	Graphic::GlobalInstance::CreateRenderPass(&renderPassCreator);

	Graphic::DescriptorSetLayout layout = Graphic::DescriptorSetLayoutUtils::CreateDescriptrSetLayout(
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , VK_SHADER_STAGE_VERTEX_BIT },
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
		}
	);
	Graphic::DescriptorSetLayoutUtils::DescoryDescriptorSetLayout(layout);
}

void Graphic::GraphicThread::OnStart()
{

	_stopped = false;

}

void Graphic::GraphicThread::OnRun()
{
	while (!_stopped && !glfwWindowShouldClose(Graphic::GlobalInstance::window))
	{
		glfwPollEvents();
	}
}

void Graphic::GraphicThread::OnEnd()
{
	_stopped = true;
}

#include "Graphic/Instance/DescriptorSet.h"
#include <Graphic/Core/Device.h>
VkDescriptorSet Graphic::Instance::DescriptorSet::VkDescriptorSet_()
{
	return _vkDescriptorSet;
}

Graphic::Instance::DescriptorSet::DescriptorSet(Asset::SlotType slotType, VkDescriptorSet set, VkDescriptorPool sourceDescriptorChunk)
	: _slotType(slotType)
	, _vkDescriptorSet(set)
	, _sourceVkDescriptorChunk(sourceDescriptorChunk)
{
}

Graphic::Instance::DescriptorSet::~DescriptorSet()
{
}

Graphic::Instance::DescriptorSet::DescriptorSetWriteData::DescriptorSetWriteData(VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
	: type(type)
	, buffer(buffer)
	, offset(offset)
	, range(range)
{
}

Graphic::Instance::DescriptorSet::DescriptorSetWriteData::DescriptorSetWriteData(VkDescriptorType type, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
	: type(type)
	, sampler(sampler)
	, imageView(imageView)
	, imageLayout(imageLayout)
{
}

void Graphic::Instance::DescriptorSet::UpdateBindingData(std::vector<uint32_t> bindingIndex, std::vector< Graphic::Instance::DescriptorSet::DescriptorSetWriteData> data)
{
	std::vector< VkDescriptorBufferInfo> bufferInfos = std::vector< VkDescriptorBufferInfo>(data.size());
	std::vector< VkDescriptorImageInfo> imageInfos = std::vector< VkDescriptorImageInfo>(data.size());
	std::vector< VkWriteDescriptorSet> writeInfos = std::vector< VkWriteDescriptorSet>(data.size());

	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			bufferInfos[i].buffer = data[i].buffer;
			bufferInfos[i].offset = data[i].offset;
			bufferInfos[i].range = data[i].range;

			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeInfos[i].dstSet = _vkDescriptorSet;
			writeInfos[i].dstBinding = bindingIndex[i];
			writeInfos[i].dstArrayElement = 0;
			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeInfos[i].descriptorCount = 1;
			writeInfos[i].pBufferInfo = &bufferInfos[i];
		}
		else if (data[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
			imageInfos[i].imageLayout = data[i].imageLayout;
			imageInfos[i].imageView = data[i].imageView;
			imageInfos[i].sampler = data[i].sampler;

			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeInfos[i].dstSet = _vkDescriptorSet;
			writeInfos[i].dstBinding = bindingIndex[i];
			writeInfos[i].dstArrayElement = 0;
			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeInfos[i].descriptorCount = 1;
			writeInfos[i].pImageInfo = &imageInfos[i];
		}
	}
	vkUpdateDescriptorSets(Core::Device::VkDevice_(), static_cast<uint32_t>(writeInfos.size()), writeInfos.data(), 0, nullptr);
}

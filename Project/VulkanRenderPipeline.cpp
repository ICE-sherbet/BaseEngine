#include "VulkanRenderPipeline.h"

#include "RendererApi.h"
#include "VulkanAPI.h"
#include "VulkanContext.h"
#include "VulkanFrameBuffer.h"
#include "VulkanUtilities.h"

namespace base_engine {

namespace Utils {
static VkPrimitiveTopology GetVulkanTopology(PrimitiveTopology topology) {
  switch (topology) {
    case PrimitiveTopology::Points:
      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case PrimitiveTopology::Lines:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PrimitiveTopology::Triangles:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PrimitiveTopology::LineStrip:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case PrimitiveTopology::TriangleStrip:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case PrimitiveTopology::TriangleFan:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
  }

  BE_CORE_ASSERT(false, "Unknown toplogy");
  return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

static VkCompareOp GetVulkanCompareOperator(
    const DepthCompareOperator compareOp) {
  switch (compareOp) {
    case DepthCompareOperator::Never:
      return VK_COMPARE_OP_NEVER;
    case DepthCompareOperator::NotEqual:
      return VK_COMPARE_OP_NOT_EQUAL;
    case DepthCompareOperator::Less:
      return VK_COMPARE_OP_LESS;
    case DepthCompareOperator::LessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case DepthCompareOperator::Greater:
      return VK_COMPARE_OP_GREATER;
    case DepthCompareOperator::GreaterOrEqual:
      return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case DepthCompareOperator::Equal:
      return VK_COMPARE_OP_EQUAL;
    case DepthCompareOperator::Always:
      return VK_COMPARE_OP_ALWAYS;
  }
  BE_CORE_ASSERT(false, "Unknown Operator");
  return VK_COMPARE_OP_MAX_ENUM;
}
}  // namespace Utils

static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type) {
  switch (type) {
    case ShaderDataType::kFloat:
      return VK_FORMAT_R32_SFLOAT;
    case ShaderDataType::kFloat2:
      return VK_FORMAT_R32G32_SFLOAT;
    case ShaderDataType::kFloat3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case ShaderDataType::kFloat4:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ShaderDataType::kInt:
      return VK_FORMAT_R32_SINT;
    case ShaderDataType::kInt2:
      return VK_FORMAT_R32G32_SINT;
    case ShaderDataType::kInt3:
      return VK_FORMAT_R32G32B32_SINT;
    case ShaderDataType::kInt4:
      return VK_FORMAT_R32G32B32A32_SINT;
  }
  BE_CORE_ASSERT(false);
  return VK_FORMAT_UNDEFINED;
}

VulkanRenderPipeline::VulkanRenderPipeline(const PipelineSpecification& spec)
    : specification_(spec) {
  Invalidate();
}

VulkanRenderPipeline::~VulkanRenderPipeline() {
  Renderer::SubmitResourceFree([pipeline = vulkan_pipeline_,
                                pipelineCache = pipeline_cache_,
                                pipelineLayout = pipeline_layout_]() {
    const auto vulkanDevice =
        VulkanContext::GetCurrentDevice()->GetVulkanDevice();
    vkDestroyPipeline(vulkanDevice, pipeline, nullptr);
    vkDestroyPipelineCache(vulkanDevice, pipelineCache, nullptr);
    vkDestroyPipelineLayout(vulkanDevice, pipelineLayout, nullptr);
  });
}

void VulkanRenderPipeline::Invalidate() {
  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
    BE_CORE_ASSERT(instance->specification_.Shader);
    Ref<VulkanShader> vulkan_shader =
        Ref<VulkanShader>(instance->specification_.Shader);
    Ref<VulkanFrameBuffer> framebuffer =
        instance->specification_.TargetFrameBuffer.As<VulkanFrameBuffer>();

    auto descriptorSetLayouts = vulkan_shader->GetAllDescriptorSetLayouts();

    const auto& pushConstantRanges = vulkan_shader->GetPushConstantRanges();

    std::vector<VkPushConstantRange> vulkanPushConstantRanges(
        pushConstantRanges.size());
    for (uint32_t i = 0; i < pushConstantRanges.size(); i++) {
      const auto& pushConstantRange = pushConstantRanges[i];
      auto& vulkanPushConstantRange = vulkanPushConstantRanges[i];

      vulkanPushConstantRange.stageFlags = pushConstantRange.ShaderStage;
      vulkanPushConstantRange.offset = pushConstantRange.Offset;
      vulkanPushConstantRange.size = pushConstantRange.Size;
    }

    {
      VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
      pPipelineLayoutCreateInfo.sType =
          VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pPipelineLayoutCreateInfo.pNext = nullptr;
      pPipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
      pPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
      pPipelineLayoutCreateInfo.pushConstantRangeCount =
          vulkanPushConstantRanges.size();
      pPipelineLayoutCreateInfo.pPushConstantRanges =
          vulkanPushConstantRanges.data();

      vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,
                             &instance->pipeline_layout_);
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.layout = instance->pipeline_layout_;
    pipeline_create_info.renderPass = framebuffer->GetRenderPass();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology =
        Utils::GetVulkanTopology(instance->specification_.Topology);

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = instance->specification_.Wireframe
                                         ? VK_POLYGON_MODE_LINE
                                         : VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = instance->specification_.BackfaceCulling
                                      ? VK_CULL_MODE_BACK_BIT
                                      : VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.lineWidth = instance->specification_.LineWidth;

    size_t colorAttachmentCount =
        framebuffer->GetSpecification().SwapChainTarget
            ? 1
            : framebuffer->GetColorAttachmentCount();
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(
        colorAttachmentCount);
    if (framebuffer->GetSpecification().SwapChainTarget) {
      blendAttachmentStates[0].colorWriteMask = 0xf;
      blendAttachmentStates[0].blendEnable = VK_TRUE;
      blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      blendAttachmentStates[0].dstColorBlendFactor =
          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
      blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
      blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    } else {
      for (size_t i = 0; i < colorAttachmentCount; i++) {
        if (!framebuffer->GetSpecification().Blend) break;

        blendAttachmentStates[i].colorWriteMask = 0xf;
        if (!framebuffer->GetSpecification().Blend) break;

        const auto& attachmentSpec =
            framebuffer->GetSpecification().Attachments.Attachments[i];
        FrameBufferBlendMode blendMode =
            framebuffer->GetSpecification().BlendMode ==
                    FrameBufferBlendMode::None
                ? attachmentSpec.BlendMode
                : framebuffer->GetSpecification().BlendMode;

        blendAttachmentStates[i].blendEnable =
            attachmentSpec.Blend ? VK_TRUE : VK_FALSE;

        blendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

        switch (blendMode) {
          case FrameBufferBlendMode::SrcAlphaOneMinusSrcAlpha:
            blendAttachmentStates[i].srcColorBlendFactor =
                VK_BLEND_FACTOR_SRC_ALPHA;
            blendAttachmentStates[i].dstColorBlendFactor =
                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendAttachmentStates[i].srcAlphaBlendFactor =
                VK_BLEND_FACTOR_SRC_ALPHA;
            blendAttachmentStates[i].dstAlphaBlendFactor =
                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
          case FrameBufferBlendMode::OneZero:
            blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;
          case FrameBufferBlendMode::Zero_SrcColor:
            blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            blendAttachmentStates[i].dstColorBlendFactor =
                VK_BLEND_FACTOR_SRC_COLOR;
            break;

          default:
            BE_CORE_VERIFY(false);
        }
      }
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = (uint32_t)blendAttachmentStates.size();
    colorBlendState.pAttachments = blendAttachmentStates.data();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    if (instance->IsDynamicLineWidth())
      dynamicStateEnables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStateEnables.size());

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable =
        instance->specification_.DepthTest ? VK_TRUE : VK_FALSE;
    depthStencilState.depthWriteEnable =
        instance->specification_.DepthWrite ? VK_TRUE : VK_FALSE;
    depthStencilState.depthCompareOp = Utils::GetVulkanCompareOperator(
        instance->specification_.DepthOperator);
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = depthStencilState.back;

    // Multi sampling state
    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.pSampleMask = nullptr;

    // Vertex input descriptor
    VertexBufferLayout& vertexLayout = instance->specification_.Layout;

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;

    VkVertexInputBindingDescription& vertexInputBinding =
        vertexInputBindingDescriptions.emplace_back();
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = vertexLayout.GetStride();
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> vertex_input_attributes(
        vertexLayout.GetElementCount());

    uint32_t binding = 0;
    uint32_t location = 0;
    for (const auto& layout : {vertexLayout}) {
      for (const auto& element : layout) {
        vertex_input_attributes[location].binding = binding;
        vertex_input_attributes[location].location = location;
        vertex_input_attributes[location].format =
            ShaderDataTypeToVulkanFormat(element.type);
        vertex_input_attributes[location].offset = element.offset;
        location++;
      }
      binding++;
    }

    // Vertex input state used for pipeline creation
    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount =
        (uint32_t)vertexInputBindingDescriptions.size();
    vertexInputState.pVertexBindingDescriptions =
        vertexInputBindingDescriptions.data();
    vertexInputState.vertexAttributeDescriptionCount =
        (uint32_t)vertex_input_attributes.size();
    vertexInputState.pVertexAttributeDescriptions =
        vertex_input_attributes.data();

    const auto& shaderStages =
        vulkan_shader->GetPipelineShaderStageCreateInfos();

    // Set pipeline shader stage info
    pipeline_create_info.stageCount =
        static_cast<uint32_t>(shaderStages.size());
    pipeline_create_info.pStages = shaderStages.data();

    // Assign the pipeline states to the pipeline creation info structure
    pipeline_create_info.pVertexInputState = &vertexInputState;
    pipeline_create_info.pInputAssemblyState = &inputAssemblyState;
    pipeline_create_info.pRasterizationState = &rasterizationState;
    pipeline_create_info.pColorBlendState = &colorBlendState;
    pipeline_create_info.pMultisampleState = &multisampleState;
    pipeline_create_info.pViewportState = &viewportState;
    pipeline_create_info.pDepthStencilState = &depthStencilState;
    pipeline_create_info.renderPass = framebuffer->GetRenderPass();
    pipeline_create_info.pDynamicState = &dynamicState;

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr,
                          &instance->pipeline_cache_);

    // Create rendering pipeline using the specified states
    vkCreateGraphicsPipelines(device, instance->pipeline_cache_, 1,
                              &pipeline_create_info, nullptr,
                              &instance->vulkan_pipeline_);
    vulkan::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_PIPELINE,
                                    instance->specification_.DebugName,
                                    instance->vulkan_pipeline_);
  });
}

bool VulkanRenderPipeline::IsDynamicLineWidth() const {
  return specification_.LineWidth == 0.0f;
}
}  // namespace base_engine

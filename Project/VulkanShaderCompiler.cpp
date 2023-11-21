#include "VulkanShaderCompiler.h"

#include <fstream>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>
#include <shaderc/shaderc.hpp>
#include "Assert.h"
#include "RendererApi.h"
#include "ShaderPreprocessor.h"

namespace base_engine {

namespace {

static ShaderUniformType SPIRTypeToShaderUniformType(
    spirv_cross::SPIRType type) {
  switch (type.basetype) {
    case spirv_cross::SPIRType::Boolean:
      return ShaderUniformType::Bool;
    case spirv_cross::SPIRType::Int:
      if (type.vecsize == 1) return ShaderUniformType::Int;
      if (type.vecsize == 2) return ShaderUniformType::IVec2;
      if (type.vecsize == 3) return ShaderUniformType::IVec3;
      if (type.vecsize == 4) return ShaderUniformType::IVec4;

    case spirv_cross::SPIRType::UInt:
      return ShaderUniformType::UInt;
    case spirv_cross::SPIRType::Float:
      if (type.columns == 3) return ShaderUniformType::Mat3;
      if (type.columns == 4) return ShaderUniformType::Mat4;

      if (type.vecsize == 1) return ShaderUniformType::Float;
      if (type.vecsize == 2) return ShaderUniformType::Vec2;
      if (type.vecsize == 3) return ShaderUniformType::Vec3;
      if (type.vecsize == 4) return ShaderUniformType::Vec4;
      break;
  }
  return ShaderUniformType::None;
}

std::string_view VKStageToShaderMacro(const VkShaderStageFlagBits stage) {
  if (stage == VK_SHADER_STAGE_VERTEX_BIT) return "__VERTEX_STAGE__";
  if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) return "__FRAGMENT_STAGE__";
  if (stage == VK_SHADER_STAGE_COMPUTE_BIT) return "__COMPUTE_STAGE__";
  return "";
}
shaderc_shader_kind ShaderStageToShaderC(const VkShaderStageFlagBits stage) {
  switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
      return shaderc_vertex_shader;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
      return shaderc_fragment_shader;
    case VK_SHADER_STAGE_COMPUTE_BIT:
      return shaderc_compute_shader;
  }
  return {};
}
}  // namespace

static std::unordered_map<uint32_t,
                          std::unordered_map<uint32_t, shader::UniformBuffer>>
    s_UniformBuffers;  // set -> binding point -> buffer
static std::unordered_map<uint32_t,
                          std::unordered_map<uint32_t, shader::StorageBuffer>>
    s_StorageBuffers;  // set -> binding point -> buffer

int SkipBOM(std::istream& in) {
  char test[4] = {0};
  in.seekg(0, std::ios::beg);
  in.read(test, 3);
  if (strcmp(test, "\xEF\xBB\xBF") == 0) {
    in.seekg(3, std::ios::beg);
    return 3;
  }
  in.seekg(0, std::ios::beg);
  return 0;
}

// Returns an empty string when failing.
std::string ReadFileAndSkipBOM(const std::filesystem::path& filepath) {
  std::string result;
  std::ifstream in(filepath, std::ios::in | std::ios::binary);
  if (in) {
    in.seekg(0, std::ios::end);
    auto fileSize = in.tellg();
    const int skippedChars = SkipBOM(in);

    fileSize -= skippedChars - 1;
    result.resize(fileSize);
    in.read(result.data() + 1, fileSize);
    // Add a dummy tab to beginning of file.
    result[0] = '\t';
  }
  in.close();
  return result;
}

VulkanShaderCompiler::VulkanShaderCompiler(
    const std::filesystem::path& shaderSourcePath, bool disableOptimization) {
  m_ShaderSourcePath = shaderSourcePath;
  m_DisableOptimization = disableOptimization;
}

bool VulkanShaderCompiler::Reload(bool forceCompile) {
  m_ShaderSource.clear();
  m_SPIRVDebugData.clear();
  m_SPIRVData.clear();
  const std::string source = ReadFileAndSkipBOM(m_ShaderSourcePath);
  m_ShaderSource = PreProcess(source);
  return true;
}

Ref<VulkanShader> VulkanShaderCompiler::Compile(
    const std::filesystem::path& shaderSourcePath, bool forceCompile,
    bool disableOptimization) {
  // Set name
  std::string path = shaderSourcePath.string();
  size_t found = path.find_last_of("/\\");
  std::string name = found != std::string::npos ? path.substr(found + 1) : path;
  found = name.find_last_of('.');
  name = found != std::string::npos ? name.substr(0, found) : name;

  Ref<VulkanShader> shader = Ref<VulkanShader>::Create();
  shader->m_AssetPath = shaderSourcePath;
  shader->m_Name = name;
  shader->m_DisableOptimization = disableOptimization;

  Ref<VulkanShaderCompiler> compiler =
      Ref<VulkanShaderCompiler>::Create(shaderSourcePath, disableOptimization);
  compiler->Reload(forceCompile);

  shader->LoadAndCreateShaders(compiler->GetSPIRVData());
  shader->CreateDescriptors();
  /*
  Renderer::AcknowledgeParsedGlobalMacros(compiler->GetAcknowledgedMacros(),
                                          shader);
  Renderer::OnShaderReloaded(shader->GetHash());
  */
  return shader;
}

bool VulkanShaderCompiler::TryRecompile(Ref<VulkanShader> shader) {
  Ref<VulkanShaderCompiler> compiler = Ref<VulkanShaderCompiler>::Create(
      shader->m_AssetPath, shader->m_DisableOptimization);
  bool compileSucceeded = compiler->Reload(true);
  if (!compileSucceeded) return false;

  shader->Release();

  return true;
}

void VulkanShaderCompiler::ReflectAllShaderStages(
    const ShaderDate& shaderData) {
  ClearReflectionData();

  for (auto [stage, data] : shaderData) {
    Reflect(stage, data);
  }
}

void VulkanShaderCompiler::Reflect(VkShaderStageFlagBits shaderStage,
                                   const std::vector<uint32_t>& shaderData) {
  spirv_cross::Compiler compiler(shaderData);
  auto resources = compiler.get_shader_resources();

  for (const auto& resource : resources.uniform_buffers) {
    auto activeBuffers = compiler.get_active_buffer_ranges(resource.id);
    // Discard unused buffers from headers
    if (activeBuffers.size()) {
      const auto& name = resource.name;
      auto& bufferType = compiler.get_type(resource.base_type_id);
      int memberCount = (uint32_t)bufferType.member_types.size();
      uint32_t binding =
          compiler.get_decoration(resource.id, spv::DecorationBinding);
      uint32_t descriptorSet =
          compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
      uint32_t size = (uint32_t)compiler.get_declared_struct_size(bufferType);

      if (descriptorSet >= m_ReflectionData.ShaderDescriptorSets.size())
        m_ReflectionData.ShaderDescriptorSets.resize(descriptorSet + 1);

      shader::ShaderDescriptorSet& shaderDescriptorSet =
          m_ReflectionData.ShaderDescriptorSets[descriptorSet];
      if (s_UniformBuffers[descriptorSet].find(binding) ==
          s_UniformBuffers[descriptorSet].end()) {
        shader::UniformBuffer uniformBuffer;
        uniformBuffer.BindingPoint = binding;
        uniformBuffer.Size = size;
        uniformBuffer.Name = name;
        uniformBuffer.ShaderStage = VK_SHADER_STAGE_ALL;
        s_UniformBuffers.at(descriptorSet)[binding] = uniformBuffer;
      } else {
        shader::UniformBuffer& uniformBuffer =
            s_UniformBuffers.at(descriptorSet).at(binding);
        if (size > uniformBuffer.Size) uniformBuffer.Size = size;
      }
      shaderDescriptorSet.UniformBuffers[binding] =
          s_UniformBuffers.at(descriptorSet).at(binding);
    }
  }

  for (const auto& resource : resources.storage_buffers) {
    auto activeBuffers = compiler.get_active_buffer_ranges(resource.id);
    // Discard unused buffers from headers
    if (activeBuffers.size()) {
      const auto& name = resource.name;
      auto& bufferType = compiler.get_type(resource.base_type_id);
      uint32_t memberCount = (uint32_t)bufferType.member_types.size();
      uint32_t binding =
          compiler.get_decoration(resource.id, spv::DecorationBinding);
      uint32_t descriptorSet =
          compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
      uint32_t size = (uint32_t)compiler.get_declared_struct_size(bufferType);

      if (descriptorSet >= m_ReflectionData.ShaderDescriptorSets.size())
        m_ReflectionData.ShaderDescriptorSets.resize(descriptorSet + 1);

      shader::ShaderDescriptorSet& shaderDescriptorSet =
          m_ReflectionData.ShaderDescriptorSets[descriptorSet];
      if (s_StorageBuffers[descriptorSet].find(binding) ==
          s_StorageBuffers[descriptorSet].end()) {
        shader::StorageBuffer storageBuffer;
        storageBuffer.BindingPoint = binding;
        storageBuffer.Size = size;
        storageBuffer.Name = name;
        storageBuffer.ShaderStage = VK_SHADER_STAGE_ALL;
        s_StorageBuffers.at(descriptorSet)[binding] = storageBuffer;
      } else {
        shader::StorageBuffer& storageBuffer =
            s_StorageBuffers.at(descriptorSet).at(binding);
        if (size > storageBuffer.Size) storageBuffer.Size = size;
      }

      shaderDescriptorSet.StorageBuffers[binding] =
          s_StorageBuffers.at(descriptorSet).at(binding);
    }
  }

  for (const auto& resource : resources.push_constant_buffers) {
    const auto& bufferName = resource.name;
    auto& bufferType = compiler.get_type(resource.base_type_id);
    auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
    uint32_t memberCount = uint32_t(bufferType.member_types.size());
    uint32_t bufferOffset = 0;
    if (m_ReflectionData.PushConstantRanges.size())
      bufferOffset = m_ReflectionData.PushConstantRanges.back().Offset +
                     m_ReflectionData.PushConstantRanges.back().Size;

    auto& pushConstantRange =
        m_ReflectionData.PushConstantRanges.emplace_back();
    pushConstantRange.ShaderStage = shaderStage;
    pushConstantRange.Size = bufferSize - bufferOffset;
    pushConstantRange.Offset = bufferOffset;

    // Skip empty push constant buffers - these are for the renderer only
    if (bufferName.empty() || bufferName == "u_Renderer") continue;

    ShaderBuffer& buffer = m_ReflectionData.ConstantBuffers[bufferName];
    buffer.Name = bufferName;
    buffer.Size = bufferSize - bufferOffset;

    for (uint32_t i = 0; i < memberCount; i++) {
      auto type = compiler.get_type(bufferType.member_types[i]);
      const auto& memberName = compiler.get_member_name(bufferType.self, i);
      auto size = compiler.get_declared_struct_member_size(bufferType, i);
      auto offset =
          compiler.type_struct_member_offset(bufferType, i) - bufferOffset;

      std::string uniformName = std::format("{}.{}", bufferName, memberName);
      buffer.Uniforms[uniformName] = ShaderUniform(
          uniformName, SPIRTypeToShaderUniformType(type), size, offset);
    }
  }

  for (const auto& resource : resources.sampled_images) {
    const auto& name = resource.name;
    auto& baseType = compiler.get_type(resource.base_type_id);
    auto& type = compiler.get_type(resource.type_id);
    uint32_t binding =
        compiler.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t descriptorSet =
        compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    uint32_t dimension = baseType.image.dim;
    uint32_t arraySize = type.array[0];
    if (arraySize == 0) arraySize = 1;
    if (descriptorSet >= m_ReflectionData.ShaderDescriptorSets.size())
      m_ReflectionData.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        m_ReflectionData.ShaderDescriptorSets[descriptorSet];
    auto& imageSampler = shaderDescriptorSet.ImageSamplers[binding];
    imageSampler.BindingPoint = binding;
    imageSampler.DescriptorSet = descriptorSet;
    imageSampler.Name = name;
    imageSampler.ShaderStage = shaderStage;
    imageSampler.Dimension = dimension;
    imageSampler.ArraySize = arraySize;
  }

  for (const auto& resource : resources.separate_images) {
    const auto& name = resource.name;
    auto& baseType = compiler.get_type(resource.base_type_id);
    auto& type = compiler.get_type(resource.type_id);
    uint32_t binding =
        compiler.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t descriptorSet =
        compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    uint32_t dimension = baseType.image.dim;
    uint32_t arraySize = type.array[0];
    if (arraySize == 0) arraySize = 1;
    if (descriptorSet >= m_ReflectionData.ShaderDescriptorSets.size())
      m_ReflectionData.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        m_ReflectionData.ShaderDescriptorSets[descriptorSet];
    auto& imageSampler = shaderDescriptorSet.SeparateTextures[binding];
    imageSampler.BindingPoint = binding;
    imageSampler.DescriptorSet = descriptorSet;
    imageSampler.Name = name;
    imageSampler.ShaderStage = shaderStage;
    imageSampler.Dimension = dimension;
    imageSampler.ArraySize = arraySize;
  }

  for (const auto& resource : resources.separate_samplers) {
    const auto& name = resource.name;
    auto& baseType = compiler.get_type(resource.base_type_id);
    auto& type = compiler.get_type(resource.type_id);
    uint32_t binding =
        compiler.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t descriptorSet =
        compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    uint32_t dimension = baseType.image.dim;
    uint32_t arraySize = type.array[0];
    if (arraySize == 0) arraySize = 1;
    if (descriptorSet >= m_ReflectionData.ShaderDescriptorSets.size())
      m_ReflectionData.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        m_ReflectionData.ShaderDescriptorSets[descriptorSet];
    auto& imageSampler = shaderDescriptorSet.SeparateSamplers[binding];
    imageSampler.BindingPoint = binding;
    imageSampler.DescriptorSet = descriptorSet;
    imageSampler.Name = name;
    imageSampler.ShaderStage = shaderStage;
    imageSampler.Dimension = dimension;
    imageSampler.ArraySize = arraySize;
  }

  for (const auto& resource : resources.storage_images) {
    const auto& name = resource.name;
    auto& type = compiler.get_type(resource.type_id);
    uint32_t binding =
        compiler.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t descriptorSet =
        compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    uint32_t dimension = type.image.dim;
    uint32_t arraySize = type.array[0];
    if (arraySize == 0) arraySize = 1;
    if (descriptorSet >= m_ReflectionData.ShaderDescriptorSets.size())
      m_ReflectionData.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        m_ReflectionData.ShaderDescriptorSets[descriptorSet];
    auto& imageSampler = shaderDescriptorSet.StorageImages[binding];
    imageSampler.BindingPoint = binding;
    imageSampler.DescriptorSet = descriptorSet;
    imageSampler.Name = name;
    imageSampler.Dimension = dimension;
    imageSampler.ArraySize = arraySize;
    imageSampler.ShaderStage = shaderStage;
  }
}

std::map<VkShaderStageFlagBits, std::string> VulkanShaderCompiler::PreProcess(
    const std::string& source) {
  std::unordered_set<std::string> specialMacros;
  std::map<VkShaderStageFlagBits, std::string> shaderSources =
      ShaderPreprocessor::PreprocessShader(source, specialMacros);

  static shaderc::Compiler compiler;

  for (auto& [stage, shaderSource] : shaderSources) {
    shaderc::CompileOptions options;
    options.AddMacroDefinition("__GLSL__");
    options.AddMacroDefinition(std::string(VKStageToShaderMacro(stage)));
    const auto preProcessingResult =
        compiler.PreprocessGlsl(shaderSource, ShaderStageToShaderC(stage),
                                m_ShaderSourcePath.string().c_str(), options);
    shaderSource =
        std::string(preProcessingResult.begin(), preProcessingResult.end());
  }
  return shaderSources;
}

void VulkanShaderCompiler::ClearReflectionData() {
  m_ReflectionData.ConstantBuffers.clear();
  m_ReflectionData.PushConstantRanges.clear();
  m_ReflectionData.ShaderDescriptorSets.clear();
}
}  // namespace base_engine

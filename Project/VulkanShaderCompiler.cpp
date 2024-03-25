#include "VulkanShaderCompiler.h"

#include <spirv-tools/libspirv.h>

#include <fstream>
#include <ranges>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Assert.h"
#include "GlslIncluder.h"
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
}  // namespace

static std::unordered_map<uint32_t,
                          std::unordered_map<uint32_t, shader::UniformBuffer>>
    s_UniformBuffers;  // set -> binding point -> buffer
static std::unordered_map<uint32_t,
                          std::unordered_map<uint32_t, shader::StorageBuffer>>
    s_StorageBuffers;  // set -> binding point -> buffer

inline int SkipBOM(std::istream& in) {
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
inline std::string ReadFileAndSkipBOM(const std::filesystem::path& filepath) {
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
  shader_source_path_ = shaderSourcePath;
  disable_optimization_ = disableOptimization;
}

bool VulkanShaderCompiler::Reload(bool forceCompile) {
  shader_source_.clear();
  spirv_debug_data_.clear();
  spirv_data_.clear();
  const std::string source = ReadFileAndSkipBOM(shader_source_path_);
  shader_source_ = PreProcess(source);

  CompileOrGetVulkanBinaries(spirv_data_);
  ReflectAllShaderStages(spirv_data_);
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
  shader->asset_path_ = shaderSourcePath;
  shader->name_ = name;
  shader->disable_optimization_ = disableOptimization;

  Ref<VulkanShaderCompiler> compiler =
      Ref<VulkanShaderCompiler>::Create(shaderSourcePath, disableOptimization);
  compiler->Reload(forceCompile);

  shader->LoadAndCreateShaders(compiler->GetSPIRVData());
  shader->SetReflectionData(compiler->reflection_data_);
  shader->CreateDescriptors();
  return shader;
}

bool VulkanShaderCompiler::TryRecompile(Ref<VulkanShader> shader) {
  Ref<VulkanShaderCompiler> compiler = Ref<VulkanShaderCompiler>::Create(
      shader->asset_path_, shader->disable_optimization_);
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

      if (descriptorSet >= reflection_data_.ShaderDescriptorSets.size())
        reflection_data_.ShaderDescriptorSets.resize(descriptorSet + 1);

      shader::ShaderDescriptorSet& shaderDescriptorSet =
          reflection_data_.ShaderDescriptorSets[descriptorSet];
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

      if (descriptorSet >= reflection_data_.ShaderDescriptorSets.size())
        reflection_data_.ShaderDescriptorSets.resize(descriptorSet + 1);

      shader::ShaderDescriptorSet& shaderDescriptorSet =
          reflection_data_.ShaderDescriptorSets[descriptorSet];
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
    if (reflection_data_.PushConstantRanges.size())
      bufferOffset = reflection_data_.PushConstantRanges.back().Offset +
                     reflection_data_.PushConstantRanges.back().Size;

    auto& pushConstantRange =
        reflection_data_.PushConstantRanges.emplace_back();
    pushConstantRange.ShaderStage = shaderStage;
    pushConstantRange.Size = bufferSize - bufferOffset;
    pushConstantRange.Offset = bufferOffset;

    if (bufferName.empty()) continue;

    ShaderBuffer& buffer = reflection_data_.ConstantBuffers[bufferName];
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
    if (descriptorSet >= reflection_data_.ShaderDescriptorSets.size())
      reflection_data_.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        reflection_data_.ShaderDescriptorSets[descriptorSet];
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
    if (descriptorSet >= reflection_data_.ShaderDescriptorSets.size())
      reflection_data_.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        reflection_data_.ShaderDescriptorSets[descriptorSet];
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
    if (descriptorSet >= reflection_data_.ShaderDescriptorSets.size())
      reflection_data_.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        reflection_data_.ShaderDescriptorSets[descriptorSet];
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
    if (descriptorSet >= reflection_data_.ShaderDescriptorSets.size())
      reflection_data_.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        reflection_data_.ShaderDescriptorSets[descriptorSet];
    auto& imageSampler = shaderDescriptorSet.StorageImages[binding];
    imageSampler.BindingPoint = binding;
    imageSampler.DescriptorSet = descriptorSet;
    imageSampler.Name = name;
    imageSampler.Dimension = dimension;
    imageSampler.ArraySize = arraySize;
    imageSampler.ShaderStage = shaderStage;
  }
  for (const auto& resource : resources.acceleration_structures) {
    const auto& name = resource.name;
    auto& type = compiler.get_type(resource.type_id);
    uint32_t arraySize = type.array[0];
    if (arraySize == 0) arraySize = 1;
    uint32_t binding =
        compiler.get_decoration(resource.id, spv::DecorationBinding);
    uint32_t descriptorSet =
        compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    if (descriptorSet >= reflection_data_.ShaderDescriptorSets.size())
      reflection_data_.ShaderDescriptorSets.resize(descriptorSet + 1);

    shader::ShaderDescriptorSet& shaderDescriptorSet =
        reflection_data_.ShaderDescriptorSets[descriptorSet];
    auto& accelerationStructure =
        shaderDescriptorSet.AccelerationStructures[binding];
    accelerationStructure.ArraySize = arraySize;
    accelerationStructure.BindingPoint = binding;
    accelerationStructure.Name = name;
    accelerationStructure.ShaderStage = shaderStage;
  }
}

bool VulkanShaderCompiler::Compile(std::vector<uint32_t>& outputBinary,
                                   const VkShaderStageFlagBits stage) const {
  const std::string& stageSource = shader_source_.at(stage);

  static shaderc::Compiler compiler;
  shaderc::CompileOptions shaderCOptions;
  shaderCOptions.SetTargetEnvironment(shaderc_target_env_vulkan,
                                      shaderc_env_version_vulkan_1_2);
  shaderCOptions.SetWarningsAsErrors();

  const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
      stageSource, shader_utils::ShaderStageToShaderC(stage),
      shader_source_path_.string().c_str(), shaderCOptions);

  if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
    BE_CORE_ASSERT(
        false,
        fmt::format("{}While compiling shader file: {} \nAt stage: {}",
                    module.GetErrorMessage(), shader_source_path_.string(),
                    shader_utils::ShaderStageToString(stage)));

    return false;
  }

  outputBinary = std::vector(module.begin(), module.end());
  return true;
}

bool VulkanShaderCompiler::CompileOrGetVulkanBinaries(
    ShaderDate& outputBinary) {
  bool compileSucceeded = true;
  for (const auto stage : shader_source_ | std::views::keys) {
    compileSucceeded &= CompileOrGetVulkanBinary(stage, outputBinary[stage]);
  }
  return compileSucceeded;
}

bool VulkanShaderCompiler::CompileOrGetVulkanBinary(
    VkShaderStageFlagBits stage, std::vector<uint32_t>& outputBinary) {
  return Compile(outputBinary, stage);
}

std::map<VkShaderStageFlagBits, std::string> VulkanShaderCompiler::PreProcess(
    const std::string& source) {
  std::unordered_set<std::string> specialMacros;
  std::map<VkShaderStageFlagBits, std::string> shaderSources =
      ShaderPreprocessor::PreprocessShader(source, specialMacros);

  static shaderc::Compiler compiler;
  shaderc_util::FileFinder fileFinder;
  fileFinder.search_path().emplace_back("Resources/");
  for (auto& [stage, shaderSource] : shaderSources) {
    shaderc::CompileOptions options;
    options.AddMacroDefinition("__GLSL__");
    options.AddMacroDefinition(
        std::string(shader_utils::VKStageToShaderMacro(stage)));

    // Deleted by shaderc and created per stage
    GlslIncluder* includer = new GlslIncluder(&fileFinder);

    options.SetIncluder(std::unique_ptr<GlslIncluder>(includer));

    const auto preProcessingResult = compiler.PreprocessGlsl(
        shaderSource, shader_utils::ShaderStageToShaderC(stage),
        shader_source_path_.string().c_str(), options);
    if (preProcessingResult.GetCompilationStatus() !=
        shaderc_compilation_status_success) {
      auto msg = preProcessingResult.GetErrorMessage();
      BE_CORE_ERROR(msg);
      return shaderSources;
    }

    shaderSource =
        std::string(preProcessingResult.begin(), preProcessingResult.end());
  }
  return shaderSources;
}

void VulkanShaderCompiler::ClearReflectionData() {
  reflection_data_.ConstantBuffers.clear();
  reflection_data_.PushConstantRanges.clear();
  reflection_data_.ShaderDescriptorSets.clear();
}
}  // namespace base_engine

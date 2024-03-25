#include "GlslIncluder.h"

#include <libshaderc_util/io_shaderc.h>

#include <fstream>

#include "Log.h"

namespace base_engine {
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

GlslIncluder::GlslIncluder(const shaderc_util::FileFinder* file_finder)
    : m_FileFinder(*file_finder) {}

GlslIncluder::~GlslIncluder() = default;

shaderc_include_result* GlslIncluder::GetInclude(
    const char* requestedPath, const shaderc_include_type type,
    const char* requestingPath, const size_t includeDepth) {
  const std::filesystem::path requestedFullPath =
      (type == shaderc_include_type_relative)
          ? m_FileFinder.FindRelativeReadableFilepath(requestingPath,
                                                      requestedPath)
          : m_FileFinder.FindReadableFilepath(requestedPath);

  auto& [source, sourceHash, stages, isGuarded] =
      m_HeaderCache[requestedFullPath.string()];

  if (source.empty()) {
    source = ReadFileAndSkipBOM(requestedFullPath);
    if (source.empty()) {
      BE_CORE_ERROR("Failed to load included file: {} in {}.",
                    requestedFullPath.string(),
                    static_cast<std::string>(requestingPath));
    }
    sourceHash = std::hash<std::string>{}(source);

    // Can clear "source" in case it has already been included in this stage and
    // is guarded.
    stages = ShaderPreprocessor::PreprocessHeader(
        source, isGuarded, m_ParsedSpecialMacros, m_includeData,
        requestedFullPath);
  } else if (isGuarded) {
    source.clear();
  }

  // Does not emplace if it finds the same include path and same header hash
  // value.
  m_includeData.emplace(IncludeData{requestedFullPath, includeDepth,
                                    type == shaderc_include_type_relative,
                                    isGuarded, sourceHash, stages});

  auto* const container = new std::array<std::string, 2>;
  (*container)[0] = requestedPath;
  (*container)[1] = source;
  auto* const data = new shaderc_include_result;

  data->user_data = container;

  data->source_name = (*container)[0].data();
  data->source_name_length = (*container)[0].size();

  data->content = (*container)[1].data();
  data->content_length = (*container)[1].size();

  return data;
}

void GlslIncluder::ReleaseInclude(shaderc_include_result* data) {
  delete static_cast<std::array<std::string, 2>*>(data->user_data);
  delete data;
}

}  // namespace base_engine

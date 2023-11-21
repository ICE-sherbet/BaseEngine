// @ShaderPreprocessor.h
// @brief
// @author ICE
// @date 2023/11/21
//
// @details

#pragma once
#include <regex>
#include <string>
#include <unordered_set>

#include "VulkanShader.h"

namespace base_engine {
namespace PreprocessUtils {
enum class State : char {
  SlashOC,
  StarIC,
  SingleLineComment,
  MultiLineComment,
  NotAComment
};

template <typename InputIt, typename OutputIt>
void CopyWithoutComments(InputIt first, InputIt last, OutputIt out) {
  State state = State::NotAComment;

  while (first != last) {
    switch (state) {
      case State::SlashOC:
        if (*first == '/')
          state = State::SingleLineComment;
        else if (*first == '*')
          state = State::MultiLineComment;
        else {
          state = State::NotAComment;
          *out++ = '/';
          *out++ = *first;
        }
        break;
      case State::StarIC:
        if (*first == '/')
          state = State::NotAComment;
        else
          state = State::MultiLineComment;
        break;
      case State::NotAComment:
        if (*first == '/')
          state = State::SlashOC;
        else
          *out++ = *first;
        break;
      case State::SingleLineComment:
        if (*first == '\n') {
          state = State::NotAComment;
          *out++ = '\n';
        }
        break;
      case State::MultiLineComment:
        if (*first == '*')
          state = State::StarIC;
        else if (*first == '\n')
          *out++ = '\n';
        break;
    }
    ++first;
  }
}
}  // namespace PreprocessUtils
class ShaderPreprocessor {
 public:
  static std::map<VkShaderStageFlagBits, std::string> PreprocessShader(
      const std::string& source,
      std::unordered_set<std::string>& specialMacros) {
    std::stringstream sourceStream;
    PreprocessUtils::CopyWithoutComments(
        source.begin(), source.end(),
        std::ostream_iterator<char>(sourceStream));
    std::string newSource = sourceStream.str();

    std::map<VkShaderStageFlagBits, std::string> shaderSources;
    std::vector<std::pair<VkShaderStageFlagBits, size_t>> stagePositions;

    size_t startOfStage = 0;
    size_t pos = newSource.find('#');

    const size_t endOfLine = newSource.find_first_of("\r\n", pos) + 1;
    const std::vector<std::string> tokens =
        SplitStringAndKeepDelims(newSource.substr(pos, endOfLine - pos));

    pos = newSource.find('#', pos + 1);

    while (pos != std::string::npos) {
      const size_t endOfLine = newSource.find_first_of("\r\n", pos) + 1;
      std::vector<std::string> tokens =
          SplitStringAndKeepDelims(newSource.substr(pos, endOfLine - pos));

      size_t index = 1;  // Skip #

      if (tokens[index] ==
          "pragma")  // Parse stage. example: #pragma stage : vert
      {
        ++index;
        if (tokens[index] == "stage") {
          ++index;
          // Jump over ':'
          ++index;

          const std::string_view stage = tokens[index];

          auto shaderStage = ShaderTypeFromString(stage);

          stagePositions.emplace_back(shaderStage, startOfStage);
        }
      } else if (tokens[index] == "ifdef") {
        ++index;
        if (tokens[index].rfind("__HZ_", 0) ==
            0)  // Hazel special macros start with "__HZ_"
        {
          specialMacros.emplace(tokens[index]);
        }
      } else if (tokens[index] == "if" || tokens[index] == "define") {
        ++index;
        for (size_t i = index; i < tokens.size(); ++i) {
          if (tokens[i].rfind("__HZ_", 0) ==
              0)  // Hazel special macros start with "__HZ_"
          {
            specialMacros.emplace(tokens[i]);
          }
        }
      } else{
        if (tokens[index] == "version") {
          ++index;
          startOfStage = pos;
        }
      }

      pos = newSource.find('#', pos + 1);
    }

    auto& [firstStage, firstStagePos] = stagePositions[0];
    if (stagePositions.size() > 1) {
      // Get first stage
      const std::string firstStageStr =
          newSource.substr(0, stagePositions[1].second);
      size_t lineCount =
          std::count(firstStageStr.begin(), firstStageStr.end(), '\n') + 1;
      shaderSources[firstStage] = firstStageStr;

      // Get stages in the middle
      for (size_t i = 1; i < stagePositions.size() - 1; ++i) {
        auto& [stage, stagePos] = stagePositions[i];
        std::string stageStr =
            newSource.substr(stagePos, stagePositions[i + 1].second - stagePos);
        const size_t secondLinePos = stageStr.find_first_of('\n', 1) + 1;
        stageStr.insert(secondLinePos, fmt::format("#line {}\n", lineCount));
        shaderSources[stage] = stageStr;
        lineCount += std::count(stageStr.begin(), stageStr.end(), '\n') + 1;
      }

      // Get last stage
      auto& [stage, stagePos] = stagePositions[stagePositions.size() - 1];
      std::string lastStageStr = newSource.substr(stagePos);
      const size_t secondLinePos = lastStageStr.find_first_of('\n', 1) + 1;
      lastStageStr.insert(secondLinePos,
                          fmt::format("#line {}\n", lineCount + 1));
      shaderSources[stage] = lastStageStr;
    } else {
      shaderSources[firstStage] = newSource;
    }

    return shaderSources;
  }

 private:
  static std::vector<std::string> SplitStringAndKeepDelims(std::string str) {
    const static std::regex re(R"((^\W|^\w+)|(\w+)|[:()])",
                               std::regex_constants::optimize);

    std::regex_iterator rit(str.begin(), str.end(), re);
    std::regex_iterator<std::string::iterator> rend;
    std::vector<std::string> result;

    while (rit != rend) {
      result.emplace_back(rit->str());
      ++rit;
    }
    return result;
  }

  inline static VkShaderStageFlagBits ShaderTypeFromString(
      const std::string_view type) {
    if (type == "vert") return VK_SHADER_STAGE_VERTEX_BIT;
    if (type == "frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
    if (type == "comp") return VK_SHADER_STAGE_COMPUTE_BIT;

    return VK_SHADER_STAGE_ALL;
  }
};

}  // namespace base_engine

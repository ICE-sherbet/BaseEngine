// @YAMLArchive.h
// @brief
// @author ICE
// @date 2023/04/18
// 
// @details

#pragma once
#include "FrozenHelper.h"
#include "frozen.h"

namespace frozen {
class YAMLOutputArchive : public frozen::OutputArchive<YAMLOutputArchive> {
 public:
  explicit YAMLOutputArchive(std::ostream& stream)
      : OutputArchive<YAMLOutputArchive>(this), its_writer_(stream) {}

  inline void SaveText(std::string data, std::streamsize size) {
    std::streamsize writtenSize = 0;
    its_writer_ << data << " ";
  }

 private:
  std::ostream& its_writer_;
};
}  // namespace frozen
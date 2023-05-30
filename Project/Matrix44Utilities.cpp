#include "Matrix44Utilities.h"

namespace base_engine::mat {
Matrix44 Inverse(const Matrix44& m) {
  Matrix44 result;
  Mof::CMatrix44Utilities::Inverse(m, result);
  return result;
}
}  // namespace base_engine::mat

#include "Matrix44Utilities.h"

namespace base_engine::mat {
Matrix44 Inverse(const Matrix44& m) {
  Matrix44 result;

#if defined(_USE_MOF_LIBRARY_)
  Mof::CMatrix44Utilities::Inverse(m, result);
#elif defined(_USE_GLM_LIBRARY_)
  result = glm::inverse(m);
#endif

  return result;
}
}  // namespace base_engine::mat

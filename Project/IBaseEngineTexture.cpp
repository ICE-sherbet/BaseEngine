#include "IBaseEngineTexture.h"
#include "TextureMof.h"
namespace base_engine {
IBaseEngineTexture* IBaseEngineTexture::Create() {
#if defined(__MOF__)
	return new TextureMof;
#else
	return nullptr;
#endif

}

IBaseEngineTexture::~IBaseEngineTexture() = default;
}  // namespace base_engine
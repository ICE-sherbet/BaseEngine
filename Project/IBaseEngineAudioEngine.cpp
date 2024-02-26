#include "IBaseEngineAudioEngine.h"


#if defined(__MOF__)
#include "MofAudioEngine.h"
#else
#include "BEAudioEngine.h"
#endif

namespace base_engine {
IBaseEngineAudioEngine* IBaseEngineAudioEngine::Create()
{

#if defined(__MOF__)
	return new MofAudioEngine();
#else
  return new BEAudioEngine();
#endif

}
}  // namespace base_engine

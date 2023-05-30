#include "IBaseEngineAudioEngine.h"

#include "MofAudioEngine.h"

namespace base_engine {
IBaseEngineAudioEngine* IBaseEngineAudioEngine::Create()
{
	return new MofAudioEngine();
}
}  // namespace base_engine

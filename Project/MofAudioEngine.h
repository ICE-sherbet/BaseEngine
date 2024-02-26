// @MofAudioEngine.h
// @brief
// @author ICE
// @date 2023/04/19
// 
// @details

#pragma once
#if defined(__MOF__)

#include "IBaseEngineAudioEngine.h"

namespace base_engine
{
class MofAudioEngine : public IBaseEngineAudioEngine
{
public:
	bool Play(std::shared_ptr<ISoundBuffer> buffer) override;
	bool Stop(std::shared_ptr<ISoundBuffer> buffer) override;
	void Initialize() override;
	Ref<Audio> CreateAudio(const std::filesystem::path& path, int buffer_count) override;
};
}

#endif
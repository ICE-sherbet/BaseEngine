#include "BESoundBuffer.h"

#include <soloud.h>
#include <soloud_wavstream.h>

#include <memory>
#include <string>

namespace base_engine {
class AudioResourceHolder {
 public:
  AudioResourceHolder() = default;

  explicit AudioResourceHolder(const std::string& path) {
    HMODULE hModule = ::GetModuleHandleW(nullptr);

    const auto path_w = std::wstring(path.begin(), path.end());

    if (HRSRC hrs = ::FindResourceW(hModule, &path_w[1], L"FILE")) {
      if (HGLOBAL resource = ::LoadResource(hModule, hrs)) {
        m_pResource = ::LockResource(resource);

        m_size = ::SizeofResource(hModule, hrs);
      }
    }
  }

  ~AudioResourceHolder() {
    m_pResource = nullptr;

    m_size = 0;
  }

  [[nodiscard]] const void* data() const noexcept { return m_pResource; }

  [[nodiscard]] uint64_t size() const noexcept { return m_size; }

 private:
  uint64_t m_size = 0;

  const void* m_pResource = nullptr;
};

BESoundBuffer::BESoundBuffer(SoLoud::Soloud* soloud) { soloud_ = soloud; }

bool BESoundBuffer::Load(const char* path) {
  auto source = std::make_unique<SoLoud::WavStream>();

  const auto resource = AudioResourceHolder{path};

  if (SoLoud::SO_NO_ERROR !=
      source->loadMem(static_cast<const unsigned char*>(resource.data()),
                      static_cast<uint32_t>(resource.size()), false, false)) {
    return false;
  }

  audio_source_ = std::move(source);

  return true;
}

bool BESoundBuffer::Load(const char* path, size_t buffer_count) {
  return Load(path);
}

bool BESoundBuffer::Play()
{
	soloud_->play(*audio_source_);

  return true;
}

bool BESoundBuffer::Stop()
{
	return false;
}

void BESoundBuffer::SetLoop(bool loop)
{
	
}

bool BESoundBuffer::IsLoop()
{
	return false;
}
}  // namespace base_engine

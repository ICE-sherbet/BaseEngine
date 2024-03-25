// @kAnimation.h
// @brief
// @author ICE
// @date 2024/02/27
//
// @details

#pragma once
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "Asset.h"

namespace base_engine {
class MeshSource;

template <typename T>
struct AnimationKey {
  T Value;
  float FrameTime;  // 0.0f = beginning of animation clip, 1.0f = end of
                    // animation clip
  uint32_t Track;

  AnimationKey() = default;
  AnimationKey(const float frameTime, const uint32_t track, const T& value)
      : Value(value), FrameTime(frameTime), Track(track) {}
};
using TranslationKey = AnimationKey<glm::vec3>;
using RotationKey = AnimationKey<glm::quat>;
using ScaleKey = AnimationKey<glm::vec3>;

class Animation {
 public:
  Animation() = default;
  Animation(const float duration, const uint32_t numTracks);

  [[nodiscard]] float GetDuration() const { return m_Duration; }
  [[nodiscard]] uint32_t GetNumTracks() const { return m_NumTracks; }

  void SetKeys(std::vector<TranslationKey> translations,
               std::vector<RotationKey> rotations,
               std::vector<ScaleKey> scales);

  [[nodiscard]] const auto& GetTranslationKeys() const {
    return m_TranslationKeys;
  }
  [[nodiscard]] const auto& GetRotationKeys() const { return m_RotationKeys; }
  [[nodiscard]] const auto& GetScaleKeys() const { return m_ScaleKeys; }

  [[nodiscard]] const glm::vec3& GetRootTranslationStart() const {
    return m_RootTranslationStart;
  }
  [[nodiscard]] const glm::vec3& GetRootTranslationEnd() const {
    return m_RootTranslationEnd;
  }
  [[nodiscard]] const glm::quat& GetRootRotationStart() const {
    return m_RootRotationStart;
  }
  [[nodiscard]] const glm::quat& GetRootRotationEnd() const {
    return m_RootRotationEnd;
  }

 public:
  static constexpr uint32_t MAXBONES = 101;

 private:
  std::vector<TranslationKey> m_TranslationKeys;
  std::vector<RotationKey> m_RotationKeys;
  std::vector<ScaleKey> m_ScaleKeys;
  glm::vec3 m_RootTranslationStart;
  glm::vec3 m_RootTranslationEnd;
  glm::quat m_RootRotationStart;
  glm::quat m_RootRotationEnd;
  float m_Duration;
  uint32_t m_NumTracks;
};

class AnimationAsset : public Asset {
 public:
  AnimationAsset(Ref<MeshSource> animationSource,
                 Ref<MeshSource> skeletonSource, const uint32_t animationIndex,
                 const bool isMaskedRootMotion,
                 const glm::vec3& rootTranslationMask, float rootRotationMask);

  static AssetType GetStaticType() { return AssetType::kAnimation; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }

  Ref<MeshSource> GetAnimationSource() const;
  Ref<MeshSource> GetSkeletonSource() const;
  uint32_t GetAnimationIndex() const;

  bool IsMaskedRootMotion() const;
  const glm::vec3& GetRootTranslationMask() const;
  float GetRootRotationMask() const;

  // Note: can return nullptr (e.g. if dependent assets (e.g. the skeleton
  // source, or the animation source) are deleted _after_ this AnimationAsset
  // has been created.
  const Animation* GetAnimation() const;

 private:
  glm::vec3 m_RootTranslationMask;
  float m_RootRotationMask;
  Ref<MeshSource> m_AnimationSource;  // kAnimation clips don't necessarily have
                                      // to come from the same DCC file as the
                                      // skeleton they are animating.
  Ref<MeshSource>
      m_SkeletonSource;  // For example, the clips could be in one file, and the
                         // "skin" (with skeleton) in a separate file.
  uint32_t m_AnimationIndex;
  bool m_IsMaskedRootMotion;
};

struct LocalTransform {
  glm::vec3 Translation = glm::zero<glm::vec3>();
  glm::quat Rotation = glm::identity<glm::quat>();
  glm::vec3 Scale = glm::one<glm::vec3>();
};

struct Pose {
  LocalTransform RootMotion;
  std::array<LocalTransform, Animation::MAXBONES> BoneTransforms;
  float AnimationDuration = 0.0f;
  float AnimationTimePos = 0.0f;
  uint32_t NumBones = 0;
};

// Caches results of sampling animations to take advantage of fact that usually
// animations play forwards Everything will still work if you play an animation
// backwards, or skip around in it, its just that in the usual situation
// (playing forwards) we can optimize some things
template <typename T>
class SamplingCache {
 public:
  uint32_t GetSize() const {
    return static_cast<uint32_t>(m_Values.size()) / 2;
  }

  void Resize(const uint32_t numTracks) {
    m_Values.resize(
        numTracks * 2,
        T());  // Values vector stores the current and next key for each track,
               // interleaved.  These are the values that we interpolate to
               // sample animation at a given time
    m_FrameTimes.resize(numTracks * 2,
                        0.0f);  // FrameTimes vector stores the frame time for
                                // current and next key.  This is used to figure
                                // out the interpolation between values.
  }

  void Reset(const uint32_t numTracks,
             const std::vector<AnimationKey<T>>& keys) {
    Resize(numTracks);
    for (uint32_t i = 0; i < numTracks; ++i) {
      m_Values[NextIndex(i)] = keys[i].Value;
      m_FrameTimes[NextIndex(i)] = keys[i].FrameTime;
      BE_CORE_ASSERT(m_FrameTimes[NextIndex(i)] == 0.0f);
    }
    m_Cursor = 0;
  }

  // step cache forward to given time, using given key frames
  void Step(const float sampleTime, const std::vector<AnimationKey<T>>& keys) {
    if ((m_Cursor == static_cast<uint32_t>(keys.size())) ||
        (sampleTime < m_PrevSampleTime)) {
      Loop();
    }
    auto track = keys[m_Cursor].Track;
    while (m_FrameTimes[NextIndex(track)] <= sampleTime) {
      m_Values[CurrentIndex(track)] = m_Values[NextIndex(track)];
      m_Values[NextIndex(track)] = keys[m_Cursor].Value;
      m_FrameTimes[CurrentIndex(track)] = m_FrameTimes[NextIndex(track)];
      m_FrameTimes[NextIndex(track)] = keys[m_Cursor].FrameTime;

      if (++m_Cursor == static_cast<uint32_t>(keys.size())) {
        break;
      }
      track = keys[m_Cursor].Track;
    }
    m_PrevSampleTime = sampleTime;
  }

  // loop back to the beginning of animation
  void Loop() {
    m_Cursor = 0;
    for (uint32_t track = 0, N = static_cast<uint32_t>(m_Values.size() / 2);
         track < N; ++track) {
      m_FrameTimes[NextIndex(track)] = 0.0;
    }
    m_PrevSampleTime = 0.0f;
  }

  void InterpolateTranslation(const float sampleTime,
                              LocalTransform* localTransform) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(m_Values.size()); i += 2) {
      const float t = (sampleTime - m_FrameTimes[i]) /
                      (m_FrameTimes[i + 1] - m_FrameTimes[i]);
      localTransform[i / 2].Translation =
          glm::mix(m_Values[i], m_Values[i + 1], t);
    }
  }

  void InterpolateRotation(const float sampleTime,
                           LocalTransform* localTransform) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(m_Values.size()); i += 2) {
      const float t = (sampleTime - m_FrameTimes[i]) /
                      (m_FrameTimes[i + 1] - m_FrameTimes[i]);
      localTransform[i / 2].Rotation =
          glm::slerp(m_Values[i], m_Values[i + 1], t);
    }
  }

  void InterpolateScale(const float sampleTime,
                        LocalTransform* localTransform) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(m_Values.size()); i += 2) {
      const float t = (sampleTime - m_FrameTimes[i]) /
                      (m_FrameTimes[i + 1] - m_FrameTimes[i]);
      localTransform[i / 2].Scale = glm::mix(m_Values[i], m_Values[i + 1], t);
    }
  }

 public:
  static uint32_t CurrentIndex(const uint32_t i) { return 2 * i; }
  static uint32_t NextIndex(const uint32_t i) { return 2 * i + 1; }

 private:
  std::vector<T> m_Values;
  std::vector<float> m_FrameTimes;

  AssetHandle m_AnimationHandle = 0;
  const Animation* m_Animation = nullptr;
  float m_PrevSampleTime = 0.0f;
  uint32_t m_Cursor = 0;
};
using TranslationCache = SamplingCache<glm::vec3>;
using RotationCache = SamplingCache<glm::quat>;
using ScaleCache = SamplingCache<glm::vec3>;

}  // namespace base_engine

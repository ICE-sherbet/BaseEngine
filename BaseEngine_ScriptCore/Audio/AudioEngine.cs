// @AudioEngine.cs
// @brief
// @author ICE
// @date 2023/04/19
// 
// @details

namespace BaseEngine_ScriptCore
{
    public static class AudioEngine
    {
        public static void PlayOneShot(AudioClip clip) => InternalCalls.AudioEnginePlayOneShot(clip.Handle);
        public static void Play(AudioClip clip) => InternalCalls.AudioEnginePlay(clip.Handle);
        public static void PlayLoop(AudioClip clip) => InternalCalls.AudioEnginePlayLoop(clip.Handle);
    }
}
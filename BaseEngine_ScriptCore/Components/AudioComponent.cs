// @AudioComponent.cs
// @brief
// @author ICE
// @date 2023/04/19
// 
// @details

using BaseEngine_ScriptCore.Render;

namespace BaseEngine_ScriptCore.Components
{
    public class AudioComponent : Component
    {
        public AudioClip Audio
        {
            get
            {
                if (!InternalCalls.AudioComponentGetAudio(Entity.ID, out var audioHandle))
                    return null;

                return new AudioClip(audioHandle);
            }

            set => InternalCalls.AudioComponentSetAudio(Entity.ID, in value.handle);
        }
        
        public void Play() => InternalCalls.AudioComponentPlay(Entity.ID,in Audio.handle);
        public void Stop() => InternalCalls.AudioComponentStop(Entity.ID,in Audio.handle);
    }
}
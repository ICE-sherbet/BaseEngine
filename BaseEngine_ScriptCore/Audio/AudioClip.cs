// @AudioClip.cs
// @brief
// @author ICE
// @date 2023/04/19
// 
// @details

namespace BaseEngine_ScriptCore
{
    public class AudioClip
    {
        internal AssetHandle handle;

        public AssetHandle Handle => handle;


        internal AudioClip() { handle = new AssetHandle(); }

        public AudioClip(AssetHandle handle)
        {
            this.handle = handle;
        }
    }
}
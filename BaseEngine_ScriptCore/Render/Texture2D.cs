// @Texture2D.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

using System;

namespace BaseEngine_ScriptCore.Render
{
    public class Texture2D
    {
        internal AssetHandle handle;

        public AssetHandle Handle => handle;
        public uint Width { get; private set; }
        public uint Height { get; private set; }

        internal Texture2D() { handle = new AssetHandle(); }

        public Texture2D(AssetHandle handle)
        {
            this.handle = handle;
            InternalCalls.Texture2DGetSize(handle, out var width, out var height);
            Width = width;
            Height = height;
        }
    }
}
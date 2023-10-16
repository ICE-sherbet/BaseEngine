// @AssetHandle.cs
// @brief
// @author ICE
// @date 2023/04/12
// 
// @details

using System.Runtime.InteropServices;

namespace BaseEngine_ScriptCore
{
    [StructLayout(LayoutKind.Sequential)]
    public struct AssetHandle
    {
        public static readonly AssetHandle Invalid = new AssetHandle(0);

        private ulong Handle;

        public AssetHandle(ulong handle) { Handle = handle; }

        public bool IsValid() => InternalCalls.AssetHandleIsValid(ref this);

        public static implicit operator bool(AssetHandle assetHandle)
        {
            return InternalCalls.AssetHandleIsValid(ref assetHandle);
        }

        public override string ToString() => Handle.ToString();
        public override int GetHashCode() => Handle.GetHashCode();
    }
}
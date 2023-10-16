// @Quaternion.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

using System.Runtime.InteropServices;

namespace BaseEngine_ScriptCore
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion
    {
        float x, y, z, w;
    }
}
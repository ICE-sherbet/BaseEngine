// @Prefab.cs
// @brief
// @author ICE
// @date 2023/05/30
// 
// @details


using BaseEngine_ScriptCore.Components;
using System;
using System.Collections.Generic;
using BaseEngine_ScriptCore;
using System.Xml.Linq;
using System.Reflection;

namespace BaseEngine_ScriptCore
{
    public class Prefab
    {
        internal AssetHandle _Handle;
        public AssetHandle Handle => _Handle;

        internal Prefab() { _Handle = AssetHandle.Invalid; }
        internal Prefab(AssetHandle handle) { _Handle = handle; }

        public static implicit operator bool(Prefab prefab)
        {
            return prefab._Handle;
        }
        

    }
}
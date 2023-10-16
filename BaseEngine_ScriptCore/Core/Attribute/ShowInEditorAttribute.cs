// @ShowInEditorAttribute.cs
// @brief
// @author ICE
// @date 2023/07/05
// 
// @details

using System;

namespace BaseEngine_ScriptCore
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class ShowInEditorAttribute : Attribute
    {
        public string DisplayName;
        public bool ReadOnly;

        public ShowInEditorAttribute(string displayName = "", bool readOnly = false)
        {
            DisplayName = displayName;
            ReadOnly = readOnly;
        }
    }
}
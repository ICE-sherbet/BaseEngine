// @Component.cs
// @brief
// @author ICE
// @date 2023/04/12
// 
// @details

namespace BaseEngine_ScriptCore.Components
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }
}
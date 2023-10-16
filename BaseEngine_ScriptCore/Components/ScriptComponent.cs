// @ScriptComponent.cs
// @brief
// @author ICE
// @date 2023/04/25
// 
// @details

namespace BaseEngine_ScriptCore.Components
{
    public class ScriptComponent : Component
    {
        public object Instance => InternalCalls.ScriptGetInstance(Entity.ID);
    }
}
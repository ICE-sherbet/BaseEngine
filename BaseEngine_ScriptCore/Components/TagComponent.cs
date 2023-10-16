// @TagComponent.cs
// @brief
// @author ICE
// @date 2023/07/11
// 
// @details

namespace BaseEngine_ScriptCore.Components
{
    public class TagComponent : Component
    {
        public string Tag
        {
            get
            {

                InternalCalls.TagComponentGetTag(Entity.ID, out var name);
                return name;
            }
            set => InternalCalls.TagComponentSetTag(Entity.ID, value);
        }
    }
}
// @Circle.cs
// @brief
// @author ICE
// @date 2023/07/10
// 
// @details

namespace BaseEngine_ScriptCore.Components
{
    public class Circle : Component
    {
        public float Radius
        {
            get => InternalCalls.CircleGetRadius(Entity.ID);
            set => InternalCalls.CircleSetRadius(Entity.ID, value);
        }
    }
}
// @VelocityComponent.cs
// @brief
// @author ICE
// @date 2023/07/10
// 
// @details

namespace BaseEngine_ScriptCore.Components
{
    public class VelocityComponent : Component
    {
        public Vector2F Velocity
        {
            get
            {
                InternalCalls.VelocityGetVelocity(Entity.ID, out var result);
                return result;
            }

            set => InternalCalls.VelocitySetVelocity(Entity.ID, in value);
        }
    }
}
// @TransformComponent.cs
// @brief
// @author ICE
// @date 2023/04/13
// 
// @details

using BaseEngine_ScriptCore;

namespace BaseEngine_ScriptCore.Components
{
    public class TransformComponent : Component
    {
        public Vector3F Translation
        {
            get
            {
                InternalCalls.TransformComponentGetTranslation(Entity.ID, out var result);
                return result;
            }

            set => InternalCalls.TransformComponentSetTranslation(Entity.ID, in value);
        }

        public Vector3F LocalTranslation
        {
            get
            {
                InternalCalls.TransformComponentGetLocalTranslation(Entity.ID, out var result);
                return result;
            }

            set => InternalCalls.TransformComponentSetLocalTranslation(Entity.ID, in value);
        }

        public Vector3F LocalEulerAngles
        {
            get
            {
                InternalCalls.TransformComponentGetLocalEulerAngles(Entity.ID, out var result);
                return result;
            }

            set => InternalCalls.TransformComponentSetLocalEulerAngles(Entity.ID, in value);
        }

        public Vector3F LocalScale
        {
            get
            {
                InternalCalls.TransformComponentGetLocalEulerAngles(Entity.ID, out var result);
                return result;
            }

            set => InternalCalls.TransformComponentSetLocalEulerAngles(Entity.ID, in value);
        }
    }
}
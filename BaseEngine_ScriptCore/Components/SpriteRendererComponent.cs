// @SpriteRendererComponent.cs
// @brief
// @author ICE
// @date 2023/04/12
// 
// @details

using BaseEngine_ScriptCore.Render;

namespace BaseEngine_ScriptCore.Components
{
    public class SpriteRendererComponent : Component
    {
        public Texture2D Texture
        {
            get
            {
                if (!InternalCalls.SpriteRendererComponentGetTexture(Entity.ID, out var textureHandle))
                    return null;

                return new Texture2D(textureHandle);
            }

            set => InternalCalls.SpriteRendererComponentSetTexture(Entity.ID, in value.handle);
        }
    }

}
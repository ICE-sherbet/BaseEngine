// @BulletController.cs
// @brief
// @author ICE
// @date 2023/05/29
// 
// @details

using System;
using BaseEngine_ScriptCore;

namespace Demo.DemoBehavior.Shooting
{
    public class BulletController : Entity
    {
        void OnUpdate(float ts)
        {
            if (Transform.Translation.y < -20)
            {
                Scene.DestroyEntity(this);
                return;
            }
            if (Transform.Translation.y > 1000)
            {
                Scene.DestroyEntity(this);
                return;
            }
        }
    }
}
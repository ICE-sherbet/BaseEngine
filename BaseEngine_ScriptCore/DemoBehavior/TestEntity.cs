// @TestEntity.cs
// @brief
// @author ICE
// @date 2023/04/25
// 
// @details

using System;
using BaseEngine_ScriptCore;

namespace Demo.DemoBehavior
{
    public class TestEntity : Entity
    {
        private float _counter = 0;

        private void OnUpdate(float ts)
        {
            _counter += 0.01f;
            Transform.LocalTranslation += new Vector3F(_counter, _counter, 0);
        }
    }
}
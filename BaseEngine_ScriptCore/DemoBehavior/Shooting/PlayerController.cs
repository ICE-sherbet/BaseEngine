// @PlayerController.cs
// @brief
// @author ICE
// @date 2023/05/29
// 
// @details

using BaseEngine_ScriptCore;
using BaseEngine_ScriptCore.Render;
using System.Collections.Generic;
using BaseEngine_ScriptCore.Components;

namespace Demo.DemoBehavior.Shooting
{
    public class PlayerController : Entity
    {
        [ShowInEditor] private float _speed = 10.0f;

        [ShowInEditor] private float _shotInterval = 0.1f;
        [ShowInEditor] private float _shotIntervalCounter = 0f;
        [ShowInEditor] private Prefab _bulletPrefab = new Prefab();

        /// <summary>
        /// Create
        /// </summary>
        private void OnCreate()
        {
            OnCollisionEnterEvent += OnCollisionEnter;
        }

        private void OnUpdate(float ts)
        {
            _shotIntervalCounter += ts;

            var move = new Vector3F(0, 0, 0);
            if (Input.GetKey(KeyCode.A))
            {
                move += new Vector3F(-_speed, 0, 0);
            }
            if (Input.GetKey(KeyCode.D))
            {
                move += new Vector3F(_speed, 0, 0);
            }
            if (Input.GetKey(KeyCode.W))
            {
                move += new Vector3F(0, -_speed, 0);
            }
            if (Input.GetKey(KeyCode.S))
            {
                move += new Vector3F(0, _speed, 0);
            }

            if (_shotIntervalCounter>_shotInterval && Input.GetKey(KeyCode.Shift))
            {
                _shotIntervalCounter = 0f;
                var bullet = Scene.InstantiatePrefab(_bulletPrefab);
                bullet.Transform.Translation = Transform.Translation;
                bullet.GetComponent<VelocityComponent>().Velocity = new Vector2F(0, -1000);
            }
            Transform.Translation += move;
        }

        void OnCollisionEnter(Entity entity)
        {
            Scene.DestroyEntity(this);
            Scene.DestroyEntity(entity);
        }
    }
}
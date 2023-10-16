// @EnemyController.cs
// @brief
// @author ICE
// @date 2023/05/30
// 
// @details

using System;
using BaseEngine_ScriptCore;
using BaseEngine_ScriptCore.Components;

namespace Demo.DemoBehavior.Shooting
{
    public class EnemyController : Entity
    {
        private float _speed = 5.0f;

        private float _shotInterval = 3f;
        private float _shotIntervalCounter = 0f;
        private Prefab _bulletPrefab = new Prefab();

        void OnCreate()
        {
            OnCollisionEnterEvent += OnCollisionEnter;
        }

        void OnUpdate(float ts)
        {
            _shotIntervalCounter += ts;
            Transform.Translation += (new Vector3F(0, 1, 0) * _speed);

            if (_shotIntervalCounter > _shotInterval)
            {
                _shotIntervalCounter = 0;

                var player = Scene.FindEntityByTag("Player");
                if (player == null) return;

                var bullet_entity = Scene.InstantiatePrefab(_bulletPrefab);
                var player_pos = player.Transform.Translation;
                var enemy_pos = Transform.Translation;
                var z = (float)Math.Atan2(player_pos.y - enemy_pos.y, player_pos.x - enemy_pos.x);
                var bullet_component = InternalCalls.ScriptGetInstance(bullet_entity.ID) as BulletController;
                bullet_entity.Transform.LocalEulerAngles = new Vector3F(0, 0, z * 180f / 3.14f);
                bullet_entity.Transform.Translation = Transform.Translation;
            }
            if (Transform.Translation.y > 1000)
            {
                Scene.DestroyEntity(this);
            }
        }

        void OnCollisionEnter(Entity entity)
        {
            Scene.DestroyEntity(entity);
            Scene.DestroyEntity(this);
        }
    }
}
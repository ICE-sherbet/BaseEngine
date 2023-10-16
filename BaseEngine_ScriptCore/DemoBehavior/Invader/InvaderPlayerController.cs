// @InvaderPlayerController.cs
// @brief
// @author ICE
// @date 2023/07/11
// 
// @details

using BaseEngine_ScriptCore;
using BaseEngine_ScriptCore.Components;

namespace Demo.DemoBehavior.Invader
{
    public class InvaderPlayerController : Entity
    {
        [ShowInEditor] private float _speed = 0;
        [ShowInEditor] private Prefab _bulletPrefab = new Prefab();
        private float _shotInterval = 0;
        [ShowInEditor] private float _shotIntervalMax;
        void OnStart()
        {
            //OnCollisionEnterEvent += OnHit;
        }

        void OnUpdate(float ts)
        {
            _shotInterval += ts;
            var move = new Vector3F(0, 0, 0);
            if (Input.GetKey(KeyCode.A))
            {
                move += new Vector3F(-_speed, 0, 0);
            }

            if (Input.GetKey(KeyCode.D))
            {
                move += new Vector3F(_speed, 0, 0);
            }

            if (_shotInterval >_shotIntervalMax && Input.GetKey(KeyCode.Space))
            {
                _shotInterval = 0;
                Shot();
            }


            Transform.Translation += move;
        }

        private void Shot()
        {
            var bullet = Scene.InstantiatePrefab(_bulletPrefab);
            bullet.Transform.Translation = Transform.Translation;
            bullet.GetComponent<VelocityComponent>().Velocity = new Vector2F(0, -1000);
        }


        private void OnHit(Entity target)
        {
            if (target.Name == "EnemyBullet")
            {
                Death();
            }
        }

        private void Death()
        {
            Scene.DestroyEntity(this);
        }
    }
}
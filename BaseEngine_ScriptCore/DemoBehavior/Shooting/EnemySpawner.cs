// @EnemySpawner.cs
// @brief
// @author ICE
// @date 2023/05/30
// 
// @details

using System;
using BaseEngine_ScriptCore;

namespace Demo.DemoBehavior.Shooting
{
    public class EnemySpawner : Entity
    {
        private Prefab _enemyPrefab = new Prefab();
        private float _spawnInterval = 2.0f;
        private float _spawnIntervalCounter = 0f;

        void OnUpdate(float ts)
        {
            _spawnIntervalCounter += ts;
            if (!(_spawnIntervalCounter > _spawnInterval)) return;
            _spawnIntervalCounter = 0f;
            Spawn();
        }

        private void Spawn()
        {
            var enemyLeft = Scene.InstantiatePrefab(_enemyPrefab);
            enemyLeft.Transform.Translation = new Vector3F(100, -100, 0);
            var enemyRight = Scene.InstantiatePrefab(_enemyPrefab);
            enemyRight.Transform.Translation = new Vector3F(900, -100, 0);
        }
    }
}
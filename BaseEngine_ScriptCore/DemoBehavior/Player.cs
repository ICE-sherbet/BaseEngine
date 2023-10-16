// @Player.cs
// @brief
// @author ICE
// @date 2023/04/17
// 
// @details

using System;
using System.Collections.Generic;
using BaseEngine_ScriptCore;
using BaseEngine_ScriptCore.Components;

namespace Demo.DemoBehavior
{
    public class Player : Entity
    {
        [ShowInEditor("")]  private float _speed = 10.0f;

        private List<Entity> _entities = new List<Entity>();

        private bool _worldStay = true;

        private void OnCreate()
        {
            Debug.Log("Player Create log Test.");
            OnCollisionEnterEvent += OnCollisionEnter;
        }

        private void OnCollisionEnter(Entity entity)
        {
            Debug.Log("Player OnCollisionEnter log Test.");
        }
        private void OnUpdate(float ts)
        {
            var move = new Vector3F(0, 0, 0);
            if (Input.GetKey(KeyCode.A))
            {
                move.x -= _speed;
            }

            if (Input.GetKey(KeyCode.D))
            {
                move.x += _speed;
            }

            if (Input.GetKey(KeyCode.W))
            {
                move.y -= _speed;
            }

            if (Input.GetKey(KeyCode.S))
            {
                move.y += _speed;
            }

            if (Input.GetKeyDown(KeyCode.R))
            {
                _worldStay = !_worldStay;
            }

            if (Input.GetKeyDown(KeyCode.F))
            {
                TestLog();
            }

            if (Input.GetKeyDown(KeyCode.G))
            {
                var entity = Scene.CreateEntity<TestEntity>("TestEntity");
                entity.Transform.Translation = Transform.Translation;
                entity.SetParent(this,true);
                var spriteRenderer = entity.AddComponent<SpriteRendererComponent>();
                spriteRenderer.Texture = GetComponent<SpriteRendererComponent>().Texture;
            }
            if (Input.GetKeyDown(KeyCode.Q))
            {
                var entity = Scene.CreateEntity("TestLog");
                entity.Transform.Translation = Transform.Translation;
                var spriteRenderer = entity.AddComponent<SpriteRendererComponent>();
                spriteRenderer.Texture = GetComponent<SpriteRendererComponent>().Texture;

                AudioEngine.PlayOneShot(GetComponent<AudioComponent>().Audio);
                _entities.Add(entity);
                var findEntity = Scene.FindEntityByTag("Player2");
                entity.SetParent(findEntity, _worldStay);
            }

            if (Input.GetKeyDown(KeyCode.E))
            {
                Scene.DestroyEntity(_entities[0]);
                _entities.RemoveAt(0);
            }
            if (Input.GetKey(KeyCode.Z))
            {
                 var entity = Scene.FindEntityByTag("Player2");
                 entity.Transform.Translation += move;
            }
            else
            {
                Transform.Translation = Transform.Translation + move;
            }

            if (Input.GetKey(KeyCode.Shift))
            {
                var entity = Scene.FindEntityByTag("Player2");
                entity.Transform.LocalEulerAngles += new Vector3F(0,0,0.1f);
            }
            if (Input.GetKey(KeyCode.Space))
            {
                Transform.LocalEulerAngles += new Vector3F(0, 0, 0.01f);
            }
        }

        void TestLog()
        {
            Debug.Log("Player log Test.");
        }
    }
}
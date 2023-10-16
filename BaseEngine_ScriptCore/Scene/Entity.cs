// @Entity.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

using BaseEngine_ScriptCore.Components;
using System;
using System.Collections.Generic;
using BaseEngine_ScriptCore;
using System.Xml.Linq;
using System.Reflection;

namespace BaseEngine_ScriptCore
{
    public class Entity
    {

        protected event Action<Entity> OnCollisionEnterEvent;
        public ulong ID { get; private set; }

        public string Name
        {
            get
            {
                InternalCalls.TagComponentGetTag(ID, out var name);
                return name;
            }
            set => InternalCalls.TagComponentSetTag(ID, value);
        }

        private Dictionary<Type, Component> _componentCache = new Dictionary<Type, Component>();
        private TransformComponent _transformComponent;

        public TransformComponent Transform
        {
            get
            {
                if (_transformComponent == null)
                    _transformComponent = GetComponent<TransformComponent>();

                return _transformComponent;
            }
        }

        public bool HasComponent<T>() where T : Component => InternalCalls.EntityHasComponent(ID, typeof(T));

        public T GetComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);

            if (!HasComponent<T>())
            {
                if (_componentCache.ContainsKey(componentType))
                    _componentCache.Remove(componentType);

                return null;
            }

            if (!_componentCache.ContainsKey(componentType))
            {
                var component = new T { Entity = this };
                _componentCache.Add(componentType, component);
                return component;
            }

            return _componentCache[componentType] as T;
        }
        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return GetComponent<T>();

            var componentType = typeof(T);
            InternalCalls.EntityAddComponent(ID, componentType);
            var component = new T { Entity = this };
            _componentCache.Add(componentType, component);
            return component;
        }
        protected Entity() { ID = 0; }

        internal static T Create<T>(ulong id) where T : Entity, new()
        {
            var instance = new T { ID = id };
            return instance;
        }
        internal Entity(ulong id) { ID = id; }


        /// <summary>
        /// 指定の Entity を親として登録し、親の子として登録される
        /// </summary>
        /// <param name="parent">親要素</param>
        public void SetParent(Entity parent) => SetParent(parent, true);

        /// <summary>
        /// 指定の Entity を親として登録し、親の子として登録される
        /// </summary>
        /// <param name="parent">親要素</param>
        /// <param name="worldPositionStays">もし true の場合、ワールド座標を維持し、親要素からの相対的な位置などが変更される。</param>
        public void SetParent(Entity parent, bool worldPositionStays) => InternalCalls.EntitySetParent(ID, parent.ID, worldPositionStays);

        /// <summary>
        /// インデックスから子Entityを取得
        /// 負の値を指定することで、最後の子からアクセスを行う。
        /// </summary>
        /// <param name="index">子の番号</param>
        /// <returns>指定されたインデックス <paramref name="index" /> の <see cref="Entity"/></returns>
        public Entity GetChild(int index) => new Entity(InternalCalls.EntityGetChild(ID, index));

        /// <summary>
        /// インデックスから子Entityを取得
        /// 負の値を指定することで、最後の子からアクセスを行う。
        /// </summary>
        /// <typeparam name="T"><see cref="Entity"/>から派生した、キャスト可能な型</typeparam>
        /// <param name="index">子の番号</param>
        /// <returns>指定されたインデックス <paramref name="index" /> の <see cref="Entity"/></returns>
        public T GetChild<T>(int index) where T : class => this.GetChild(index) as T;


        private void OnCollisionEnterInternal(ulong id) => OnCollisionEnterEvent?.Invoke(new Entity(id));
    }
}
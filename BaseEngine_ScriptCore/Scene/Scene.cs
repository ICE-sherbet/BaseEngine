// @Scene.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

using System;

namespace BaseEngine_ScriptCore
{
    public class Scene
    {
        /// <summary>
        /// Entityの生成を行う
        /// </summary>
        /// <param name="tag">Entityの名前</param>
        /// <returns>生成されたEntity</returns>
        public static Entity CreateEntity(string tag = "Unnamed") => new Entity(InternalCalls.SceneCreateEntity(tag));

        /// <summary>
        /// Entityの生成を行う
        /// </summary>
        /// <typeparam name="T">Entity 派生型</typeparam>
        /// <param name="tag">Entityの名前</param>
        /// <returns>生成されたEntity</returns>
        public static T CreateEntity<T>(string tag = "Unnamed") where T : Entity, new()
        {
            return Entity.Create<T>(InternalCalls.SceneCreateEntityByScript(tag, typeof(T)));
        }

        /// <summary>
        /// Entityの検索を行う
        /// </summary>
        /// <param name="tag">検索したい名前</param>
        /// <returns>存在する場合、最初に見つかったEntity。\n
        /// 存在しない場合、nullを返す</returns>
        public static Entity FindEntityByTag(string tag)
        {
            var entityId = InternalCalls.SceneFindEntityByTag(tag);
            var newEntity = entityId != 0 ? new Entity(entityId) : null;
            return newEntity;
        }

        /// <summary>
        /// Entityの破棄を行う
        /// </summary>
        /// <param name="entity">破棄したいEntity</param>
        public static void DestroyEntity(Entity entity)
        {
            InternalCalls.SceneDestroyEntity(entity.ID);
        }

        /// <summary>
        /// プレハブからEntityを生成する
        /// </summary>
        /// <param name="prefab">生成に利用するプレハブ</param>
        /// <returns>生成されたEntity</returns>
        public static Entity InstantiatePrefab(Prefab prefab)
        {
            var entityId = InternalCalls.SceneInstantiatePrefab(ref prefab._Handle);
            return entityId == 0 ? null : new Entity(entityId);
        }
    }
}
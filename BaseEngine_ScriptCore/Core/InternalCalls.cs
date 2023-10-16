// @InternalCalls.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

using System;
using System.Runtime.CompilerServices;

namespace BaseEngine_ScriptCore
{
    internal static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TestMethod();

        #region Asset

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AssetHandleIsValid(ref AssetHandle assetHandle);

        #endregion

        #region Audio

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioComponentGetAudio(ulong entityId, out AssetHandle assetHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioComponentSetAudio(ulong entityId, in AssetHandle assetHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioComponentPlay(ulong entityId, in AssetHandle assetHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioComponentStop(ulong entityId, in AssetHandle assetHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioEnginePlayOneShot(in AssetHandle assetHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioEnginePlay(in AssetHandle assetHandle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AudioEnginePlayLoop(in AssetHandle assetHandle);


        #endregion

        #region Debug

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void DebugLog(string message);

        #endregion

        #region Entity

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool EntityHasComponent(ulong entityId, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void EntityAddComponent(ulong entityId, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong EntitySetParent(ulong entityId, ulong parent, bool worldPositionStays);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong EntityGetChild(ulong entityId, int index);

        #endregion

        #region RigidBody

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void VelocityGetVelocity(ulong entityId, out Vector2F v);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void VelocitySetVelocity(ulong entityId, in Vector2F v);


        #endregion

        #region Shapes

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void CircleSetRadius(ulong entityId, float radius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float CircleGetRadius(ulong entityId);

        #endregion

        #region Scene

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong SceneCreateEntity(string tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong SceneCreateEntityByScript(string tag, Type scripType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong SceneFindEntityByTag(string tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SceneDestroyEntity(ulong tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern ulong SceneInstantiatePrefab(ref AssetHandle assetHandle);

        #endregion

        #region Script

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern object ScriptGetInstance(ulong entityId);

        #endregion

        #region Tag

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TagComponentGetTag(ulong entityId, out string tag);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TagComponentSetTag(ulong entityId, in string tag);

        #endregion

        #region Texture

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Texture2DGetSize(in AssetHandle handle, out uint w, out uint h);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool SpriteRendererComponentGetTexture(ulong id, out AssetHandle assetHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SpriteRendererComponentSetTexture(ulong id, in AssetHandle assetHandle);

        #endregion

        #region Transform

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentGetTranslation(ulong entityId, out Vector3F vector3F);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentSetTranslation(ulong entityId, in Vector3F translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentGetLocalTranslation(ulong entityId, out Vector3F vector3F);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentSetLocalTranslation(ulong entityId, in Vector3F translation);


        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentGetLocalEulerAngles(ulong entityId, out Vector3F eulerAngle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentSetLocalEulerAngles(ulong entityId, in Vector3F eulerAngle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentGetLocalScale(ulong entityId, out Vector3F scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TransformComponentSetLocalScale(ulong entityId, in Vector3F scale);

        #endregion

        #region Input

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool InputGetKeyDown(KeyCode keycode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool InputGetKey(KeyCode keycode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool InputGetKeyUp(KeyCode keycode);

        #endregion


    }
}
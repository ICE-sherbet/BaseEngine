// @Input.cs
// @brief
// @author ICE
// @date 2023/04/18
// 
// @details

namespace BaseEngine_ScriptCore
{
    public static class Input
    {


        /// <summary>
        /// 任意のキーが押されたフレーム時のみTrueを返す
        /// </summary>
        /// <param name="keycode">任意のキーコード</param>
        /// <returns>押されたかどうか</returns>
		public static bool GetKeyDown(KeyCode keycode) => InternalCalls.InputGetKeyDown(keycode);

        /// <summary>
        /// 任意のキーが押されている間ずっとTrueを返す
        /// </summary>
        /// <param name="keycode">任意のキーコード</param>
        /// <returns>押されているかどうか</returns>
        public static bool GetKey(KeyCode keycode) => InternalCalls.InputGetKey(keycode);

        /// <summary>
        /// 任意のキーが押されてから離されたフレーム時のみTrueを返す
        /// </summary>
        /// <param name="keycode">任意のキーコード</param>
        /// <returns>離したかどうか</returns>
        public static bool GetKeyUp(KeyCode keycode) => InternalCalls.InputGetKeyUp(keycode);

    }
}
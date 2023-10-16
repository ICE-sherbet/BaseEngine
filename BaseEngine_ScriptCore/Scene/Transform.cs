// @Transform.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

namespace BaseEngine_ScriptCore
{
    public class Transform
    {
        public Vector3F position; 
        public Vector3F scale;
        
        public Vector3F rotation_euler;
        private Quaternion rotation;
    }
}
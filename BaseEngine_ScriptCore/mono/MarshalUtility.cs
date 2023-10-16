// @MarshalUtility.cs
// @brief
// @author ICE
// @date 2023/06/25
// 
// @details


namespace BaseEngine_ScriptCore.mono
{
    public class MarshalUtility
    {
        public enum MarshalType
        {
            Nil,
            Void,
            Bool,
            Int8,
            Int16,
            Int32,
            Int64,
            UInt8,
            UInt16,
            UInt32,
            UInt64,

            Float,
            Double,

            String,

            RECT2D,
            RECT2F,
            RECT2I,

            VECTOR2F,
            VECTOR3F,
            VECTOR4F,

            COLOR,

            AssetHandle,

            MANAGED,

        }
        public static void GetVariantType()
        {

        }
    }
}
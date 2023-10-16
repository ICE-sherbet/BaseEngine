// @Vector2.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

namespace BaseEngine_ScriptCore
{
    public struct Vector2I
    {
        int x, y;
    }
    public struct Vector2F
    {
        public float x, y;

        public Vector2F(float x, float y)
        {
            this.x = x;
            this.y = y;
        }
    }

    public struct Vector2D
    {
        double x, y;
    }
}
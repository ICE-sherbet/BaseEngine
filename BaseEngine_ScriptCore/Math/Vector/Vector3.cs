// @Vector3.cs
// @brief
// @author ICE
// @date 2023/04/11
// 
// @details

using System;

namespace BaseEngine_ScriptCore
{
    public struct Vector3F : IEquatable<Vector3F>
    {
        public float x, y, z;

        public Vector3F(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public bool Equals(Vector3F other)
        {
            return x.Equals(other.x) && y.Equals(other.y) && z.Equals(other.z);
        }

        public override bool Equals(object obj)
        {
            return obj is Vector3F other && Equals(other);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = x.GetHashCode();
                hashCode = (hashCode * 397) ^ y.GetHashCode();
                hashCode = (hashCode * 397) ^ z.GetHashCode();
                return hashCode;
            }
        }

        public static Vector3F Zero = new Vector3F(0, 0, 0);
        public static Vector3F One = new Vector3F(1, 1, 1);

        public static Vector3F Right = new Vector3F(1, 0, 0);
        public static Vector3F Left = new Vector3F(-1, 0, 0);

        public static Vector3F Up = new Vector3F(0, 1, 0);
        public static Vector3F Down = new Vector3F(0, -1, 0);

        public static Vector3F Forward = new Vector3F(0, 0, 1);
        public static Vector3F Back = new Vector3F(0, 0, -1);

        public static Vector3F operator +(Vector3F left, Vector3F right) => new Vector3F(left.x + right.x, left.y + right.y, left.z + right.z);
        public static Vector3F operator -(Vector3F left, Vector3F right) => new Vector3F(left.x - right.x, left.y - right.y, left.z - right.z);
        public static Vector3F operator *(Vector3F left, Vector3F right) => new Vector3F(left.x * right.x, left.y * right.y, left.z * right.z);
        public static Vector3F operator /(Vector3F left, Vector3F right) => new Vector3F(left.x / right.x, left.y / right.y, left.z / right.z);

        public static Vector3F operator +(Vector3F left, float right) => new Vector3F(left.x + right, left.y + right, left.z + right);
        public static Vector3F operator -(Vector3F left, float right) => new Vector3F(left.x - right, left.y - right, left.z - right);
        public static Vector3F operator *(Vector3F left, float right) => new Vector3F(left.x * right, left.y * right, left.z * right);
        public static Vector3F operator /(Vector3F left, float right) => new Vector3F(left.x / right, left.y / right, left.z / right);

        public static Vector3F operator +(float left, Vector3F right) => new Vector3F(left + right.x, left + right.y, left + right.z);
        public static Vector3F operator -(float left, Vector3F right) => new Vector3F(left - right.x, left - right.y, left - right.z);
        public static Vector3F operator *(float left, Vector3F right) => new Vector3F(left * right.x, left * right.y, left * right.z);
        public static Vector3F operator /(float left, Vector3F right) => new Vector3F(left / right.x, left / right.y, left / right.z);

        public static Vector3F operator -(Vector3F value) => new Vector3F(-value.x, -value.y, -value.z);

        public static bool operator ==(Vector3F left, Vector3F right) => left.Equals(right);
        public static bool operator !=(Vector3F left, Vector3F right) => !left.Equals(right);
    }
}
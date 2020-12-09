
/*

  https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
  https://iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm
  
 */

//
// NOTE: Helper math
//

mat2 RotationM2(float Theta)
{
    mat2 Result = mat2(cos(Theta), -sin(Theta), sin(Theta), cos(Theta));
    return Result;
}

vec3 Rotate(vec3 Pos, float ThetaX, float ThetaY, float ThetaZ)
{
    mat4 RotX, RotY, RotZ;

    {
        float c = cos(ThetaX);
        float s = sin(ThetaX);
        RotX = mat4(vec4(1, 0, 0, 0),
                    vec4(0, c, -s, 0),
                    vec4(0, s, c, 0),
                    vec4(0, 0, 0, 1));
    }

    {
        float c = cos(ThetaY);
        float s = sin(ThetaY);
        RotY = mat4(vec4(c, 0, s, 0),
                    vec4(0, 1, 0, 0),
                    vec4(-s, 0, c, 0),
                    vec4(0, 0, 0, 1));
    }

    {
        float c = cos(ThetaZ);
        float s = sin(ThetaZ);
        RotZ = mat4(vec4(c, -s, 0, 0),
                    vec4(s, c, 0, 0),
                    vec4(0, 0, 1, 0),
                    vec4(0, 0, 0, 1));
    }

    return ((RotX * RotY * RotZ) * vec4(Pos, 1)).xyz;
}

//
// NOTE: SDF operators
//

vec3 TranslateSdf(vec3 RayPos, vec3 Displacement)
{
    vec3 Result = RayPos - Displacement;
    return Result;
}

float IntersectSdf(float DistA, float DistB)
{
    return max(DistA, DistB);
}

float UnionSdf(float DistA, float DistB)
{
    return min(DistA, DistB);
}

float DifferenceSdf(float DistA, float DistB)
{
    return max(DistA, -DistB);
}

//
// NOTE: SDF Folding Operators
//

vec3 SierpinskiFold(vec3 RayPos)
{
    // NOTE: This is the same as the if statement version
    RayPos.xy -= min(RayPos.x + RayPos.y, 0);
    RayPos.xz -= min(RayPos.x + RayPos.z, 0);
    RayPos.yz -= min(RayPos.y + RayPos.z, 0);

    return RayPos;
}

//
// NOTE: SDF Objects
//

float Box2dSdf(vec2 RayPos, vec2 Dim)
{
    vec2 Distance = abs(RayPos) - Dim;
    float InsideDist = min(max(Distance.x, Distance.y), 0);
    float OutsideDist = length(max(Distance, 0));

    return InsideDist + OutsideDist;
}

float BoxSdf(vec3 RayPos, vec3 Dim)
{
    vec3 Distance = abs(RayPos) - Dim;
    float InsideDist = min(max(Distance.x, max(Distance.y, Distance.z)), 0);
    float OutsideDist = length(max(Distance, 0));

    return InsideDist + OutsideDist;
}

float SphereSdf(vec3 RayPos, float SphereRadius)
{
    // NOTE: This finds the distance to a circle, where the radius is 1.0 for it
    return length(RayPos) - SphereRadius;
}

float CrossSdf(vec3 RayPos)
{
    float Dist1 = Box2dSdf(RayPos.xy, vec2(1));
    float Dist2 = Box2dSdf(RayPos.yz, vec2(1));
    float Dist3 = Box2dSdf(RayPos.zx, vec2(1));

    return UnionSdf(UnionSdf(Dist1, Dist2), Dist3);
}

float TetrahedronSdf(vec4 p, float r)
{
    float md = max(max(-p.x - p.y - p.z, p.x + p.y - p.z),
                   max(-p.x + p.y + p.z, p.x - p.y + p.z));
    return (md - r) / (p.w * sqrt(3.0));
}

//
// NOTE: SDF Functions
//

struct sdf_result
{
    uint NumIterations;
    float T;
};

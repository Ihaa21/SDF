#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(binding = 0) uniform sdf_input_buffer
{
    float Time;
    float RenderWidth;
    float RenderHeight;
} SdfInputs;

layout(location = 0) in vec2 InUv;

layout(location = 0) out vec4 OutColor;

/*

  NOTE: SDF notes:

  http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
  
  - To apply transformations to models, you have to apply the inverse transformation to the point before evaluating the SDF. The
  transformations must be rigid for the result to still be a SDF.
  - To scale uniformly by X, we do SDF(Pos / X) * X;
      
*/

#include "shader_sdf_lib.cpp"
#include "shader_phong_lighting.cpp"

float SceneSdf(vec3 Pos)
{
    float T;
    
    // NOTE: Distorted Sphere
#if 0
    {
        // NOTE: https://michaelwalczyk.com/blog-ray-marching.html
        float RealTime = 4.0*sin(0.25*SdfInputs.Time);
        float Displacement = sin(5.0 * Pos.x * RealTime) * sin(5.0 * Pos.y * RealTime) * sin(5.0 * Pos.z * RealTime) * 0.25;
        float Sphere = SphereSdf(Pos, 1);
        T = Displacement + Sphere;
    }
#endif
    
    // NOTE: Repeated Objects
#if 0
    {
        vec3 TransformedPos = mod(Pos + 0.5*2, 2) - 0.5*2;
        //TransformedPos = (RotateYM4(-SdfInputs.Time * 0.25) * vec4(TransformedPos, 1)).xyz;

        float NormalizedTime = abs(sin(SdfInputs.Time) * 0.65);
        T = DifferenceSdf(CubeSdf(TransformedPos / 0.45) * 0.45, SphereSdf(TransformedPos, NormalizedTime));
    }
#endif

    // NOTE: Cross Fractal
#if 0
    {
        T = BoxSdf(Pos, vec3(1));

        float Scale = 1;
        for (int m = 0; m < 4; ++m)
        {
            vec3 NewPos = mod(Pos * Scale, 2.0) - 1.0;
            Scale *= 3.0;
            vec3 r = 1.0 - 3.0*abs(NewPos);

            T = DifferenceSdf(T, CrossSdf(r) / Scale);
        }
    }
#endif

    // NOTE: Sierpinski Triangle
#if 0
    {
        /*
            NOTE: We can construct the triangle fractal by taking a tetrahedron, then mirroring it on 3 planes to make a "triforce",
                  and repeating this process x amount of times. For SDFs, you have to work backwards so you start at a point, you do the
                  folding, and then you eventually arrive at the base tetrahedron which you get a distance value from.
         */

        int i = 0;
        for (i = 0; i < 1; ++i)
        {
            Pos = SierpinskiFold(Pos);
            Pos = Pos*2 - 1;
        }

        T = TetrahedronSdf(vec4(Pos, 1), .3);
    }
#endif

    // NOTE: Failed Menger Sponge
#if 0
    {
        //Pos = (RotateYM4(-SdfInputs.Time * 0.25) * vec4(Pos, 1)).xyz;

        int n = 0;
        while (n < 6)
        {
            // NOTE: Mirror in xyz axis
            Pos = abs(Pos);

            // NOTE: Translate and scale to keep proportions correct
            Pos /= 0.5f;
            Pos = TranslateSdf(Pos, vec3(1.5, 1.5, 1.5));
        
            // NOTE: Mirror along xz axis
            if (Pos.x > Pos.z)
            {
                Pos.xz = Pos.zx;
            }
        
            // NOTE: Mirror along the xy axis
            if (Pos.x > Pos.y)
            {
                Pos.xy = Pos.yx;
            }
            Pos = TranslateSdf(Pos, vec3(-0.5, 0, 0));
        
            // NOTE: Mirror along the x axis
            Pos.x = abs(Pos.x);
            Pos = TranslateSdf(Pos, vec3(0.5, 0, 0));

            n++;
        }
        
        T = BoxSdf(Pos, vec3(0.5));
        T *= pow(0.5f, float(n));
    }
#endif
    
    // NOTE: Menger Sponge
#if 0
    {
        Pos = (RotateYM4(-SdfInputs.Time * 0.25) * vec4(Pos, 1)).xyz;

        int n = 0;

        float Scale = 2.0f;
        Pos /= Scale;
        while (n < 10)
        {
            // NOTE: Mirror in xyz axis
            Pos /= (1.0/3.0);
            Pos.xyz = abs(Pos.xyz);

            // NOTE: Translate and scale to keep proportions correct
            Pos = TranslateSdf(Pos, vec3(1));
        
            // NOTE: Mirror along xz axis
            if (Pos.x > Pos.z)
            {
                Pos.xz = Pos.zx;
            }
        
            // NOTE: Mirror along the xy axis
            if (Pos.x > Pos.y)
            {
                Pos.xy = Pos.yx;
            }
            Pos = TranslateSdf(Pos, vec3(-0.5, 0, 0));
        
            // NOTE: Mirror along the x axis
            Pos.x = abs(Pos.x);
            Pos = TranslateSdf(Pos, vec3(0.5, 0, 0));

            n++;
        }
        
        T = BoxSdf(Pos, vec3(0.5));
        T *= pow(1.0 / 3.0, float(n));
        T *= Scale;
    }
#endif
    
    // NOTE: Modified Menger Sponge
#if 1
    {
        float Scale = 5.0f;
        Pos = mod(Pos + 0.5 * vec3(Scale), vec3(Scale)) - 0.5*vec3(Scale);
        
        int n = 0;

        Pos /= Scale;
        while (n < 10)
        {
            // NOTE: Mirror in xyz axis
            Pos /= (1.0/3.0);
            Pos.xyz = abs(Pos.xyz);

            // NOTE: Translate and scale to keep proportions correct
            Pos.xz *= RotationM2(SdfInputs.Time * 0.05 + 1.8*3.14);
            Pos.xy *= RotationM2(SdfInputs.Time * 0.01 + 1.9*3.14);
            //Pos.xy *= RotationM2(SdfInputs.Time * 0.25);
            //Pos = Rotate(Pos, SdfInputs.Time * 0.02, -SdfInputs.Time * 0.06, 0);
            Pos = TranslateSdf(Pos, vec3(0.25));
            Pos = TranslateSdf(Pos, vec3(1));
        
            // NOTE: Mirror along xz axis
            if (Pos.x > Pos.z)
            {
                Pos.xz = Pos.zx;
            }
        
            // NOTE: Mirror along the xy axis
            if (Pos.x > Pos.y)
            {
                Pos.xy = Pos.yx;
            }
            Pos = TranslateSdf(Pos, vec3(-0.5, 0, 0));
        
            // NOTE: Mirror along the x axis
            Pos.x = abs(Pos.x);
            Pos = TranslateSdf(Pos, vec3(0.5, 0, 0));

            n++;
        }
        
        T = BoxSdf(Pos, vec3(0.5));
        T *= pow(1.0 / 3.0, float(n));
        T *= Scale;
    }
#endif
    
    return T;
}

//
// NOTE: SDF tracing
//

const float MAX_DIST = 100;

sdf_result TraceRay(vec3 RayPos, vec3 RayDir, int LoopCount)
{
    const float Epsilon = 0.0001;

    sdf_result Result;
    Result.T = 0.0;
    Result.NumIterations = 0;
    for (Result.NumIterations = 0; Result.NumIterations < LoopCount; ++Result.NumIterations)
    {
        vec3 Pos = RayPos + Result.T*RayDir;
        float Dist = SceneSdf(Pos);
        
        if (Dist < Epsilon)
        {
            return Result;
        }

        Result.T += Dist;
        if (Result.T > MAX_DIST)
        {
            break;
        }
    }
    
    return Result;
}

vec3 SceneNormal(vec3 Pos)
{
    const float Epsilon = 0.01;
    vec3 Result = normalize(vec3(SceneSdf(vec3(Pos.x + Epsilon, Pos.y, Pos.z)) - SceneSdf(vec3(Pos.x - Epsilon, Pos.y, Pos.z)),
                                 SceneSdf(vec3(Pos.x, Pos.y + Epsilon, Pos.z)) - SceneSdf(vec3(Pos.x, Pos.y - Epsilon, Pos.z)),
                                 SceneSdf(vec3(Pos.x, Pos.y, Pos.z + Epsilon)) - SceneSdf(vec3(Pos.x, Pos.y, Pos.z - Epsilon))));

    return Result;
}

vec3 SceneLighting(vec3 Pos, vec3 CameraPos, uint NumIterations)
{
    vec3 Color = vec3(0);
    vec3 Normal = SceneNormal(Pos);

    // NOTE: Ligting globals
    float AO = 1.0 / (1.0 + float(NumIterations)*0.1);
    vec3 Ambient = AO*vec3(0.1, 0.8, 0.8);
    vec3 Diffuse = vec3(0.2, 0.5, 0.6);
    vec3 Specular = vec3(1, 1, 1);
    float SpecularPower = 10;

    Color += Ambient;

    {
        //vec3 LightPos = vec3(4.0 * sin(SdfInputs.Time), 2.0, 4.0 * cos(SdfInputs.Time));
        vec3 LightPos = vec3(0, 0, 0);
        vec3 LightIntensity = vec3(0.6, 0.6, 0.6);

        vec3 LightDir = normalize(Pos - LightPos);
        sdf_result Result = TraceRay(LightPos, LightDir, 256);
        vec3 ResultPos = LightPos + Result.T * LightDir;

        if (length(ResultPos - Pos) < 0.0001)
        {
            Color += PhongLighting(Diffuse, Specular, SpecularPower, Pos, Normal, CameraPos, LightPos, LightIntensity);
        }
    }

#if 0
    {
        vec3 LightPos = vec3(2.0 * sin(0.37 * SdfInputs.Time), 2.0 * cos(0.37 * SdfInputs.Time), -2.0);
        vec3 LightIntensity = vec3(0.4, 0.4, 0.4);

        vec3 LightDir = normalize(Pos - LightPos);
        sdf_result Result = TraceRay(LightPos, LightDir, 256);
        vec3 ResultPos = LightPos + Result.T * LightDir;

        if (length(ResultPos - Pos) < 0.0001)
        {
            Color += PhongLighting(Diffuse, Specular, SpecularPower, Pos, Normal, CameraPos, LightPos, LightIntensity);
        }
    }
#endif
    
    return Color;
}

void main()
{
    //vec2 NormalizedUv = InUv * vec2(1, SdfInputs.RenderWidth / SdfInputs.RenderHeight);
    //NormalizedUv.y = 1.0 - NormalizedUv.y;
    //OutColor = vec4(NormalizedUv, 0, 1);

    vec2 UV = gl_FragCoord.xy / vec2(SdfInputs.RenderWidth, SdfInputs.RenderHeight);
    UV.y = 1.0 - UV.y; // NOTE: Take into accoutn vulkan coords
    UV = 2.0 * UV - 1.0; // NOTE: Convert range to -1, 1
    UV.x *= SdfInputs.RenderWidth / SdfInputs.RenderHeight;

    vec3 RayPos = vec3(0, 0, -3);
    vec3 RayDir = normalize(vec3(UV, 1)); // NOTE: This 1 controls the field of view
    
    // NOTE: Camera Movement
    {
        RayPos += vec3(0, 0.25*SdfInputs.Time, 0.25*SdfInputs.Time); // NOTE: Add translation
        RayDir.xz *= RotationM2(SdfInputs.Time * 0.25); // NOTE: Add rotation
    }
    
    sdf_result Result = TraceRay(RayPos, RayDir, 512);
    if (Result.T < MAX_DIST)
    {
        vec3 FinalPos = RayPos + Result.T*RayDir;
        OutColor = vec4(SceneLighting(FinalPos, RayPos, Result.NumIterations), 1.0);
    }
    else
    {
        OutColor = vec4(0, 0, 0, 1);
    }
    
    // TODO: DEBUG
#if 0
    if (UV.x > UV.y)
    {
        OutColor = vec4(1, 1, 1, 1);
    }
    else
    {
        OutColor = vec4(0, 0, 0, 1);
    }
#endif
}

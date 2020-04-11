cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITIONT;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITIONT;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor[2]			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NOISE_SIMPLEX_1_DIV_289 0.00346020761245674740484429065744f
 
float mod289(float x)
{
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}
 
float2 mod289(float2 x)
{
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}
 
float3 mod289(float3 x)
{
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}
 
float4 mod289(float4 x)
{
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}
 
 
// ( x*34.0 + 1.0 )*x =
// x*x*34.0 + x
float permute(float x)
{
    return mod289(
        x * x * 34.0 + x
    );
}
 
float3 permute(float3 x)
{
    return mod289(
        x * x * 34.0 + x
    );
}
 
float4 permute(float4 x)
{
    return mod289(
        x * x * 34.0 + x
    );
}
 
 
 
float taylorInvSqrt(float r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}
 
float4 taylorInvSqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}
 
 
 
float4 grad4(float j, float4 ip)
{
    const float4 ones = float4(1.0, 1.0, 1.0, -1.0);
    float4 p, s;
    p.xyz = floor(frac(j * ip.xyz) * 7.0) * ip.z - 1.0;
    p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
 
    // GLSL: lessThan(x, y) = x < y
    // HLSL: 1 - step(y, x) = x < y
    s = float4(
        1 - step(0.0, p)
    );
 
    // Optimization hint Dolkar
    // p.xyz = p.xyz + (s.xyz * 2 - 1) * s.www;
    p.xyz -= sign(p.xyz) * (p.w < 0);
 
    return p;
}
 
 
 
// ----------------------------------- 2D -------------------------------------
 
float snoise(float2 v)
{
    const float4 C = float4(
        0.211324865405187, // (3.0-sqrt(3.0))/6.0
        0.366025403784439, // 0.5*(sqrt(3.0)-1.0)
     -0.577350269189626, // -1.0 + 2.0 * C.x
        0.024390243902439 // 1.0 / 41.0
    );
 
// First corner
    float2 i = floor(v + dot(v, C.yy));
    float2 x0 = v - i + dot(i, C.xx);
 
// Other corners
    // float2 i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
    // Lex-DRL: afaik, step() in GPU is faster than if(), so:
    // step(x, y) = x <= y
 
    // Optimization hint Dolkar
    //int xLessEqual = step(x0.x, x0.y); // x <= y ?
    //int2 i1 =
    //    int2(1, 0) * (1 - xLessEqual) // x > y
    //    + int2(0, 1) * xLessEqual // x <= y
    //;
    //float4 x12 = x0.xyxy + C.xxzz;
    //x12.xy -= i1;
 
    float4 x12 = x0.xyxy + C.xxzz;
    int2 i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
    x12.xy -= i1;
 
// Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    float3 p = permute(
        permute(
                i.y + float3(0.0, i1.y, 1.0)
        ) + i.x + float3(0.0, i1.x, 1.0)
    );
 
    float3 m = max(
        0.5 - float3(
            dot(x0, x0),
            dot(x12.xy, x12.xy),
            dot(x12.zw, x12.zw)
        ),
        0.0
    );
    m = m * m;
    m = m * m;
 
// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
 
    float3 x = 2.0 * frac(p * C.www) - 1.0;
    float3 h = abs(x) - 0.5;
    float3 ox = floor(x + 0.5);
    float3 a0 = x - ox;
 
// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
 
// Compute final noise value at P
    float3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

    Output.pos = lerp(lerp(patch[0].pos, patch[1].pos, domain.x), lerp(patch[2].pos, patch[3].pos, domain.x), domain.y);
    Output.normal = lerp(lerp(patch[0].normal, patch[1].normal, domain.x), lerp(patch[2].normal, patch[3].normal, domain.x), domain.y);
    Output.color = lerp(lerp(patch[0].color, patch[1].color, domain.x), lerp(patch[2].color, patch[3].color, domain.x), domain.y);
	
    Output.worldPos = mul(Output.pos, model);
    Output.pos = mul(Output.pos, model);
    Output.pos = mul(Output.pos, view);
    Output.pos = mul(Output.pos, projection);

	return Output;
}

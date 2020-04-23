struct GSInputOutput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
    float clip : SV_ClipDistance0;
};

[maxvertexcount(3)]
void main(triangle GSInputOutput input[3] : SV_POSITION, inout TriangleStream<GSInputOutput> output)
{
    GSInputOutput element = (GSInputOutput) 0;

    float3 vec1 = input[0].normal;
    float3 vec2 = input[1].normal;
    float3 vec3 = input[2].normal;

    float3 faceNormal = cross((vec2 - vec1), (vec2 - vec3));
    faceNormal = normalize(faceNormal);
    faceNormal.y = faceNormal.y * -1;

    for (uint i = 0; i < 3; i++)
    {
        element = input[i];
        //element.normal = faceNormal;
        output.Append(element);
    }
}
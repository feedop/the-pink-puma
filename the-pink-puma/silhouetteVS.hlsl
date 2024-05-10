cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
    matrix worldMatrix;
};


struct VSInput
{
    float3 pos : POSITION;
    float3 norm : NORMAL0;
};


struct GSInput
{
    float4 worldPos : SV_POSITION;
};


GSInput main(VSInput i)
{
    GSInput o;
    o.worldPos = mul(worldMatrix, float4(i.pos, 1.0f));
    
    return o;
}
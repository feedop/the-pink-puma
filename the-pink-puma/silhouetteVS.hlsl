cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
    matrix worldMatrix;
};


cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
    matrix viewMatrix;
    matrix invViewMatrix;
};


cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
    matrix projMatrix;
};


struct VSInput
{
    float3 pos : POSITION;
    float3 norm : NORMAL0;
};


struct GSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
};


GSInput main(VSInput i)
{
    GSInput o;
    o.worldPos = mul(worldMatrix, float4(i.pos, 1.0f)).xyz;
    o.pos = mul(viewMatrix, float4(o.worldPos, 1.0f));
    o.pos = mul(projMatrix, o.pos);
    
    return o;
}
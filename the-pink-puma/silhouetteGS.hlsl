cbuffer cbView : register(b0) //Vertex Shader constant buffer slot 1
{
    matrix viewMatrix;
    matrix invViewMatrix;
};


cbuffer cbProj : register(b1) //Vertex Shader constant buffer slot 2
{
    matrix projMatrix;
};


cbuffer cbLights : register(b2)
{
    float4 lightPos;
};


struct PSInput
{
	float4 pos : SV_POSITION;
};


struct GSInput
{
    float4 worldPos : SV_POSITION;
};


void EmitLine(int startIndex, int endIndex, triangleadj GSInput input[6], inout LineStream<PSInput> output)
{
    PSInput i;
    
    i.pos = mul(viewMatrix, input[startIndex].worldPos);
    i.pos = mul(projMatrix, i.pos);
    output.Append(i);
            
    i.pos = mul(viewMatrix, input[endIndex].worldPos);
    i.pos = mul(projMatrix, i.pos);
    output.Append(i);
            
    output.RestartStrip();
}


static const float EPSILON = 0.0001;


void EmitQuad(float3 StartVertex, float3 EndVertex, inout TriangleStream<PSInput> output)
{
    PSInput i;
    
    // Vertex #1: the starting vertex (just a tiny bit below the original edge)
    float3 LightDir = normalize(StartVertex - lightPos.xyz);
    i.pos = mul(viewMatrix, float4((StartVertex + LightDir * EPSILON), 1.0));
    i.pos = mul(projMatrix, i.pos);
    output.Append(i);

    // Vertex #2: the starting vertex projected to infinity
    i.pos = mul(viewMatrix, float4((StartVertex + LightDir * 5.f), 1.0));
    i.pos = mul(projMatrix, i.pos);
    output.Append(i);

    // Vertex #3: the ending vertex (just a tiny bit below the original edge)
    LightDir = normalize(EndVertex - lightPos.xyz);
    i.pos = mul(viewMatrix, float4((EndVertex + LightDir * EPSILON), 1.0));
    i.pos = mul(projMatrix, i.pos);
    output.Append(i);

    // Vertex #4: the ending vertex projected to infinity
    i.pos = mul(viewMatrix, float4((EndVertex + LightDir * 5.f), 1.0));
    i.pos = mul(projMatrix, i.pos);
    output.Append(i);
   
    output.RestartStrip();
}


[maxvertexcount(18)]
void main(
	triangleadj GSInput input[6] : SV_POSITION,
	inout TriangleStream<PSInput> output
)
{   
    float3 e1 = input[2].worldPos.xyz - input[0].worldPos.xyz;
    float3 e2 = input[4].worldPos.xyz - input[0].worldPos.xyz;
    float3 e3 = input[1].worldPos.xyz - input[0].worldPos.xyz;
    float3 e4 = input[3].worldPos.xyz - input[2].worldPos.xyz;
    float3 e5 = input[4].worldPos.xyz - input[2].worldPos.xyz;
    float3 e6 = input[5].worldPos.xyz - input[0].worldPos.xyz;

    float3 Normal = cross(e1, e2);
    float3 LightDir = lightPos.xyz - input[0].worldPos.xyz;

    if (dot(Normal, LightDir) > 0)
    {
        PSInput i;
        Normal = cross(e3, e1);

        if (dot(Normal, LightDir) <= 0)
        {
            float3 StartVertex = input[0].worldPos.xyz;
            float3 EndVertex = input[2].worldPos.xyz;
            EmitQuad(StartVertex, EndVertex, output);
            //EmitLine(0, 2, input, output);
        }

        Normal = cross(e4, e5);
        LightDir = lightPos.xyz - input[2].worldPos.xyz;

        if (dot(Normal, LightDir) <= 0)
        {
            float3 StartVertex = input[2].worldPos.xyz;
            float3 EndVertex = input[4].worldPos.xyz;
            EmitQuad(StartVertex, EndVertex, output);
            //EmitLine(2, 4, input, output);
        }

        Normal = cross(e2, e6);
        LightDir = lightPos.xyz - input[4].worldPos.xyz;

        if (dot(Normal, LightDir) <= 0)
        {
            float3 StartVertex = input[4].worldPos.xyz;
            float3 EndVertex = input[0].worldPos.xyz;
            EmitQuad(StartVertex, EndVertex, output);
            //EmitLine(4, 0, input, output);
        }
    }
    
}
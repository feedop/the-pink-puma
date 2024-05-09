cbuffer cbLights : register(b1)
{
    float4 lightPos;
};


struct PSInput
{
	float4 pos : SV_POSITION;
};


struct GSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
};


void EmitLine(int startIndex, int endIndex, triangleadj GSInput input[6], inout LineStream<PSInput> output)
{
    PSInput i;
    
    i.pos = input[startIndex].pos;
    output.Append(i);
            
    i.pos = input[endIndex].pos;
    output.Append(i);
            
    output.RestartStrip();
}


[maxvertexcount(6)]
void main(
	triangleadj GSInput input[6] : SV_POSITION,
	inout LineStream< PSInput > output
)
{   
    float3 e1 = input[2].worldPos - input[0].worldPos;
    float3 e2 = input[4].worldPos - input[0].worldPos;
    float3 e3 = input[1].worldPos - input[0].worldPos;
    float3 e4 = input[3].worldPos - input[2].worldPos;
    float3 e5 = input[4].worldPos - input[2].worldPos;
    float3 e6 = input[5].worldPos - input[0].worldPos;

    float3 Normal = cross(e1, e2);
    float3 LightDir = lightPos.xyz - input[0].worldPos;

    if (dot(Normal, LightDir) > 0)
    {
        PSInput i;
        Normal = cross(e3, e1);

        if (dot(Normal, LightDir) <= 0)
        {
            EmitLine(0, 2, input, output);
        }

        Normal = cross(e4, e5);
        LightDir = lightPos.xyz - input[2].worldPos;

        if (dot(Normal, LightDir) <= 0)
        {
            EmitLine(2, 4, input, output);
        }

        Normal = cross(e2, e6);
        LightDir = lightPos.xyz - input[4].worldPos;

        if (dot(Normal, LightDir) <= 0)
        {
            EmitLine(4, 0, input, output);
        }
    }
    
}
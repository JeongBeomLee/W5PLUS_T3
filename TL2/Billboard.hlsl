cbuffer CameraInfo : register(b0)
{
    float3 CompWorldPos;
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    row_major matrix viewInverse;
};

// C++의 BillboardCharInfo
struct VS_INPUT
{
    float3 centerPos : WORLDPOSITION;
    float2 size : SIZE;
    float4 uvRect : UVRECT;
    uint vertexId : SV_VertexID; // GPU가 자동으로 부여하는 고유 정점 ID
};

struct PS_INPUT
{
    float4 pos_screenspace : SV_POSITION;
    float2 tex : TEXCOORD0;
};

Texture2D iconSprite : register(t0);
SamplerState iconSampler : register(s0);


PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    float3 pos_aligned = mul(float4(input.centerPos, 0.0f), viewInverse).xyz; //카메라 회전 무시시키고
    float3 finalPos_worldspace = CompWorldPos + pos_aligned; //월드좌표계에서 원하는 위치에 위치시킨다(4개의 corner점을)
     
    
    output.pos_screenspace = mul(float4(finalPos_worldspace, 1.0f), mul(viewMatrix, projectionMatrix)) * 0.1; //월드좌표기준에서 view proj
    
    output.tex = input.uvRect.xy; // UV는 C++에서 계산했으므로 그대로 전달

    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float4 color = iconSprite.Sample(iconSampler, input.tex);

    clip(color.a - 0.5f); // alpha - 0.5f < 0 이면 해당픽셀 렌더링 중단

    return color;
}
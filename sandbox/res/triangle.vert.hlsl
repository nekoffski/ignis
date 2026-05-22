
struct VSInput
{
    float3 pos : POSITION;
    float3 color : COLOR0;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 color : COLOR0;
};

VSOutput main(VSInput i)
{
    VSOutput o;
    o.pos = float4(i.pos, 1.0);
    o.color = i.color;
    return o;
}

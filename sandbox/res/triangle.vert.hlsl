
struct VSOutput {
  float4 pos : SV_Position;
  float3 color : COLOR0;
};

static const float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5),
};

static const float3 colors[3] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0),
};

VSOutput main(uint vertexID : SV_VertexID) {
  VSOutput o;
  o.pos = float4(positions[vertexID], 0.0, 1.0);
  o.color = colors[vertexID];
  return o;
}

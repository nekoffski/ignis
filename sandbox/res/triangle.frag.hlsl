struct PSInput {
  float4 pos : SV_Position;
  float3 color : COLOR0;
};

[[vk::push_constant]] struct PushConstants {
  float4 tint;
} pc;

float4 main(PSInput i) : SV_Target { return float4(i.color, 1.0) * pc.tint; }

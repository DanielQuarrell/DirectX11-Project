struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer CBuf
{
	matrix transform;
};

VS_OUTPUT VS(float3 position : POSITION, float4 color : COLOR)
{
	VS_OUTPUT output;

	output.position = mul(float4(position, 1.0f), transform);
	output.color = color;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return input.color;
}

cbuffer CBufTransform
{
	matrix transform;
};

float4 VS(float3 position : POSITION) : SV_POSITION
{
	return mul(float4(position, 1.0f), transform);
}

cbuffer CBufColor
{
	float4 face_colors[6];
};

float4 PS(uint tid : SV_PrimitiveID) : SV_TARGET
{
	return face_colors[tid / 2];
}

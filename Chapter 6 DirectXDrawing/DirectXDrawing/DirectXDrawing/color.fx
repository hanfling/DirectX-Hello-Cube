cbuffer cbPerObject 
{
	float4x4 gWorldViewProj;
};

struct VertexIn 
{
	float3 Pos : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

struct GeoOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR4;
	float4 A : COLOR0;
	float4 B : COLOR1;
	float4 C : COLOR2;
	float4 D : COLOR3;
	float a : TEXCOORD0;
	float b : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	vout.Color = vin.Color;

	return vout;
}

[maxvertexcount(3)]
void GS(triangleadj VertexOut gin[6], inout TriangleStream<GeoOut> stream)
{
	GeoOut gout;

	gout.A = gin[2].Color;
	gout.B = gin[4].Color;
	gout.C = gin[5].Color;
	gout.D = gin[0].Color;

	gout.PosH = gin[0].PosH;
	gout.Color = gin[0].Color;
	gout.a = 1.0;
	gout.b = 0.0;
	stream.Append( gout );

	gout.PosH = gin[2].PosH;
	gout.Color = gin[2].Color;
	gout.a = 0.0;
	gout.b = 0.0;
	stream.Append( gout );

	gout.PosH = gin[4].PosH;
	gout.Color = gin[4].Color;
	gout.a = 0.0;
	gout.b = 1.0;
	stream.Append( gout );
}

float4 PS(GeoOut pin) : SV_TARGET
{
#if 1
	return (1.0-pin.a)*(1-pin.b)*pin.A+(1.0-pin.a)*(pin.b)*pin.B+(pin.a)*(pin.b)*pin.C+(pin.a)*(1-pin.b)*pin.D;
#else
	return pin.Color;
#endif
}

technique11 ColorTech {
	pass P0 {
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_4_0,GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};

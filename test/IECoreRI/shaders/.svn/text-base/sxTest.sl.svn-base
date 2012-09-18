surface sxTest(
	
	float noiseFrequency = 10;
	color tint = color( 1, 0, 0 );
	
	output varying float outputFloat = 0;
	output varying color outputColor = 0;
	
)
{
	outputFloat = noise( P * noiseFrequency );
	float n = 1 - normalize( N ).normalize( -I );
	
	outputColor = color cellnoise( P * noiseFrequency );
	
	Ci = tint * n * outputFloat * outputColor;
	Oi = 1;
}

surface sxUniformPrimitiveVariableShaderParameterTest(
	
	uniform color colorPrimVar = 0;
	uniform float floatPrimVar = 0;
	uniform vector vectorPrimVar = 0;
	uniform string stringPrimVar = "";
	uniform string stringVectorPrimVar[] = {};

)
{

	if(	
		colorPrimVar == color( 0, 0.5, 1 ) &&
		floatPrimVar == 16 &&
		vectorPrimVar == vector( 0.25, 0.5, 2 ) &&
		stringPrimVar == "hello shader!" &&
		arraylength( stringVectorPrimVar ) == 4 &&
		stringVectorPrimVar[0] == "who's" &&
		stringVectorPrimVar[1] == "a" &&
		stringVectorPrimVar[2] == "good" &&
		stringVectorPrimVar[3] == "boy"
	)
	{
		Ci = color( 0.125, 0.25, 0.5 );
	}
	else
	{
		Ci = color( 0, 0, 0 );
	}
	Oi = 1;

}

surface sxUniformPrimitiveVariableTest(
	
)
{
	
	uniform color colorPrimVar;
	uniform float floatPrimVar;
	uniform vector vectorPrimVar;
	uniform string stringPrimVar;
	uniform string stringVectorPrimVar[] = { "", "", "", "" };
	
	getvar( null, "colorPrimVar", colorPrimVar );
	getvar( null, "floatPrimVar", floatPrimVar );
	getvar( null, "vectorPrimVar", vectorPrimVar );
	getvar( null, "stringPrimVar", stringPrimVar );
	getvar( null, "stringVectorPrimVar", stringVectorPrimVar );
	
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
		Ci = color( 0.125, 0.25, 0.75 );
	}
	else
	{
		Ci = color( 0, 0, 0 );
	}
	Oi = 1;

}

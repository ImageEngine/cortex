surface sxPredefinedPrimitiveVariableTest()
{
	if(	N == normal( 0,0,1 ) &&
		I == vector( 0,0,-1 ) &&
		dPdu == vector( 2,0,0 ) &&
		dPdv == vector( 0,2,0 )
	)
	{
		Ci = color( 1, 1, 1 );
	}
	else
	{
		Ci = color( 0,0,0 );
	}

}

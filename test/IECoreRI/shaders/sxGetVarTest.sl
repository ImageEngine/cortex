surface sxGetVarTest()
{

	float c = 0;
	getvar( null, "iDontExist", c );
	Ci[0] = c;
	getvar( null, "floatValue1", c );
	Ci[1] = c;
	getvar( null, "floatValue2", c );
	Ci[2] = c;
	
	Oi = 1;

}

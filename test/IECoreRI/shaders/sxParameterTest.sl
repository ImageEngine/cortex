surface sxParameterTest(
	float mustBeOne = 0;
	color mustBeRed = color( 0, 1, 0 );
	point mustBeTwo = 0;
	vector mustBeThree = 0;
	normal mustBeFour = 0;
	string mustBeHelloWorld = "";
)
{
	Ci = color( 0, 1, 0 );
	if(
		mustBeOne!=1 ||
		mustBeRed!=color( 1, 0, 0 ) ||
		mustBeTwo!=point( 2 ) ||
		mustBeThree!=vector( 3 ) ||
		mustBeFour!=normal( 4 ) ||
		mustBeHelloWorld!="helloWorld"
	)
	{
		Ci = color( 1, 0, 0 );
	}

	Oi = 1;
}

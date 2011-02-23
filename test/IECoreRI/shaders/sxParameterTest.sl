surface sxParameterTest(
	varying float mustBeOne = 0;
	varying color mustBeRed = color( 0, 1, 0 );
	varying point mustBeTwo = 0;
	varying vector mustBeThree = 0;
	varying normal mustBeFour = 0;
	string mustBeHelloWorld = "";
	varying float mustBeOneTwoThree[3] = { 0, 0, 0 };
)
{
	Ci = color( 0, 1, 0 );
	if(
		mustBeOne!=1 ||
		mustBeRed!=color( 1, 0, 0 ) ||
		mustBeTwo!=point( 2 ) ||
		mustBeThree!=vector( 3 ) ||
		mustBeFour!=normal( 4 ) ||
		mustBeHelloWorld!="helloWorld" ||
		mustBeOneTwoThree[0]!=1 ||
		mustBeOneTwoThree[1]!=2 ||
		mustBeOneTwoThree[2]!=3
	)
	{
		Ci = color( 1, 0, 0 );
	}

	Oi = 1;
}

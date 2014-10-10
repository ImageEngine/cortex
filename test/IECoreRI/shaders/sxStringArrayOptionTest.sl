surface sxStringArrayOptionTest()
{
	string stringArray[];
	option("user:stringArray", stringArray);
	
	Ci = color(0);
	if( arraylength( stringArray ) == 3 )
	{
		if(	stringArray[0] == "this" &&
			stringArray[1] == "should" &&
			stringArray[2] == "work"	)
		{
			Ci = color(1);
		}
	}
	
	
}

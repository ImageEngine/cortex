class sxCoshaderTest( color shaderColor = 0; color sColor = 0; color tColor = 0; varying color colorPrimVar = 0; uniform string primVarName = ""; )
{
	public color getColor()
	{
		if ( primVarName == "" )
		{
			return shaderColor + s * sColor + t * tColor + colorPrimVar;
		}
		else
		{
			varying color C = ( 0, 0, 0 );
			getvar( null, primVarName, C );
			return C;
		}
	}
}

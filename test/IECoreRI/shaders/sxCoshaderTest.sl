class sxCoshaderTest( color shaderColor = 0; color sColor = 0; color tColor = 0; varying color colorPrimVar = 0; )
{
	public color getColor()
	{
		return shaderColor + s * sColor + t * tColor + colorPrimVar;
	}
}

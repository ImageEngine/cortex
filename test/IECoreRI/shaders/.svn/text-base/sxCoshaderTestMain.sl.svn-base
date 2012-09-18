class sxCoshaderTestMain(
	string coshaders[] = {};
)
{
	public void surface( output color Ci; output color Oi; )
	{
		Ci = 0;
		Oi = 1;
		uniform float i = 0;
		for( i = 0; i<arraylength( coshaders ); i+=1 )
		{
			shader c = getshader( coshaders[i] );
			Ci += c->getColor();
		}
	}
}

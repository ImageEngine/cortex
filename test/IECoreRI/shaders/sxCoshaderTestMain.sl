class sxCoshaderTestMain(
	string coshaders[] = {};
)
{
	public void surface( output color Ci; output color Oi; )
	{
		// temp fix - main shader must access all primitive variables needed by coshaders
		// otherwise 3delight crashed in SxCallShader.
		/// \todo Remove me when a fix gets released.
		Ci = color( s, t, 0 ); 
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

class sxGetShaderTest(
	string coshader = "";
)
{
	shader m_coshader;

	public void construct()
	{
		if( coshader!="" )
		{
			m_coshader = getshader( coshader );
		}
	}

	public void surface( output color Ci; output color Oi; )
	{
		Ci = 0;
		Oi = 1;
		uniform float i = 0;
		Ci += m_coshader->getColor();
	}
}

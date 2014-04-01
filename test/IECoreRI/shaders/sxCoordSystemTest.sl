surface sxCoordSystemTest( string coordSysName = "current" )
{
	Ci = color( transform( coordSysName, "current", P ) );
}

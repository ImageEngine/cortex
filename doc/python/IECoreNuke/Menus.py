
import os

import IECore
import IECoreNuke

def addProceduralCreationCommands( menu ) :

	loader = IECore.ClassLoader.defaultProceduralLoader()
	for c in loader.classNames() :
		menuPath = "/".join( [ IECore.CamelCase.toSpaced( x ) for x in c.split( "/" ) ] )
		menu.addCommand( menuPath, IECore.curry( IECoreNuke.FnProceduralHolder.create, os.path.basename( c ), c ) )
		
def addOpCreationCommands( menu ) :

	loader = IECore.ClassLoader.defaultOpLoader()
	for c in loader.classNames() :
		menuPath = "/".join( [ IECore.CamelCase.toSpaced( x ) for x in c.split( "/" ) ] )	
		menu.addCommand( menuPath, IECore.curry( IECoreNuke.FnOpHolder.create, os.path.basename( c ), c ) )
		

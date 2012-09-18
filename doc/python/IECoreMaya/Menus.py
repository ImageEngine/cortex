
import os

import maya.cmds

import IECore
import IECoreMaya

## \addtogroup environmentGroup
#
# IECOREMAYA_DISABLE_MENU
# Set this to a value of 1 to disable the creation of the Cortex menu
# in Maya. You can then use the helper functions in IECoreMaya.Menus
# to build your own site-specific menu structure.

__cortexMenu = None
def createCortexMenu() :

	if os.environ.get( "IECOREMAYA_DISABLE_MENU", "0" ) == "1" :
		return
		
	m = IECore.MenuDefinition()
	m.append(
		"/Create Procedural",
		{
			"subMenu" : proceduralCreationMenuDefinition,
		}
	)
	
	m.append(
		"/Create Op",
		{
			"subMenu" : opCreationMenuDefinition,
		}
	)
	
	global __cortexMenu
	__cortexMenu = IECoreMaya.createMenu( m, "MayaWindow", "Cortex" )
	
def removeCortexMenu() :

	global __cortexMenu
	if __cortexMenu is not None :
		maya.cmds.deleteUI( __cortexMenu )
		__cortexMenu = None

def __createProcedural( className ) :

	fnPH = IECoreMaya.FnProceduralHolder.create( os.path.basename( className ), className )
	maya.cmds.select( fnPH.fullPathName() )
	
def proceduralCreationMenuDefinition() :

	m = IECore.MenuDefinition()
	loader = IECore.ClassLoader.defaultProceduralLoader()
	for className in loader.classNames() :
		m.append(
			"/" + className,	
			{
				"command" : IECore.curry( __createProcedural, className ),
			}
		)
		
	return m

def __createOp( className ) :

	fnOH = IECoreMaya.FnOpHolder.create( os.path.basename( className ), className )
	maya.cmds.select( fnOH.fullPathName() )
			
def opCreationMenuDefinition() :

	m = IECore.MenuDefinition()
	loader = IECore.ClassLoader.defaultOpLoader()
	for className in loader.classNames() :
		m.append(
			"/" + className,	
			{
				"command" : IECore.curry( __createOp, className ),
			}
		)
		
	return m
	

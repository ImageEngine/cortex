import maya.cmds

# create the 'IE' menu when the UI is available
if hasattr( maya.cmds, "about" ) and not maya.cmds.about( batch=True, q=True ):
	import IECore
	import IECoreMaya
	IECoreMaya.Menus.createMenu( "IE", IECore.MenuDefinition(), "MayaWindow" )

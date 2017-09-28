import IECore.BasicPreset
import os.path

class basicPresetLoadTest( IECore.BasicPreset ):

	def __init__( self ):
		dir = os.path.dirname( __file__ )
		IECore.BasicPreset.__init__( self, dir+"/basicPresetLoadTest-1.cob"	)


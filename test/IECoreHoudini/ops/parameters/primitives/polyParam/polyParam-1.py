from IECore import *

class polyParam( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that has point parameters.",
			MeshPrimitiveParameter(
				name = "result",
				description = "A pass through of the input primitive parameter.",
				defaultValue = MeshPrimitive()
			)
		)

		self.parameters().addParameter(
			MeshPrimitiveParameter(
				name = "input",
				description = "The input object.",
				defaultValue = MeshPrimitive()
			)
		)

	def doOperation( self, args ) :
		return args['input'].copy()

registerRunTimeTyped( polyParam )

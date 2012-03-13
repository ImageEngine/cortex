from IECore import *

class pointParam( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that has point parameters.",
			PointsPrimitiveParameter(
				name = "result",
				description = "A pass through of the input primitive parameter.",
				defaultValue = PointsPrimitive(V3fVectorData([]))
			)
		)

		self.parameters().addParameter(
			PointsPrimitiveParameter(
				name = "input",
				description = "The input object.",
				defaultValue = PointsPrimitive(V3fVectorData([]))
			)
		)

	def doOperation( self, args ) :
		return args['input'].copy()

registerRunTimeTyped( pointParam )

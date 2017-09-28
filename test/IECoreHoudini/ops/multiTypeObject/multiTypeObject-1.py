from IECore import *

class multiTypeObject( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that returns either a MeshPrimitive or a PointsPrimitive, depending on the input type.",
			ObjectParameter(
				name = "result",
				description = "",
				defaultValue = Group(),
				type = TypeId.Primitive
			)
		)

		self.parameters().addParameter(
			ObjectParameter(
				name = "input",
				description = "An object parameter that accepts multiple types.",
				defaultValue = V3fVectorData([]),
				types = [ TypeId.Primitive, TypeId.V3fVectorData ]
			),
		)

	def doOperation( self, args ) :
		obj = args['input']

		if isinstance( obj, V3fVectorData ) :
			return PointsPrimitive( obj )

		return obj

registerRunTimeTyped( multiTypeObject )

import IECore
import IECoreScene

class multiTypeObject( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that returns either a MeshPrimitive or a PointsPrimitive, depending on the input type.",
			IECore.ObjectParameter(
				name = "result",
				description = "",
				defaultValue = IECoreScene.Group(),
				type = IECore.TypeId.Primitive
			)
		)

		self.parameters().addParameter(
			IECore.ObjectParameter(
				name = "input",
				description = "An object parameter that accepts multiple types.",
				defaultValue = IECore.V3fVectorData([]),
				types = [ IECore.TypeId.Primitive, IECore.TypeId.V3fVectorData ]
			),
		)

	def doOperation( self, args ) :
		obj = args['input']

		if isinstance( obj, IECore.V3fVectorData ) :
			return IECoreScene.PointsPrimitive( obj )

		return obj

IECore.registerRunTimeTyped( multiTypeObject )

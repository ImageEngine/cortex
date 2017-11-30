import IECore
import IECoreScene

class primParam( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that has primitive parameters.",
			IECoreScene.PrimitiveParameter(
				name = "result",
				description = "A pass through of the input primitive parameter.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData([]) )
			)
		)

		self.parameters().addParameter(
			IECoreScene.PrimitiveParameter(
				name = "input",
				description = "The input object.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData([]) )
			)
		)

	def doOperation( self, args ) :
		return args['input'].copy()

IECore.registerRunTimeTyped( primParam )

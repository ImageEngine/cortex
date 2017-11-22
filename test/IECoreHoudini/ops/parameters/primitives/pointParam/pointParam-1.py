import IECore
import IECoreScene

class pointParam( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that has point parameters.",
			IECoreScene.PointsPrimitiveParameter(
				name = "result",
				description = "A pass through of the input primitive parameter.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [] ) )
			)
		)

		self.parameters().addParameter(
			IECoreScene.PointsPrimitiveParameter(
				name = "input",
				description = "The input object.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [] ) )
			)
		)

	def doOperation( self, args ) :
		return args['input'].copy()

IECore.registerRunTimeTyped( pointParam )

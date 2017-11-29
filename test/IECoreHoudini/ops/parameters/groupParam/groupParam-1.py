import IECore
import IECoreScene

class groupParam( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that has group parameters.",
			IECoreScene.PrimitiveParameter(
				name = "result",
				description = "A pass through the first child of the input parameter.",
				defaultValue = IECoreScene.MeshPrimitive()
			)
		)

		self.parameters().addParameter(
			IECoreScene.GroupParameter(
				name = "input",
				description = "The input object.",
				defaultValue = IECoreScene.Group()
			)
		)

	def doOperation( self, args ) :

		for child in args["input"].children() :
			if isinstance( child, IECoreScene.Primitive ) :
				return child.copy()

		return IECoreScene.PointsPrimitive( 0 )

IECore.registerRunTimeTyped( groupParam )

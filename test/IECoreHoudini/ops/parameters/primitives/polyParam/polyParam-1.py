import IECore
import IECoreScene

class polyParam( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that has point parameters.",
			IECoreScene.MeshPrimitiveParameter(
				name = "result",
				description = "A pass through of the input primitive parameter.",
				defaultValue = IECoreScene.MeshPrimitive()
			)
		)

		self.parameters().addParameter(
			IECoreScene.MeshPrimitiveParameter(
				name = "input",
				description = "The input object.",
				defaultValue = IECoreScene.MeshPrimitive()
			)
		)

	def doOperation( self, args ) :
		return args['input'].copy()

IECore.registerRunTimeTyped( polyParam )

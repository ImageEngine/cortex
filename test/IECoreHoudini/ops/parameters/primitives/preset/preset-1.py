import IECore
import IECoreScene

class preset( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that has two group parameters.",
			IECoreScene.PrimitiveParameter(
				name = "result",
				description = "A pass through the first child of the input parameter.",
				defaultValue = IECoreScene.MeshPrimitive()
			)
		)

		self.parameters().addParameters( [
			IECore.IntParameter(
				name = "switch",
				description = "Switch between the groups",
				defaultValue = 20,
				presets = (
					( "A", 20 ),
					( "B", 30 ),
				),
				presetsOnly=True
			),

			IECoreScene.PrimitiveParameter(
				name = "inputA",
				description = "The first input object.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData([]) )
			),

			IECoreScene.PrimitiveParameter(
				name = "inputB",
				description = "The second input object.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData([]) )
			)
		] )

	def doOperation( self, args ) :

		primitive = args["inputA"] if args["switch"].value == 20 else args["inputB"]

		if primitive:
			return primitive

		return self.resultParameter().defaultValue.copy()

IECore.registerRunTimeTyped( preset )

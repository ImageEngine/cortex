import IECore

class groupParam( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that has two group parameters.",
			IECore.PrimitiveParameter(
				name = "result",
				description = "A pass through the first child of the input parameter.",
				defaultValue = IECore.MeshPrimitive()
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
			IECore.GroupParameter(
				name = "inputA",
				description = "The first input object.",
				defaultValue = IECore.Group()
			),
			IECore.GroupParameter(
				name = "inputB",
				description = "The second input object.",
				defaultValue = IECore.Group()
			)
		] )

	def doOperation( self, args ) :

		group = args["inputA"] if args["switch"].value == 20 else args["inputB"]

		for child in group.children() :
			if child.isInstanceOf( IECore.TypeId.Primitive ) :
				return child.copy()

		return self.resultParameter().defaultValue.copy()

IECore.registerRunTimeTyped( groupParam )

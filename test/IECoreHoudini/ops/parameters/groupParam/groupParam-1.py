import IECore

class groupParam( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that has group parameters.",
			IECore.PrimitiveParameter(
				name = "result",
				description = "A pass through the first child of the input parameter.",
				defaultValue = IECore.MeshPrimitive()
			)
		)

		self.parameters().addParameter(
			IECore.GroupParameter(
				name = "input",
				description = "The input object.",
				defaultValue = IECore.Group()
			)
		)

	def doOperation( self, args ) :

		for child in args["input"].children() :
			if child.isInstanceOf( IECore.TypeId.Primitive ) :
				return child.copy()

		return IECore.PointsPrimitive( 0 )

IECore.registerRunTimeTyped( groupParam )

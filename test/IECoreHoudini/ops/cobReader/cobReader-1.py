import IECore

class cobReader( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that reads a COB from disk.",
			IECore.ObjectParameter(
				name = "result",
				description = "The Cortex Object read from disk.",
				defaultValue = IECore.NullObject(),
				type = IECore.TypeId.Object
			)
		)

		self.parameters().addParameter(

			IECore.PathParameter(
				name = "filename",
				description = "The path to the COB on disk.",
				defaultValue = "",
			)
		)

	def doOperation( self, args ) :
		filename = args['filename'].value
		obj = IECore.Reader.create(filename).read()
		return obj

IECore.registerRunTimeTyped( cobReader )

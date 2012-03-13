from IECore import *

class cobReader( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that reads a COB from disk.",
			ObjectParameter(
				name = "result",
				description = "The Cortex Object read from disk.",
				defaultValue = NullObject(),
				type = TypeId.Object
			)
		)

		self.parameters().addParameter(

			PathParameter(
				name = "filename",
				description = "The path to the COB on disk.",
				defaultValue = "",
			)
		)

	def doOperation( self, args ) :
		filename = args['filename'].value
		obj = Reader.create(filename).read()
		return obj

registerRunTimeTyped( cobReader )

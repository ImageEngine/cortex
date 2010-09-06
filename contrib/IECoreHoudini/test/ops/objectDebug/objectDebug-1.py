from IECore import *

class objectDebug( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that prints out debug information about an object parameter.",
			ObjectParameter(
				name = "result",
				description = "A pass through of the input object parameter.",
				defaultValue = NullObject(),
				type = TypeId.Object
			)
		)

		self.parameters().addParameter(

			ObjectParameter(
				name = "input",
				description = "The input object.",
				defaultValue = NullObject(),
				type = TypeId.Object
			)
		)
		self.parameters().addParameter(

			BoolParameter(
				name = "quiet",
				description = "Silences the debug output.",
				defaultValue = False
			)
		)

	def doOperation( self, args ) :
		object = args['input']
		quiet = args['quiet'].value
		if not quiet:
			print object.typeName(), object
		return object

registerRunTimeTyped( objectDebug )

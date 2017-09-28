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

			# Print the objects name and its str() representation
			print object.typeName(), object

			# For meshes & points we can print out more verbose information
			if object.typeId()==TypeId.MeshPrimitive or object.typeId()==TypeId.PointsPrimitive:
				for k in object.keys():
					primvar = object[k]
					print "[%s]" % k, primvar.interpolation, primvar.data.typeName()
					print "\t", primvar.data

		return object

registerRunTimeTyped( objectDebug )

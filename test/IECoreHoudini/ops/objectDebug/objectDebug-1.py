import IECore
import IECoreScene

class objectDebug( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that prints out debug information about an object parameter.",
			IECore.ObjectParameter(
				name = "result",
				description = "A pass through of the input object parameter.",
				defaultValue = IECore.NullObject(),
				type = IECore.TypeId.Object
			)
		)

		self.parameters().addParameter(

			IECore.ObjectParameter(
				name = "input",
				description = "The input object.",
				defaultValue = IECore.NullObject(),
				type = IECore.TypeId.Object
			)
		)
		self.parameters().addParameter(

			IECore.BoolParameter(
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
			if object.typeId()==IECoreScene.TypeId.MeshPrimitive or object.typeId()==IECoreScene.TypeId.PointsPrimitive:
				for k in object.keys():
					primvar = object[k]
					print "[%s]" % k, primvar.interpolation, primvar.data.typeName()
					print "\t", primvar.data

		return object

IECore.registerRunTimeTyped( objectDebug )

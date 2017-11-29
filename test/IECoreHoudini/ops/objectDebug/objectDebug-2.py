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

		levelPresets = [ ( str(x), int(x) ) for x in IECore.MessageHandler.Level.values.values() if x != IECore.MessageHandler.Level.Invalid ]

		self.parameters().addParameters( [
			IECore.IntParameter(
				name = "messageLevel",
				description = "The IECore Message Level of the debug info",
				defaultValue = IECore.MessageHandler.Level.Debug,
				presets = levelPresets,
				presetsOnly = True,
			)
		] )

	def doOperation( self, args ) :

		obj = args['input']

		level = IECore.MessageHandler.Level( args["messageLevel"].value )
		IECore.msg( level, "objectDebug", obj.typeName() + " " + str(obj) )

		if obj.isInstanceOf( IECoreScene.TypeId.Primitive ) :
			for k in obj.keys() :
				primvar = obj[k]
				msg = "[%s], %s, %s, %s" % ( k, primvar.interpolation, primvar.data.typeName(), str(primvar.data) )
				IECore.msg( level, "objectDebug", msg )

		return obj

IECore.registerRunTimeTyped( objectDebug )

import IECore
import IECoreScene
import hou

class swapAttribute( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that replaces an attribute on one mesh with that from another.",
			IECoreScene.PrimitiveParameter(
				name = "result",
				description = "The primitive with replaced attribute.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData() ),
			)
		)

		self.parameters().addParameters( [
			IECoreScene.PrimitiveParameter(
				name = "destination",
				description = "The primitive with an attribute to be added/replaced.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData() )
			),
			IECoreScene.PrimitiveParameter(
				name = "source",
				description = "The primitive with an attribute to use.",
				defaultValue = IECoreScene.PointsPrimitive( IECore.V3fVectorData() )
			),
			IECore.StringParameter(
				name = "name",
				description = "The name of the attribute to copy from input2 onto input1.",
				defaultValue = "P"
			)
		] )

	def doOperation( self, args ) :
		dst = args['destination'].copy()
		src = args['source']
		attribute = args['name'].value

		# check for attribute on source
		if not attribute in src:
			raise Exception("Must have primvar '%s' in source primitive!" % attribute)

		dst[attribute] = src[attribute]
		return dst

IECore.registerRunTimeTyped( swapAttribute )

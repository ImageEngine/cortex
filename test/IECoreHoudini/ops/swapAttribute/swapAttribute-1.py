from IECore import *
import hou

class swapAttribute( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that replaces an attribute on one mesh with that from another.",
			PrimitiveParameter(
				name = "result",
				description = "The primitive with replaced attribute.",
				defaultValue = PointsPrimitive(V3fVectorData()),
			)
		)

		self.parameters().addParameters([
			PrimitiveParameter(
				name = "destination",
				description = "The primitive with an attribute to be added/replaced.",
				defaultValue = PointsPrimitive(V3fVectorData()) ),
			PrimitiveParameter(
				name = "source",
				description = "The primitive with an attribute to use.",
				defaultValue = PointsPrimitive(V3fVectorData()) ),
			StringParameter(
				name = "name",
				description = "The name of the attribute to copy from input2 onto input1.",
				defaultValue = "P" )
			]
		)

	def doOperation( self, args ) :
		dst = args['destination'].copy()
		src = args['source']
		attribute = args['name'].value

		# check for attribute on source
		if not attribute in src:
			raise Exception("Must have primvar '%s' in source primitive!" % attribute)

		dst[attribute] = src[attribute]
		return dst

registerRunTimeTyped( swapAttribute )

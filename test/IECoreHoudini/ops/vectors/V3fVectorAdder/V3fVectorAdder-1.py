import IECore
from six.moves import zip

class V3fVectorAdder( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that adds two V3fVectorData objects together.",
			IECore.ObjectParameter(
				name = "result",
				description = "A V3fVectorData object.",
				defaultValue = IECore.V3fVectorData(),
				type = IECore.TypeId.V3fVectorData
			)
		)

		self.parameters().addParameters([
			IECore.ObjectParameter(
				name = "vector1",
				description = "The first V3fVectorData object.",
				defaultValue = IECore.V3fVectorData(),
				type = IECore.TypeId.V3fVectorData
			),
			IECore.ObjectParameter(
				name = "vector2",
				description = "The second V3fVectorData object.",
				defaultValue = IECore.V3fVectorData(),
				type = IECore.TypeId.V3fVectorData
			) ]
		)

	def doOperation( self, args ) :
		v1 = args['vector1']
		v2 = args['vector2']
		if len(v1)!=len(v2):
			raise "Vector array lengths must match!"
		result = IECore.V3fVectorData( [ x+y for (x,y) in zip(v1,v2) ] )
		return result

IECore.registerRunTimeTyped( V3fVectorAdder )

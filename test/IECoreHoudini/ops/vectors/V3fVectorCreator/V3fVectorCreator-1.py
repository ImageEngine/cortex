import IECore

class V3fVectorCreator( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"Op that generates a V3fVectorData object.",
			IECore.ObjectParameter(
				name = "result",
				description = "A V3fVectorData object.",
				defaultValue = IECore.V3fVectorData(),
				type = IECore.TypeId.V3fVectorData
			)
		)

		self.parameters().addParameters([
			IECore.IntParameter(
				name = "size",
				description = "The number of elements to put in the result vector.",
				defaultValue = 1
			),
			IECore.V3fParameter(
				name = "value",
				description = "Value to put in each of the vector elements.",
				defaultValue = IECore.V3f(1)
			) ]
		)

	def doOperation( self, args ) :
		size = args['size'].value
		value = args['value'].value
		data = [ value for x in range(size) ]
		result = IECore.V3fVectorData( data )
		return result

IECore.registerRunTimeTyped( V3fVectorCreator )

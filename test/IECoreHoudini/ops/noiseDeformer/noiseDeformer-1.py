from IECore import *
import hou

class noiseDeformer( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that displaces verts along their normals using some noise.",
			PrimitiveParameter(
				name = "result",
				description = "The primitive with displaced verts.",
				defaultValue = PointsPrimitive(V3fVectorData()),
			)
		)

		self.parameters().addParameters([
			PrimitiveParameter(
				name = "input",
				description = "The primitive to work on.",
				defaultValue = PointsPrimitive(V3fVectorData()) ),
			FloatParameter(
				name = "magnitude",
				description = "The amount to displace by.",
				defaultValue = 1.0 ),
			V3fParameter(
				name = "frequency",
				description = "The frequency of the displacement noise.",
				defaultValue = V3f(1.0) )
			]
		)

	def doOperation( self, args ) :

		# take a copy of our input parameter
		prim = args['input'].copy()

		# check for P & N
		if not "P" in prim:
			raise Exception("Must have primvar 'P' in primitive!")
		if not "N" in prim:
			error( "Must have primvar 'N' in primitive!" )
			return PointsPrimitive( 1 )

		# get our magnitude & frequency parameters
		mag = args['magnitude'].value
		freq = args['frequency'].value

		# calculate a new set of positions
		p_data = prim['P'].data
		new_p = []
		for p in p_data:
			noise_val = mag * ( hou.hmath.noise3d( [p.x * freq.x, p.y * freq.y, p.z * freq.z] ) - hou.Vector3(.5,.5,.5) ) * 2
			new_p.append( p + V3f( noise_val[0], noise_val[1], noise_val[2] ) )

		# overwrite with our new P and return from the Op
		prim['P'] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData( new_p ) )

		# recalculate normals
		if prim.typeId()==TypeId.MeshPrimitive:
			MeshNormalsOp()( input=prim, copyInput=False )

		return prim

registerRunTimeTyped( noiseDeformer )

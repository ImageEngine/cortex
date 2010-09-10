from IECore import *

class compoundParameters( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op with some compound parameters.",
			ObjectParameter(
				name = "result",
				description = "Dummy.",
				defaultValue = PointsPrimitive(V3fVectorData()),
				type = TypeId.PointsPrimitive
			)
		)

		self.parameters().addParameters( [
										
			CompoundParameter(
				name = "compound_1",
				description = "a compound parameter",
				userData = { "label":StringData("My Compound 1") },
				members = [
					V3dParameter(
						name = "j",
						description = "a v3d",
						defaultValue = V3dData( V3d( 8, 16, 32 ) ),
						userData = { "label":StringData("A Vector") }
					),
					Color3fParameter(
						name = "k",
						description = "an m44f",
						defaultValue = Color3f(1,0.5,0),
						userData = { "label":StringData("A Colour") }
					),
					]),		
										
			CompoundParameter(
				name = "compound_2",
				description = "a compound parameter",
				userData = { "label":StringData("My Compound 2") },
				members = [
					V3dParameter(
						name = "j",
						description = "a v3d",
						defaultValue = V3dData( V3d( 8, 16, 32 ) ),
						presets = (
							( "one", V3d( 1 ) ),
							( "two", V3d( 2 ) )
						),
						userData = { "label":StringData("Compound->V3d") }
					),
					V2fParameter(
						name = "k",
						description = "an v2f",
						defaultValue = V2f(1,1)
					),
					]),		
										
			CompoundParameter(
				name = "compound_3",
				description = "a compound parameter",
				userData = { "label":StringData("My Compound 3") },
				members = [
							CompoundParameter(
								name = "compound_4",
								description = "a compound parameter",
								userData = { "label":StringData("My Compound 4") },
								members = [
									IntParameter(
										name = "some_int",
										description = "Int",
										defaultValue = 123,
										userData = { "label":StringData("Int") }
									),
									])
						]),
						
			FloatParameter(
						name="blah",
						description="blah",
						defaultValue = 123.0 ),
										
			CompoundParameter(
				name = "compound_5",
				description = "a compound parameter",
				userData = { "label":StringData("Another Compound Parameter") },
				members = [
							BoolParameter(
								name = "bool_1",
								description = "a boolean parameter",
								defaultValue = True )
									])
			])

	def doOperation( self, args ) :
		return PointsPrimitive(V3fVectorData())

registerRunTimeTyped( compoundParameters )

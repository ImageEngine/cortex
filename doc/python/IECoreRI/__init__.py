
from _IECoreRI import *
from ExecuteProcedural import executeProcedural


import IECore
import IECoreRI

## This convenience function loads a named and versioned procedural
# using the default IECore procedural loader, parses the args string
# to set the procedural parameters and then renders the procedural. It's
# useful to use this in the command string for a dynamicload procedural
# that executes python.
def executeProcedural( name, version, args ) :

	procedural = IECore.ClassLoader.defaultProceduralLoader().load( name, version )()

	if procedural :

		IECore.ParameterParser().parse( args, procedural.parameters() )

		renderer = IECoreRI.Renderer()
		procedural.render( renderer, inAttributeBlock=False, withState=False, withGeometry=True, immediateGeometry=True )

from MakeRibOp import MakeRibOp


import IECore
import IECoreRI

class MakeRibOp( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "Converts files to rib format.",
			IECore.FileNameParameter(
				name = "result",
				description = "The name of the new rib file.",
				defaultValue = "",
				check = IECore.FileNameParameter.CheckType.DontCare,
				allowEmptyString = True,
			)
		)

		self.parameters().addParameters(

			[
				IECore.FileNameParameter(
					name = "src",
					description = "The name of an input file to convert to rib format.",
					defaultValue = "",
					check = IECore.FileNameParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				IECore.FileNameParameter(
					name = "dst",
					description = "The name of the destination rib file.",
					defaultValue = "",
					check = IECore.FileNameParameter.CheckType.DontCare,
					allowEmptyString = False,
				),
				IECore.BoolParameter(
					name = "addWorld",
					description = "Turn this on to enclose the generated rib in a WorldBegin/WorldEnd block.",
					defaultValue = False,
				),
			]
		)

	def doOperation( self, operands ) :

		reader = IECore.Reader.create( operands.src.value )
		if not reader :
			raise Exception( "Unable to create a Reader for \"%s\." % operands.src.value )

		renderable = reader.read()
		if not renderable or not renderable.inheritsFrom( IECore.Renderable.staticTypeId() ) :
			raise Exception( "\"%s\ does not contain a Renderable object." % operands.src.value )

		renderer = IECoreRI.RIRenderer( operands.dst.value )
		if operands.addWorld.value :
			renderer.worldBegin()

		renderable.render( renderer )

		if operands.addWorld.value :
			renderer.worldEnd()

		return operands.dst

IECore.registerRunTimeTyped( MakeRibOp )


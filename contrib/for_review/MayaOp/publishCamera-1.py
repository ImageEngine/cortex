import os
import subprocess

import IECore

class MayaOp( IECore.Op ) :

	def __init__( self, description ) :

		IECore.Op.__init__( self, description, IECore.IntParameter( "result", "The return status.", 0  ) )

		if self.mayaModule() is None :

			self.parameters().addParameters(

				[

					IECore.FileNameParameter(
						name = "mayaFile",
						description = "The file to perform the publish from",
						defaultValue = "",
						check = IECore.FileNameParameter.CheckType.MustExist,
						extensions = "ma mb",
					)

				]
			)

	def doOperation( self, args ) :

		if self.mayaModule() is None :

			# really we'd expect a maya wrapper to take care of this,
			# but just in case it isn't, make sure maya doesn't get
			# launched with the wrong version of python.
			env = os.environ.copy()
			del env["PYTHONHOME"]

			serialisedArgs = IECore.ParameterParser().serialise( self.parameters() )
			# remove the mayaFile argument
			del serialisedArgs[:2]

			pythonCommands = [
				"import IECore",
				"op = IECore.ClassLoader.defaultLoader( 'IECORE_OP_PATHS' ).load( '%s', %d )" % ( self.path, self.version ),
				"opInstance = op()",
				"IECore.ParameterParser().parse( %s, opInstance.parameters() )" % str( serialisedArgs ),
				"opInstance()",
			]

			melCommand = ";\n".join( [ "python( \"%s\" )" % c for c in pythonCommands ] )
			melCommand += ";\nquit -force;"

			f = open( "/tmp/test.mel", "w" )
			f.write( melCommand + "\n" )
			f.close()

			subprocess.check_call( [ "/software/maya/linux.centos6.x86_64/2012/bin/maya", "-batch", "-file", args["mayaFile"].value, "-script", "/tmp/test.mel" ], env = env )

			return IECore.IntData( 0 )

		else :

			return self.doMayaOperation( args )

	def doMayaOperation( self, args ) :

		raise NotImplementedError

	__mayaModule = False
	@classmethod
	def mayaModule( cls ) :

		if cls.__mayaModule is False :
			try :
				cls.__mayaModule = __import__( "maya" )
			except ImportError :
				cls.__mayaModule = None

		return cls.__mayaModule

IECore.registerRunTimeTyped( MayaOp )










class publishCamera( MayaOp ) :

	def __init__( self ) :

		MayaOp.__init__( self, "I publish cameras" )

		self.parameters().addParameters(

			[

				IECore.FileNameParameter(
					name = "outputFile",
					description = "Where the camera will be published",
					defaultValue = "",
					allowEmptyString = False,
				),

			]

		)

	def doMayaOperation( self, args ) :

		maya = self.mayaModule()
		camera = maya.cmds.ls( type="camera" )
		maya.cmds.select( camera, replace=True )
		maya.cmds.file( args["outputFile"].value, exportSelected=True, typ="mayaAscii" )

		return IECore.IntData( 0 )

IECore.registerRunTimeTyped( publishCamera )

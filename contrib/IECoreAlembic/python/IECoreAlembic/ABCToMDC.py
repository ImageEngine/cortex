##########################################################################
#
#  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import os
import glob

import IECore
import IECoreAlembic

class ABCToMDC( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "Converts Alembic files into Cortex ModelCache files.", IECore.FileNameParameter( "result", "" ) )

		self.parameters().addParameters(

			[

				IECore.FileNameParameter(
					"inputFile",
					"The alembic file to be converted",
					defaultValue = "",
					allowEmptyString = False,
					check = IECore.FileNameParameter.CheckType.MustExist,
					extensions = "abc",
				),

				IECore.FileNameParameter(
					"outputFile",
					"The filename of the model cache to be written",
					defaultValue = "",
					allowEmptyString = False,
					extensions = "mdc",
				),

			],

		)

	def doOperation( self, args ) :

		inFile = IECoreAlembic.AlembicInput( args["inputFile"].value )
		outFile = IECore.ModelCache( args["outputFile"].value, IECore.IndexedIO.OpenMode.Write )

		time = inFile.timeAtSample( 0 )

		def walk( alembicInput, modelCache ) :

			o = alembicInput.objectAtTime( time, IECore.Primitive.staticTypeId() )
			if o is not None :
				modelCache.writeObject( o )

			t = alembicInput.transformAtTime( time )
			modelCache.writeTransform( t )

			numChildren = alembicInput.numChildren()
			for i in range( 0, numChildren ) :
				alembicChild = alembicInput.child( i )
				modelCacheChild = modelCache.writableChild( alembicChild.name() )
				walk( alembicChild, modelCacheChild )

		walk( inFile, outFile )

		return args["outputFile"].value

IECore.registerRunTimeTyped( ABCToMDC )

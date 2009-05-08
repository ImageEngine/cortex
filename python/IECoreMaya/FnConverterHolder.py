##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
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

import maya.cmds
import IECore
import IECoreMaya
import os.path

class FnConverterHolder( IECoreMaya.FnParameterisedHolder ) :

	def __init__( self, node ) :

		IECoreMaya.FnParameterisedHolder.__init__( self, node )

		if self.typeName()!="ieConverterHolder" :

			raise TypeError( "\"%s\" is not a ConverterHolder." )

	## Returns the converter held by this node
	def converter( self ) :

		c = self.getParameterised()
		c = c[0]
		if not c or not c.isInstanceOf( IECoreMaya.Converter.staticTypeId() ) :
			return None

		return c

	## Performs a conversion at the specified frame
	def convertAtFrame( self, frame ) :

		c = self.converter()
		if not c :
			raise RuntimeError( "No converter found on node \"%s\"." % self.name() )

		fileName = str( maya.cmds.getAttr( self.name() + ".fileName", asString = True ) )
		try :
			f = IECore.FileSequence( fileName, IECore.FrameRange( frame, frame ) )
			fileName = f.fileNameForFrame( frame )
		except :
			pass

		if fileName=="" :
			raise RuntimeError( "No filename specified on node \"%s\"." % self.name() )

		maya.cmds.currentTime( frame )
		o = c.convert()
		if not o :
			raise RuntimeError( "Conversion failed for node \"%s\"." % self.name() )

		w = IECore.Writer.create( o, fileName )
		if not w :
			ext = os.path.splitext( fileName )[1]
			raise RuntimeError(	"Unable to create a Writer for object of type \"%s\" and file type \"%s\"." % ( o.typeName(), ext ) )

		w.write()

	## Performs a conversion for every frame in an IECore.FrameList object
	def convertAtFrames( self, frameList ) :

		for f in frameList.asList() :

			self.convertAtFrame( f )

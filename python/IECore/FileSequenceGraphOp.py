##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

# \ingroup python

import os, copy
from IECore import *

# Creates a bar graph object that represents the file sizes as bar height and file status as bar color.
class FileSequenceGraphOp( FileSequenceAnalyzerOp ):

	def __init__( self ):

		FileSequenceAnalyzerOp.__init__( self, 
			name = 'FileSequenceGraphOp',
			description = 'Creates a bar graph object that represents the file sizes as bar height and file status as bar color.',
			resultParameter = ObjectParameter( 
								name = 'barGraph', 
								description = 'Resulting bar graph object that represents the input file sequence.', 
								defaultValue = NullObject(), 
								type = TypeId.Group, 
			)
		)

		self.userData()["UI"] = CompoundObject(
			{
				"showResult": BoolData( True ),
				"farmExecutable": BoolData( False ),
			}
		)

	def doOperation( self, args ) :

		frameInfos = self.frameInfos()
		frames = self.allFrames()

		# get the max file size to normalize later.
		maxSize = -1
		for v in frameInfos.values():
			s = v.get('size', -1)
			if maxSize < s:
				maxSize = s

		# create a Mesh primitive as a litle square 1x1 aligned to plane X,Y
		mesh = MeshPrimitive.createPlane( Box2f( V2f(0,0), V2f(1,1) ) )

		# create the main group
		graph = Group()

		colors = {
			'ok': Color3f( 0.1, 0.7, 0.1 ),
			'missing': Color3f( 0.5, 0.3, 0.7 ),
			'corrupted': Color3f( 0.8, 0.3, 0.3 ),
			'suspicious': Color3f( 1, 1, 0.3 ),
		}

		# set a value that will be added to the height of every file. So even super small files will be visible on the graph.
		baseHeight = maxSize/6.

		# create one Group for each frame in the file sequence.
		for (fi, f) in enumerate(frames):

			info = frameInfos[f]

			frameGrp = Group()
			m = M44f()
			m.translate( V3f( fi, 0, 0 ) )
			m.scale( V3f( 1.0, info.get('size', maxSize) + baseHeight, 1.0 ) )
			frameGrp.setTransform( MatrixTransform( m ) )
			state = AttributeState()
			state.attributes['name'] = StringData( info['path'] )
			state.attributes['color'] = Color3fData( colors[ info['type'] ] )
			frameGrp.addState( state )
			frameGrp.addChild( mesh )

			graph.addChild( frameGrp )

		m = M44f()
		m.translate( V3f( -1./2., -1./10., 0 ) )
		m.scale( V3f( 1. / len( frames ), (1. / (maxSize +  + baseHeight)) / 5., 1. ) )
		graph.setTransform( MatrixTransform( m ) )
		# Indicate this Renderable object is meant to be a hyperlink to entities on the file system.
		graph.blindData()['hyperlink'] = StringData('FileSystem')
		return graph

makeRunTimeTyped( FileSequenceGraphOp, 100020, FileSequenceAnalyzerOp )

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

import maya.cmds
import math
import IECore
import IECoreMaya

class BakeTransform( IECore.Op ) :

	def __init__( self ) :
	
		IECore.Op.__init__( self, "BakeTransform", "Bakes transforms from one object onto another.",
			IECoreMaya.DAGPathParameter(
				name = "result",
				description = "The name of the baked transform",
				defaultValue = "",
				allowEmptyString = True,
			)
		)
		
		self.parameters().addParameters(
			[
				IECoreMaya.DAGPathParameter(
					name = "src",
					description = "The source transform node to bake from.",
					defaultValue = "",
					check = IECoreMaya.DAGPathParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				IECoreMaya.DAGPathParameter(
					name = "dst",
					description = "The destination transform node to apply the bake to. If this doesn't exist"
						"then it'll be created for you.",
					defaultValue = "",
					check = IECoreMaya.DAGPathParameter.CheckType.DontCare,
					allowEmptyString = False,
				),
				IECore.FrameListParameter(
					name = "frames",
					description = "The frame range over which to perform the bake."
						"Keyframes will be made at every frame in this range.",
					defaultValue = IECoreMaya.PlaybackFrameList( IECoreMaya.PlaybackFrameList.Range.Playback ),
				),
				IECoreMaya.BoolParameter(
					name = "lock",
					description = "If this is specified then the transform attributes which are"
						"keyframed on the destination object are also locked.",
					defaultValue = False,
				),	
			]
		)
		
	def doOperation( self, operands ) :

		if maya.cmds.objExists( operands.dst.value ) :
			dst = self.dst.getDAGPathValue()
		else :
			dst = maya.cmds.createNode( "transform", name=operands.dst.value, skipSelect=True )
			dst = IECoreMaya.DagNode( str(dst) )

		worldMatrixPlug = self.src.getDAGPathValue().plug( "worldMatrix" )[0]

		for f in self.frames.getFrameListValue().asList() :

			maya.cmds.currentTime( float( f ) )

			worldMatrix = worldMatrixPlug.convert().value
			
			e = IECore.Eulerf()
			e.extract( worldMatrix.rotate )

			maya.cmds.xform( dst, translation=tuple( worldMatrix.translate ), rotation=[math.degrees(x) for x in e], scale=tuple( worldMatrix.scale ) )
			maya.cmds.setKeyframe( dst, attribute=["translate", "rotate", "scale"] )
			
		# fix discontinuous rotations
		maya.cmds.filterCurve( str( dst.plug( "rotateX" ) ), str( dst.plug( "rotateY" ) ), str( dst.plug( "rotateZ" ) ) )	

		if operands.lock.value :
			maya.cmds.setAttr( str( dst.plug( "translate" ) ), lock=True )
			maya.cmds.setAttr( str( dst.plug( "rotate" ) ), lock=True )
			maya.cmds.setAttr( str( dst.plug( "scale" ) ), lock=True )
			maya.cmds.setAttr( str( dst.plug( "shear" ) ), lock=True )

		return IECore.StringData( str( dst ) )

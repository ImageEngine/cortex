##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreScene

class multiple( IECoreScene.ParameterisedProcedural ) :

	def __init__( self ) :
		IECoreScene.ParameterisedProcedural.__init__( self, "Renders all of it's input geo" )
		self.parameters().addParameters( [
			IECoreScene.MeshPrimitiveParameter(
				name = "mesh",
				description = "A mesh (Houdini polygons)",
				defaultValue = IECoreScene.MeshPrimitive(),
			),
			IECoreScene.PointsPrimitiveParameter(
				name = "points",
				description = "Points",
				defaultValue = IECoreScene.PointsPrimitive( 0 ),
			),
			IECore.ObjectParameter(
				name = "meshOnlyObject",
				description = "A mesh (Houdini polygons)",
				defaultValue = IECoreScene.MeshPrimitive(),
				types = [ IECore.TypeId.MeshPrimitive ]
			),
			IECore.ObjectParameter(
				name = "meshPointsOrGroupObject",
				description = "A mesh (Houdini polygons) or points",
				defaultValue = IECoreScene.MeshPrimitive(),
				types = [ IECore.TypeId.MeshPrimitive, IECore.TypeId.PointsPrimitive, IECore.TypeId.Group ]
			),
		] )

	def doBound( self, args ) :
		meshBound = args['mesh'].bound()
		pointsBound = args['points'].bound()
		meshPointsOrGroupBound = args['meshPointsOrGroupObject'].bound()
		meshOnlyBound = args['meshOnlyObject'].bound()

		bound = IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 0 ) )
		bound.min = min( min( min( meshBound.min, pointsBound.min ), meshPointsOrGroupBound.min ), meshOnlyBound.min )
		bound.max = max( max( max( meshBound.max, pointsBound.max ), meshPointsOrGroupBound.max ), meshOnlyBound.max )

		return bound

	def doRenderState( self, renderer, args ) :
		pass

	def doRender( self, renderer, args ) :
		args['mesh'].render( renderer )
		args['points'].render( renderer )
		args['meshPointsOrGroupObject'].render( renderer )
		args['meshOnlyObject'].render( renderer )

IECore.registerRunTimeTyped( multiple )

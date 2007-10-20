##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
import IECoreRI

class MakeRibOp( IECore.Op ) :

	def __init__( self ) :
	
		IECore.Op.__init__( self, "MakeRibOp", "Converts files to rib format.",
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

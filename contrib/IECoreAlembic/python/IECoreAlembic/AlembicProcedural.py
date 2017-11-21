##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
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
import IECoreAlembic

class AlembicProcedural( IECoreScene.ParameterisedProcedural ) :

	def __init__( self ) :

		IECoreScene.ParameterisedProcedural.__init__( self )

		self.parameters().addParameters(

			[

				IECore.FileNameParameter(
					name = "fileName",
					description = "The filename of an Alembic cache.",
					defaultValue = "",
					allowEmptyString = True,
					check = IECore.PathParameter.CheckType.MustExist,
					extensions = "abc",
				),

				IECore.FloatParameter(
					name = "time",
					description = "The time at which to load from the Alembic cache",
					defaultValue = 0,
					userData = {
						"maya" : {
							"defaultExpression" : IECore.StringData( " = time" ),
						},
					},
				),

			],

		)

		self.__input = None
		self.__inputFileName = None

	def doBound( self, args ) :

		a = self.__alembicInput( args )
		if a is None :
			return IECore.Box3f()

		return _ChildProcedural( a, args["time"].value ).bound()

	def doRenderState( self, renderer, args ) :

		pass

	def doRender( self, renderer, args ) :

		a = self.__alembicInput( args )
		if a is None :
			return

		renderer.procedural( _ChildProcedural( a, args["time"].value ) )

	def __alembicInput( self, args ) :

		if self.__input is not None and self.__inputFileName == args["fileName"].value :
			return self.__input

		self.__input = None
		self.__inputFileName = args["fileName"].value
		if args["fileName"].value :
			try :
				self.__input = IECoreAlembic.AlembicInput( args["fileName"].value )
			except :
				IECore.msg( IECore.Msg.Level.Error, "AlembicProcedural", "Unable to open file \"%s\"" % args["fileName"].value )

		return self.__input

class _ChildProcedural( IECoreScene.Renderer.Procedural ) :

	def __init__( self, alembicInput, time ) :

		IECoreScene.Renderer.Procedural.__init__( self )

		self.__alembicInput = alembicInput
		self.__time = time

	def bound( self ) :

		b = self.__alembicInput.boundAtTime( self.__time )
		b = b.transform( self.__alembicInput.transformAtTime( self.__time ) )
		return IECore.Box3f( IECore.V3f( b.min ), IECore.V3f( b.max ) )

	def render( self, renderer ) :

		with IECoreScene.AttributeBlock( renderer ) :

			renderer.setAttribute( "name", self.__alembicInput.fullName() )

			transform = self.__alembicInput.transformAtTime( self.__time )
			if transform is not None :
				transform = IECore.M44f(
					transform[0,0], transform[0,1], transform[0,2], transform[0,3],
				    transform[1,0], transform[1,1], transform[1,2], transform[1,3],
				    transform[2,0], transform[2,1], transform[2,2], transform[2,3],
				    transform[3,0], transform[3,1], transform[3,2], transform[3,3]
				)
				renderer.concatTransform( transform )

			primitive = self.__alembicInput.objectAtTime( self.__time, IECoreScene.Primitive.staticTypeId() )
			if primitive is not None :
				primitive.render( renderer )

			for childIndex in range( 0, self.__alembicInput.numChildren() ) :
				child = self.__alembicInput.child( childIndex )
				childProcedural = _ChildProcedural( child, self.__time )
				if child.hasStoredBound() :
					renderer.procedural( childProcedural )
				else :
					childProcedural.render( renderer )

IECore.registerRunTimeTyped( AlembicProcedural )

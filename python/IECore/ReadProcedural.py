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

from IECore import *
import math

class ReadProcedural( Renderer.Procedural ) :

	def __init__( self ) :
	
		Renderer.Procedural.__init__( self, "ReadProcedural", "A procedural to render simple objects which can be loaded with a Reader." )
		
		self.parameters().addParameters(
		
			[
				CompoundParameter(
					name = "files",
					description = "Parameters controlling what files are rendered.",
					members = [
						FileNameParameter(
							name = "name",
							description = "The filename. This can include a series of one or more # characters to specify a file sequence. It may also include a @ character which will be substituted with each of the file numbers specified in the parameter below.",
							defaultValue = "",
							allowEmptyString = True,
							check = PathParameter.CheckType.DontCare,
						),
						StringParameter(
							name = "numbers",
							description = "A list of numbers to be substituted for the @ in the name above, to specify multiple files to render. This uses the same syntax as for frame ranges - e.g 1-10, 13, 15, 20-40x2.",
							defaultValue = "",
						),
						IntParameter(
							name = "frame",
							description = "The frame number of the files to render. This is substituted for any # characters in the filename.",
							defaultValue = 1,
							userData = CompoundObject( {
								"maya" : {
									"defaultConnection" : StringData( "time1.outTime" ),
								}
							} ),
						)
					]
				),
				
				CompoundParameter(
					name = "bounds",
					description = "Parameters controlling how the bounds are calculated for the particles.",
					members = [
						StringParameter(
							name = "mode",
							description = "How the bounds are calculated.",
							defaultValue = "calculated",
							presets = {
								"Calculated" : "calculated",
								"Specified" : "specified",
							},
							presetsOnly = True,
						),
						Box3fParameter(
							name = "specified",
							description = "The bounding box to use when bounding box mode is set to 'specified'.",
							defaultValue = Box3f( V3f( -1 ), V3f( 1 ) ),
						)
					]
				),
				CompoundParameter(
					name = "motion",
					description = "Parameters controlling motion.",
					members = [
						BoolParameter(
							name = "blur",
							description = "When this is on, motion blur is applied by loading two files per frame.",
							defaultValue = True
						)
					]
				),
			]
		
		)
	
	def doBound( self, args ) :
			
		if args.bounds.mode.value=="specified" :
		
			return args.bounds.specified.value
		
		else :
		
			bound = Box3f()
			fileNames = self.__allFileNames( args )
			for fileName in fileNames :
			
				if type( fileName ) is str :
				
					o = self.__readFile( fileName )
					if o :
						bound.extendBy( o.bound() )
						
				else :
				
					o = self.__readFile( fileName[0] )
					if o :
						bound.extendBy( o.bound() )
						
					o = self.__readFile( fileName[1] )
					if o :
						bound.extendBy( o.bound() )	
						
			return bound
		
	def doRender( self, renderer, args ) :
			
		fileNames = self.__allFileNames( args )
		shutter = renderer.getOption( "shutter" ).value

		for fileName in fileNames :
		
			if type( fileName ) is str :
			
				o = self.__readFile( fileName )
				if o :
					o.render( renderer )
					
			else :
			
				if shutter[0]==shutter[1] :
					o = self.__readFile( fileName[0] )
					if o :
						o.render( renderer )
				else :
				
					o0 = self.__readFile( fileName[0] )
					o1 = self.__readFile( fileName[1] )

					if o0 and o1 :

						m = MotionPrimitive()
						m[shutter[0]] = o0
						m[shutter[0]+1] = o1

						m.render( renderer )

	# returns either a list of filenames (no motion blur) or a list of tuples of filenames (for motion blur)
	def __allFileNames( self, args ) :
	
		if args.files.name.value=="" :
			return []
	
		frame = args.files.frame.value

		result = []
		if FileSequence.fileNameValidator().match( args.files.name.value ) :
			sequence = FileSequence( args.files.name.value, FrameRange( frame, frame ) )
			if args.motion.blur.value :
				result.append( ( sequence.fileNameForFrame( frame ), sequence.fileNameForFrame( frame + 1 ) ) )
			else :
				result.append( sequence.fileNameForFrame( frame ) )
		else :
			result.append( args.files.name.value )
				
		if "@" in args.files.name.value :
		
			numbers = FrameList.parse( args.files.numbers.value ).asList()
			
			newResult = []
			for f in result :
			
				for n in numbers :
				
					if type( f ) is str :

						newResult.append( f.replace( "@", str( n ) ) )

					else :

						f0 = f[0].replace( "@", str( n ) )
						f1 = f[1].replace( "@", str( n ) )
						newResult.append( ( f0, f1 ) )
			
		return result
	
	def __readFile( self, fileName ) :
	
		reader = Reader.create( fileName )
		if not reader :
			msg( Msg.Level.Error, "Read procedural", "Unable to create a Reader for '%s'." % fileName )
			return None
		
		o = reader.read()
		if not o or not o.isInstanceOf( "VisibleRenderable" ) :
			msg( Msg.Level.Error, "Read procedural", "Failed to load an object of type VisibleRenderable for '%s'." % fileName )
			return None
			
		return o

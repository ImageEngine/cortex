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

class LsHeaderOp( Op ) :

	def __init__( self ) :
	
		Op.__init__( self, "LsHeaderOp", "Lists the contents of file headers using reader classes such as AttributeCache, HierarchicalCache and Reader.",
			Parameter(
				name = "result",
				description = "A list of meta-data contained in the file header.",
				defaultValue = StringVectorData()
			)
		)
		
		self.parameters().addParameters(
		
			[
				FileNameParameter(
					name = "file",
					description = "The file to list the header from.",
					defaultValue = "",
					check = FileNameParameter.CheckType.MustExist,
					extensions = " ".join( IndexedIOInterface.supportedExtensions() + Reader.supportedExtensions() ),
					allowEmptyString = False,
				),

				StringParameter(
					name = "resultType",
					description = "The format of the result",
					defaultValue = "string",
					presets = {
						"string" : "string",
						"stringVector" : "stringVector",
					},
					presetsOnly = True,
				)
			]
		)

		self.userData()["UI"] = CompoundObject( 
									{
										"showResult": BoolData( True ),
										"closeAfterExecution": BoolData( True ),
									}
								)

	def doOperation( self, operands ) :

		headers = None
		try:
			cache = HierarchicalCache( operands.file.value, IndexedIOOpenMode.Read )
			headers = cache.readHeader()
		except:
			debugException( "Error reading header as HierarchicalCache." )
			try:
				cache = AttributeCache( operands.file.value, IndexedIOOpenMode.Read )
				headers = cache.readHeader()
			except:
				debugException( "Error reading header as AttributeCache." )
				try:
					reader = Reader.create( operands.file.value )
					headers = reader.readHeader()
				except:
					debugException( "Error reading header as Reader." )
					headers = None

		if headers is None:
			raise Exception, ("Could not get header from file " + operands.file.value)

		def formatCompound( compound, lines, level = 0 ):
			levelStr = "";
			i = 0
			while( i < level ):
				levelStr += "     "
				i += 1

			for key in compound.keys():
				value = compound[ key ]
				if isinstance( value, CompoundObject ) or isinstance( value, CompoundData ):
					lines.append( levelStr + key + ": " )
					formatCompound( value, lines, level + 1 )
				elif isSimpleDataType( value ):
					lines.append( levelStr + key + ": " + str(value.value) )
				elif isSequenceDataType( value ):
					lines.append( levelStr + key + ": " + ", ".join( map( str, value ) ) )
				else:
					lines.append( levelStr + key + ": " + str(value) )

		headerLines = []
		formatCompound( headers, headerLines )
	
		if operands.resultType.value == "string" :
			return StringData( "\n".join( headerLines ) )
		else :
			return StringVectorData( headerLines )
		
makeRunTimeTyped( LsHeaderOp, 100015, Op )

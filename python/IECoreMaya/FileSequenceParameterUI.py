##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

import os.path

import IECore
import IECoreMaya

class FileSequenceParameterUI( IECoreMaya.PathParameterUI ) :

	def __init__( self, node, parameter, **kw ):

		IECoreMaya.PathParameterUI.__init__( self, node, parameter, **kw )

	def _fileDialog( self ) :

		tools = FileSequenceParameterUI.FileSequenceFilter( self.parameter.extensions )

		IECoreMaya.PathParameterUI._fileDialog( self,
			filter = tools.filter,
			validate = tools.validate,
		)

	class FileSequenceFilter :

		def __init__( self, extensions=None ) :

			if extensions:
				self.__extensions = IECore.StringVectorData( extensions )
			else:
				self.__extensions = IECore.StringVectorData()

		def filter( self, path, items ) :

			fsOp = IECore.SequenceLsOp()

			oldItems = list( items )
			del items[:]

			for i in oldItems:
				if os.path.isdir( i["path"] ) :
					items.append( i )

			sequences = fsOp(
				dir=path,
				type="files",
				resultType="stringVector",
				format="<PREFIX><#PADDING><SUFFIX> <FRAMES>",
				extensions=self.__extensions,
			)

			for s in sequences :

				firstFrame = IECore.FileSequence( s ).fileNames()[0]
				stat = os.stat( firstFrame )

				seqItem = {
					"path" : s,
					"name" : s.replace( "%s/" % path, "" ),
					"mode" :  stat[0],
					"uid" :  stat[4],
					"gid" :  stat[5],
					"size" :  stat[6],
					"atime" :  stat[7],
					"mtime" :  stat[8],
					"ctime" :  stat[9],
				}

				items.append( seqItem )

		# FileExtensionFilter will get confused by the extra info on
		# the end of the sequence string.
		def validate( self, path, items ):

			if not items:
				return False

			for i in items:
				if os.path.isdir( "%s/%s" % ( path, i["name"] ) ) :
					return False

			return True



IECoreMaya.ParameterUI.registerUI( IECore.TypeId.FileSequenceParameter, FileSequenceParameterUI )

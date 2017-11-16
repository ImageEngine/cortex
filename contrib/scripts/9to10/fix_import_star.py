##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

import lib2to3
from lib2to3.fixer_base import BaseFix
from lib2to3.pgen2 import token
from lib2to3.fixer_util import Name, Newline
from lib2to3.pytree import Leaf, Node
from lib2to3.pygram import python_symbols as syms

import IECore

class FixImportStar( BaseFix ) :

	PATTERN = """
		regularImport=simple_stmt< import_name< 'import' 'IECore' > "\\n" >
		|
		starImport=simple_stmt< import_from< 'from' 'IECore' 'import' '*' > "\\n" >
		|
		name = ( {0} )
	""" .format( "|".join( [ "'" + x + "'" for x in dir( IECore ) if not x.startswith( "_" ) ] ) )

	def start_tree( self, tree, filename ) :

		BaseFix.start_tree( self, tree, filename )

		self.__hadStarImport = False
		self.__haveRegularImport = False

	def transform( self, node, results ) :

		if "starImport" in results and node.depth() == 1 :

			if self.__haveRegularImport :

				results["starImport"].remove()

			else :

				regularImport = Node(
					syms.import_name,
					[
						Leaf( token.NAME, u"import", prefix = results["starImport"].prefix ),
						Leaf( token.NAME, "IECore", prefix=" " )
					]
				)
				regularImportLine = Node(
					syms.simple_stmt,
					[
						regularImport,
						Newline()
					]
				)

				node.replace( regularImportLine )

				self.__haveRegularImport = True

			self.__hadStarImport = True

		elif "regularImport" in results and node.depth() == 1:

			if self.__haveRegularImport :
				results["regularImport"].remove()

			self.__haveRegularImport = True

		elif "name" in results :

			if (
				self.__hadStarImport and
				not str( node.parent ).startswith( "." )
			) :
				node.value = "IECore." + node.value
				node.changed()

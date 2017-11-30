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

from lib2to3.fixer_base import BaseFix
from lib2to3.pgen2 import token
from lib2to3.fixer_util import Name, Newline
from lib2to3.pytree import Leaf, Node
from lib2to3.pygram import python_symbols as syms

import IECoreScene

class FixScene( BaseFix ) :

	PATTERN = """
		coreImport=simple_stmt< import_name< 'import' 'IECore' > "\\n" >
		|
		sceneImport=simple_stmt< import_name< 'import' 'IECoreScene' > "\\n" >
		|
		power< module='IECore' trailer<'.' name=any > remainder=any* >
	"""

	def start_tree( self, tree, filename ) :

		self.__coreImport = None
		self.__haveSceneImport = False

	def transform( self, node, results ) :

		if "coreImport" in results and node.depth() == 1 :

			self.__coreImport = results["coreImport"]

		elif "sceneImport" in results and node.depth() == 1 :

			self.__haveSceneImport = True

		elif "name" in results and hasattr( IECoreScene, results["name"].value ) :

			if results["name"].value == "TypeId" :
				attr = None
				if results["remainder"] :
					attr = str( results["remainder"][0] ).lstrip( "." )
				if not attr or not attr[0].isupper() or not hasattr( IECoreScene.TypeId, attr ) :
					# Not an IECoreScene TypeId, so no transformation
					# is needed.
					return

			results["module"].value = "IECoreScene"
			node.changed()

			if not self.__haveSceneImport and self.__coreImport is not None :

				sceneImport = Node(
					syms.import_name,
					[
						Leaf( token.NAME, u"import" ),
						Leaf( token.NAME, "IECoreScene", prefix=" " )
					]
				)
				sceneImportLine = Node(
					syms.simple_stmt,
					[
						sceneImport,
						Newline()
					]
				)

				self.__coreImport.parent.insert_child(
					self.__coreImport.parent.children.index( self.__coreImport ) + 1,
					sceneImportLine
				)

				self.__haveSceneImport = True

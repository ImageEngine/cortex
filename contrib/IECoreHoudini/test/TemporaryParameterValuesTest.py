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

from __future__ import with_statement

import hou

import IECore
import IECoreHoudini

class TemporaryParameterValuesTest( IECoreHoudini.TestCase ) :

	def test( self ) :

		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		node = geo.createNode( "attribcreate" )
		node.parm( "class" ).set( 1 )
		node.parm( "writevalues" ).set( True )
		node.parm( "size" ).set( 4 )
		node.parm( "default1" ).set( 10.5 )
		node.parm( "string" ).set( "hi" )
		node.parmTuple( "value" ).set( ( 1.25, 2.5, 3.75, 4.99 ) )
		nodePath = node.path()
		
		context = IECoreHoudini.TemporaryParameterValues(
			{
				nodePath + "/class" : 2,
				nodePath + "/writevalues" : False,
				nodePath + "/size" : 2,
				nodePath + "/default1" : 20.5,
				nodePath + "/string" : "bye",
				nodePath + "/value" : ( 9.25, 6.5, 54.12, 5636.4 ),
			}
		)

		with context :

			self.assertEqual( hou.parm( nodePath + "/class" ).eval(), 2 )
			self.assertEqual( hou.parm( nodePath + "/writevalues" ).eval(), False )
			self.assertEqual( hou.parm( nodePath + "/size" ).eval(), 2 )
			self.assertEqual( hou.parm( nodePath + "/default1" ).eval(), 20.5 )
			self.assertEqual( hou.parm( nodePath + "/string" ).eval(), "bye" )
			value = hou.parmTuple( nodePath + "/value" ).eval()
			tempValue = [ 9.25, 6.5, 54.12, 5636.4 ]
			for i in range( 0, 4 ) :
				self.assertAlmostEqual( value[i], tempValue[i], 3 )
		
		self.assertEqual( hou.parm( nodePath + "/class" ).eval(), 1 )
		self.assertEqual( hou.parm( nodePath + "/writevalues" ).eval(), True )
		self.assertEqual( hou.parm( nodePath + "/size" ).eval(), 4 )
		self.assertEqual( hou.parm( nodePath + "/default1" ).eval(), 10.5 )
		self.assertEqual( hou.parm( nodePath + "/string" ).eval(), "hi" )
		value = hou.parmTuple( nodePath + "/value" ).eval()
		realValue = [ 1.25, 2.5, 3.75, 4.99 ]
		for i in range( 0, 4 ) :
			self.assertAlmostEqual( value[i], realValue[i], 3 )

if __name__ == "__main__":
	IECoreHoudini.TestProgram()

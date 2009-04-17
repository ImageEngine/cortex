##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

import maya.cmds as cmds
import maya.OpenMaya as OpenMaya
import unittest, MayaUnitTest
import os.path
import IECore
import IECoreMaya

class TestParameterisedHolder( unittest.TestCase ) :
		
	def testNode( self ):
		""" Test ParameterisedHolderNode """
		n = cmds.createNode( "ieParameterisedHolderNode" )	
		h = IECoreMaya.FnParameterisedHolder( str(n) )
		self.assert_( h )
		
		p = IECore.ParticleMeshOp()
		
		h.setParameterised( p )
		
		p.parameters().filename = "testValue"		
		h.setNodeValue( p.parameters()["filename"], False )
		pl = h.parameterPlug( p.parameters()["filename"] )
		v = IECoreMaya.FromMayaPlugConverter.create( pl, IECore.TypeId.StringData ).convert()
		self.assertEqual( v.value, "testValue" )
				
		cmds.setAttr( pl.name(), "testValue2", typ="string" )
		h.setParameterisedValue( p.parameters()["filename"] )
		self.assertEqual( p.parameters()["filename"].getValue().value, "testValue2" )
					
	def testParameterisedHolderSetReference( self ):	
		""" Test multiple references to ieParameterisedHolderSet nodes """
								
		nodeType = "ieParameterisedHolderSet" 	
				
		nodeName = cmds.createNode( nodeType )
		
		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "reference.ma" ) )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )
		
		cmds.file( new = True, force = True )				
		cmds.file( scene, reference = True, namespace = "ns1" )
		cmds.file( scene, reference = True, namespace = "ns2" )
		
		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "referenceMaster.ma" ) )
		masterScene = cmds.file( force = True, type = "mayaAscii", save = True )
				
		cmds.file( masterScene, force = True, open = True )
		
		nodeName1 = "ns1:" + nodeName	
		nodeName2 = "ns2:" + nodeName	
		
		l = OpenMaya.MSelectionList()
		l.add( nodeName1 )
		l.add( nodeName2 )
				
		node1 = OpenMaya.MObject()
		l.getDependNode( 0, node1 )
		node2 = OpenMaya.MObject()
		l.getDependNode( 1, node2 )
		
		fn1 = OpenMaya.MFnDependencyNode( node1 )
		fn2 = OpenMaya.MFnDependencyNode( node2 )
		
		self.assert_( fn1.userNode() )
		self.assert_( fn2.userNode() ) # This failure is due to a Maya bug. When referencing the same scene twice, as an optimisation Maya will duplicate existing nodes instead of creating new ones. There is a bug in MPxObjectSet::copy() which gets exercised here. Setting the environment variable MAYA_FORCE_REF_READ to 1 will disable this optimisation, however.
	
	def testSetParameterisedFromClassLoader( self ) :
	
		fnPH = IECoreMaya.FnProceduralHolder.create( "test", "read", 1 )
		proc = fnPH.getProcedural()
		self.assert_( isinstance( proc, IECore.ParameterisedProcedural ) )
	
	def testAllParameters( self ) :
	
		node = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( node )
		
		op = IECore.ClassLoader.defaultOpLoader().load( "parameterTypes", 1 )()
		op.parameters().removeParameter( "m" ) # we're not even attempting to represent Color4fParameters at present
		
		fnOH.setParameterised( op )
	
	def testStringVectorParameter( self ) :
	
		p = IECore.Parameterised( "", "" )
		p.parameters().addParameter( 
			IECore.StringVectorParameter(
				"sv",
				"",
				IECore.StringVectorData( [ "hello", "goodbye" ] )
			)
		)
		self.assertEqual( p["sv"].getValue(), IECore.StringVectorData( [ "hello", "goodbye" ] ) )
		
		node = cmds.createNode( "ieParameterisedHolderLocator" )
		fnOH = IECoreMaya.FnParameterisedHolder( node )
		fnOH.setParameterised( p )
		
		self.assertEqual( cmds.getAttr( node + ".parm_sv" ), [ "hello", "goodbye" ] )
		
		fnOH.setParameterisedValues()

		self.assertEqual( p["sv"].getValue(), IECore.StringVectorData( [ "hello", "goodbye" ] ) )
		
	def tearDown( self ) :
		
		for f in [ "test/IECoreMaya/reference.ma" , "test/IECoreMaya/referenceMaster.ma" ] :
		
			if os.path.exists( f ) :
		
				os.remove( f )
			
		
		

		

if __name__ == "__main__":
	MayaUnitTest.TestProgram()

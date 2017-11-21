##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

import os
import glob
import unittest
import threading

import IECore
from IECoreRI import *

class TestSLOReader( unittest.TestCase ) :

	def shaderPath( self ) :

		return os.environ["SHADER_PATH"]

	def assertExpectedParams( self, s, params ) :

		for p in params :
			self.assertEqual( s.parameters[p[0]].typeName(), p[1] )
			if p[1]=="FloatData" :
				self.assertAlmostEqual( s.parameters[p[0]].value, p[2] )
			else :
				self.assertEqual( s.parameters[p[0]].value, p[2] )
			if len( p )>3 :
				self.assertEqual( p[3], s.blindData()["ri:parameterTypeHints"][p[0]].value )

	def testPlastic( self ) :

		r = SLOReader( os.path.join( self.shaderPath(), "plastic.sdl" ) )
		s = r.read()
		self.assertEqual( s.name, "plastic" )
		self.assertEqual( s.type, "surface" )
		self.assertEqual( len( s.parameters ), 5 )

		self.assertExpectedParams ( s,
			[
				["Ks", "FloatData", 0.5],
				["Kd", "FloatData", 0.5],
				["Ka", "FloatData", 1],
				["roughness", "FloatData", 0.1],
				["specularcolor", "Color3fData", IECore.Color3f( 1 ) ],
			]
		)

	def testSpotLight( self ) :

		r = SLOReader( os.path.join( self.shaderPath(), "spotlight.sdl" ) )
		s = r.read()
		self.assertEqual( s.name, "spotlight" )
		self.assertEqual( s.type, "light" )
		if "__category" in s.parameters :
			self.assertEqual( len( s.parameters ), 13 )
		else :
			self.assertEqual( len( s.parameters ), 12 )

		self.assertExpectedParams ( s,
			[
				["intensity", "FloatData", 1],
				["lightcolor", "Color3fData", IECore.Color3f( 1 ) ],
				["shadowmap", "StringData", ""],
				["blur", "FloatData", 0],
				["from", "V3fData", IECore.V3f( 0 ), "point" ],
				["to", "V3fData", IECore.V3f( 0, 0, 1 ), "point" ],
			]
		)

	def testTypes( self ) :

		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/types.sdl test/IECoreRI/shaders/types.sl" ), 0 )

		r = SLOReader( "test/IECoreRI/shaders/types.sdl" )
		s = r.read()

		expectedParams = [
			["f", IECore.FloatData( 1 )],
			["s", IECore.StringData( "s")],
			["c", IECore.Color3fData( IECore.Color3f( 1, 2, 3 ) )],
			["p", IECore.V3fData( IECore.V3f( 0, 1, 2 ), IECore.GeometricData.Interpretation.Point )],
			["v", IECore.V3fData( IECore.V3f( -1, 0, 1 ), IECore.GeometricData.Interpretation.Vector )],
			["n", IECore.V3fData( IECore.V3f( -2, -1, 0 ), IECore.GeometricData.Interpretation.Normal )],
			["m", IECore.M44fData( IECore.M44f( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 ) ) ],
			["fa", IECore.FloatVectorData( [ 1, 2] ) ],
			["sa", IECore.StringVectorData( [ "one", "two" ] ) ],
			["ca", IECore.Color3fVectorData( [ IECore.Color3f( 1 ), IECore.Color3f( 2 ) ] ) ],
			["pa", IECore.V3fVectorData( [ IECore.V3f( 2 ), IECore.V3f( 3 ) ], IECore.GeometricData.Interpretation.Point ) ],
			["va", IECore.V3fVectorData( [ IECore.V3f( 2 ), IECore.V3f( 3 ) ], IECore.GeometricData.Interpretation.Vector ) ],
			["na", IECore.V3fVectorData( [ IECore.V3f( 2 ), IECore.V3f( 3 ) ], IECore.GeometricData.Interpretation.Normal ) ],
			["ma", IECore.M44fVectorData( [ IECore.M44f(), IECore.M44f() ] ) ],
			["fav", IECore.FloatVectorData() ],
			["sav", IECore.StringVectorData() ],
			["cav", IECore.Color3fVectorData() ],
			["pav", IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Point ) ],
			["vav", IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Vector ) ],
			["nav", IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Normal ) ],
			["mav", IECore.M44fVectorData() ],
			["f3", IECore.FloatVectorData( [ 1, 2, 3 ] ) ],
			["sh", IECore.StringData( "" ) ],
			["sha", IECore.StringVectorData() ],
			["sha3", IECore.StringVectorData( [ "", "", "" ] ) ],
		]

		self.assertEqual( len( expectedParams ), len( s.parameters ) )

		for p in expectedParams :

			self.assertEqual( s.parameters[p[0]], p[1] )

	def testThreading( self ) :

		shaders = [
			"spotlight.sdl",
			"plastic.sdl",
			"matte.sdl",
			"distantlight.sdl",
		]

		def read( shader ) :

			SLOReader( shader ).read()

		for i in range( 0, 100 ) :

			threads = []
			for shader in shaders :
				t = threading.Thread( target = IECore.curry( read, os.path.join( self.shaderPath(), shader ) ) )
				threads.append( t )

			for t in threads :
				t.start()

			for t in threads :
				t.join()

	def testCantReadEmpty( self ) :

		self.assertRaises( RuntimeError, IECore.Reader.create, 'test/IECore/data/empty' )
		self.assertRaises( RuntimeError, IECore.Reader.create, 'test/IECore/data/null' )
		self.assertRaises( RuntimeError, IECore.Reader.create, 'test/IECore/data/null.cin' )

	def testBlindData( self ) :

		r = SLOReader( os.path.join( self.shaderPath(), "matte.sdl" ) )
		s = r.read()

		self.assertEqual( s.blindData()["ri:orderedParameterNames"], IECore.StringVectorData( [ "Ka", "Kd" ] ) )

	def testAnnotations( self ) :

		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/types.sdl test/IECoreRI/shaders/types.sl" ), 0 )

		r = SLOReader( "test/IECoreRI/shaders/types.sdl" )
		s = r.read()

		self.assertTrue( "ri:annotations" in s.blindData() )

		self.assertEqual( s.blindData()["ri:annotations"]["author"], IECore.StringData( "JohnJohn" ) )
		self.assertEqual( s.blindData()["ri:annotations"]["version"], IECore.StringData( "1.0" ) )

	def tearDown( self ) :

		for f in [ "test/IECoreRI/shaders/types.sdl" ] :
			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()

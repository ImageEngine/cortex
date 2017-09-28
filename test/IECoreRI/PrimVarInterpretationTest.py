##########################################################################
#
#  Copyright (c) 2008-2015, Image Engine Design Inc. All rights reserved.
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

import unittest
import IECore
import IECoreRI
import os.path
import os

class PrimVarInterpretationTest( IECoreRI.TestCase ) :

	def test( self ) :

		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m["testPoint"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 0 ), IECore.GeometricData.Interpretation.Point ) )
		m["testNormal"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 0 ), IECore.GeometricData.Interpretation.Normal ) )
		m["testVector"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 0 ), IECore.GeometricData.Interpretation.Vector ) )
		m["testColor"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 0 ), IECore.GeometricData.Interpretation.Color ) )
		m["testNumeric"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 0 ), IECore.GeometricData.Interpretation.Numeric ) )

		m["testPointArray"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 1 ) ], IECore.GeometricData.Interpretation.Point ) )
		m["testNormalArray"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 1 ) ], IECore.GeometricData.Interpretation.Normal ) )
		m["testVectorArray"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 1 ) ], IECore.GeometricData.Interpretation.Vector ) )
		m["testColorArray"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 1 ) ], IECore.GeometricData.Interpretation.Color ) )
		m["testNumericArray"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 1 ) ], IECore.GeometricData.Interpretation.Numeric ) )

		r = IECoreRI.Renderer( "test/IECoreRI/output/testPrimVarInterpretation.rib" )
		r.worldBegin()

		m.render( r )

		r.worldEnd()

		rib = "".join( open( "test/IECoreRI/output/testPrimVarInterpretation.rib" ).readlines() )

		self.assert_( '"constant point testPoint" [ 0 0 0 ]' in rib )
		self.assert_( '"constant normal testNormal" [ 0 0 0 ]' in rib )
		self.assert_( '"constant vector testVector" [ 0 0 0 ]' in rib )
		self.assert_( '"constant color testColor" [ 0 0 0 ]' in rib )
		self.assert_( '"constant float[3] testNumeric" [ 0 0 0 ]' in rib )

		self.assert_( '"constant point testPointArray[2]" [ 0 0 0 1 1 1 ]' in rib )
		self.assert_( '"constant normal testNormalArray[2]" [ 0 0 0 1 1 1 ]' in rib )
		self.assert_( '"constant vector testVectorArray[2]" [ 0 0 0 1 1 1 ]' in rib )
		self.assert_( '"constant color testColorArray[2]" [ 0 0 0 1 1 1 ]' in rib )

		# Note that this triggers a special case PrimitiveVariableList::PrimitiveVariableList, since we can't have a type of float[3][2]
		self.assert_( '"constant vector testNumericArray[2]" [ 0 0 0 1 1 1 ]' in rib )

if __name__ == "__main__":
    unittest.main()

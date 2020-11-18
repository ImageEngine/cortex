##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

import maya.cmds

import IECore
import IECoreScene
import IECoreMaya

class FromMayaParticleConverterTest( IECoreMaya.TestCase ) :

	def testSimple( self ) :

		particle = maya.cmds.particle( n = 'particles' )[0]
		particle = maya.cmds.listRelatives( particle, shapes = True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( str( particle ), IECoreScene.PointsPrimitive.staticTypeId() )

		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaParticleConverter ) ) )

		particle = converter.convert()

		self.assertTrue( particle.isInstanceOf( IECoreScene.PointsPrimitive.staticTypeId() ) )
		self.assertTrue( particle.arePrimitiveVariablesValid() )
		self.assertEqual( particle.numPoints, maya.cmds.particle( 'particles', q = True, count = True ) )

		self.assertTrue( "P" in particle )
		self.assertTrue( particle["P"].data.isInstanceOf( IECore.TypeId.V3fVectorData ) )
		self.assertEqual( particle["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertTrue( "velocity" in particle )
		self.assertTrue( particle["velocity"].data.isInstanceOf( IECore.TypeId.V3fVectorData ) )
		self.assertEqual( particle["velocity"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertTrue( "mass" in particle )

	def testEmitter( self ) :

		maya.cmds.emitter( speed = 2.00, rate = 1000, n = 'emitter' )
		particle = maya.cmds.particle( n = 'particles' )[0]
		particle = maya.cmds.listRelatives( particle, shapes = True )[0]
		maya.cmds.connectDynamic( 'particles', em = 'emitter' )

		maya.cmds.addAttr( particle, ln="ieParticleAttributes", dt="string")
		# ensure we can split on any of the following: ' ', ':', ','
		maya.cmds.setAttr( "{}.ieParticleAttributes".format(particle), "radiusPP=width,userScalar1PP:userScalar2PP userScalar3PP", type="string" )

		maya.cmds.addAttr( particle, ln="radiusPP", dt="doubleArray")
		maya.cmds.addAttr( particle, ln="radiusPP0", dt="doubleArray")

		maya.cmds.addAttr( particle, ln="userScalar1PP", dt="doubleArray")
		maya.cmds.addAttr( particle, ln="userScalar1PP0", dt="doubleArray")

		maya.cmds.addAttr( particle, ln="userScalar2PP", dt="doubleArray")
		maya.cmds.addAttr( particle, ln="userScalar2PP0", dt="doubleArray")

		maya.cmds.addAttr( particle, ln="userScalar3PP", dt="doubleArray")
		maya.cmds.addAttr( particle, ln="userScalar3PP0", dt="doubleArray")

		maya.cmds.addAttr( particle, ln="userScalar4PP", dt="doubleArray")
		maya.cmds.addAttr( particle, ln="userScalar4PP0", dt="doubleArray")

		for i in range( 0, 25 ) :
			maya.cmds.currentTime( i )

		converter = IECoreMaya.FromMayaShapeConverter.create( str( particle ), IECoreScene.PointsPrimitive.staticTypeId() )

		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaParticleConverter ) ) )

		particleConvert = converter.convert()

		self.assertTrue( particleConvert.isInstanceOf( IECoreScene.PointsPrimitive.staticTypeId() ) )
		self.assertTrue( particleConvert.arePrimitiveVariablesValid() )
		self.assertEqual( particleConvert.numPoints, maya.cmds.particle( 'particles', q = True, count = True ) )
		self.assertTrue( particleConvert.numPoints > 900 )
		self.assertTrue( particleConvert.numPoints < 1100 )
		self.assertTrue( "P" in particleConvert )
		self.assertTrue( particleConvert["P"].data.isInstanceOf( IECore.TypeId.V3fVectorData ) )
		self.assertEqual( particleConvert["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertTrue( "velocity" in particleConvert )
		self.assertTrue( particleConvert["velocity"].data.isInstanceOf( IECore.TypeId.V3fVectorData ) )
		self.assertEqual( particleConvert["velocity"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertTrue( "mass" in particleConvert )
		self.assertTrue( particleConvert["mass"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )
		self.assertTrue( "width" in particleConvert )
		self.assertTrue( particleConvert["width"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )
		self.assertEqual( list( particleConvert["width"].data ), [ 2.0 * rad for rad in maya.cmds.getAttr( "{}.radiusPP".format( particle ) ) ] )
		self.assertTrue( "userScalar1PP" in particleConvert )
		self.assertTrue( particleConvert["userScalar1PP"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )
		self.assertTrue( "userScalar2PP" in particleConvert )
		self.assertTrue( particleConvert["userScalar2PP"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )
		self.assertTrue( "userScalar3PP" in particleConvert )
		self.assertTrue( particleConvert["userScalar3PP"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )

		# userScalar4PP is defined on the particles not specified in ieParticleAttributes and therefore shouldn't be converted
		self.assertTrue( "userScalar4PP" not in particleConvert )


	def testErrors( self ) :

		particle = maya.cmds.particle( n = 'particles' )[0]
		particle = maya.cmds.listRelatives( particle, shapes = True )[0]

		self.assertFalse( IECoreMaya.FromMayaShapeConverter.create( str( particle ), IECoreScene.MeshPrimitive.staticTypeId() ) )

	def testNParticle( self ):
		import imath

		points = [ ( 0, 0, 0 ), ( 3, 5, 6 ), ( 5, 6, 7) , ( 9, 9, 9 ) ]
		transform, nParticle = maya.cmds.nParticle( p=points )

		maya.cmds.addAttr( nParticle, ln="ieParticleAttributes", dt="string" )

		# ensure we can split on any of the following: ' ', ':', ','
		maya.cmds.setAttr( "{}.ieParticleAttributes".format(nParticle), "radiusPP=width,userScalar1PP:userScalar2PP userScalar3PP rgbPP=Cs rotationPP=orientation rotationPP=euler", type="string" )

		maya.cmds.addAttr( nParticle, ln="radiusPP", dt="doubleArray" )
		maya.cmds.addAttr( nParticle, ln="radiusPP0", dt="doubleArray" )

		maya.cmds.addAttr(nParticle, ln="rotationPP", dt="vectorArray")
		maya.cmds.addAttr(nParticle, ln="rotationPP0", dt="vectorArray")

		maya.cmds.addAttr( nParticle, ln="userScalar1PP", dt="doubleArray" )
		maya.cmds.addAttr( nParticle, ln="userScalar1PP0", dt="doubleArray" )

		maya.cmds.addAttr( nParticle, ln="userScalar2PP", dt="doubleArray" )
		maya.cmds.addAttr( nParticle, ln="userScalar2PP0", dt="doubleArray" )

		maya.cmds.addAttr( nParticle, ln="userScalar3PP", dt="doubleArray" )
		maya.cmds.addAttr( nParticle, ln="userScalar3PP0", dt="doubleArray" )

		maya.cmds.addAttr( nParticle, ln="userScalar4PP", dt="doubleArray" )
		maya.cmds.addAttr( nParticle, ln="userScalar4PP0", dt="doubleArray" )

		converter = IECoreMaya.FromMayaShapeConverter.create( str( nParticle ), IECoreScene.PointsPrimitive.staticTypeId() )

		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaParticleConverter ) ) )

		nParticleConvert = converter.convert()

		self.assertTrue( nParticleConvert.isInstanceOf( IECoreScene.PointsPrimitive.staticTypeId() ) )
		self.assertTrue( nParticleConvert.arePrimitiveVariablesValid() )
		self.assertEqual( nParticleConvert.numPoints, maya.cmds.particle( nParticle, q = True, count = True ) )
		self.assertEqual( nParticleConvert.numPoints, 4 )
		self.assertTrue( "P" in nParticleConvert )
		self.assertTrue( nParticleConvert["P"].data.isInstanceOf( IECore.TypeId.V3fVectorData ) )
		self.assertEqual( nParticleConvert["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( list( nParticleConvert["P"].data ), [ imath.V3f( *vec ) for vec in points ] )
		self.assertTrue( "velocity" in nParticleConvert )
		self.assertTrue( nParticleConvert["velocity"].data.isInstanceOf( IECore.TypeId.V3fVectorData ) )
		self.assertEqual( nParticleConvert["velocity"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertTrue( "mass" in nParticleConvert )
		self.assertTrue( nParticleConvert["mass"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )
		self.assertTrue( "userScalar1PP" in nParticleConvert )
		self.assertTrue( nParticleConvert["userScalar1PP"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )
		self.assertTrue( "userScalar2PP" in nParticleConvert )
		self.assertTrue( nParticleConvert["userScalar2PP"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )
		self.assertTrue( "userScalar3PP" in nParticleConvert )
		self.assertTrue( nParticleConvert["userScalar3PP"].data.isInstanceOf( IECore.TypeId.FloatVectorData ) )

		# check radius to width conversion
		self.assertTrue( "width" in nParticleConvert )
		self.assertEqual( list( nParticleConvert["width"].data ), [ 2.0 * rad for rad in maya.cmds.getAttr( "{}.radiusPP".format( nParticle ) ) ] )
		self.assertEqual( nParticleConvert["width"].data.typeId(), IECore.TypeId.FloatVectorData )

		# check rotation
		self.assertTrue( "orientation" in nParticleConvert )
		self.assertTrue( "euler" in nParticleConvert )
		self.assertEqual( nParticleConvert["orientation"].data.typeId(), IECore.TypeId.QuatfVectorData )
		self.assertEqual( nParticleConvert["euler"].data.typeId(), IECore.TypeId.V3fVectorData )

		# check constant color
		maya.cmds.setAttr( "{}.colorInput".format( nParticle ), 0 )
		maya.cmds.setAttr( "{}.color[0].color_Color".format( nParticle ), 0.1, 0.1, 0.1, type="double3" )
		nParticleConvert = converter.convert()
		self.assertTrue( "Cs" in nParticleConvert )
		self.assertEqual( nParticleConvert["Cs"].data.typeId(), IECore.TypeId.Color3fData )
		self.assertEqual( nParticleConvert["Cs"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( nParticleConvert["Cs"].data, IECore.Color3fData( imath.Color3f( 0.1 ) ) )

		# check per-particle color
		maya.cmds.setAttr( "{}.colorInput".format( nParticle ), 1 )
		maya.cmds.setAttr( "{}.color[0].color_Color".format( nParticle ), 0.3, 0.3, 0.3, type="double3" )
		nParticleConvert = converter.convert()
		self.assertTrue( "Cs" in nParticleConvert )
		self.assertTrue( nParticleConvert["Cs"].data.typeId(), IECore.TypeId.Color3fVectorData )
		self.assertEqual( nParticleConvert["Cs"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( nParticleConvert["Cs"].data[0], imath.Color3f( 0.3 )  )

		# userScalar4PP is defined on the nParticle not specified in ieParticleAttributes and therefore shouldn't be converted
		self.assertTrue( "userScalar4PP" not in nParticleConvert )

if __name__ == "__main__":
	IECoreMaya.TestProgram()

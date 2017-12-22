##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

"""Unit test for SmoothSkinningData binding"""
import imath

import IECore
import IECoreScene

import os
import unittest

class TestSmoothSkinningData( unittest.TestCase ) :

    def testData( self ) :
        # test the object
        s = IECoreScene.SmoothSkinningData()
        self.assertEqual( s.influenceNames(), IECore.StringVectorData() )
        self.assertEqual( s.influencePose(), IECore.M44fVectorData() )
        self.assertEqual( s.pointIndexOffsets(), IECore.IntVectorData() )
        self.assertEqual( s.pointInfluenceCounts(), IECore.IntVectorData() )
        self.assertEqual( s.pointInfluenceIndices(), IECore.IntVectorData() )
        self.assertEqual( s.pointInfluenceWeights(), IECore.FloatVectorData() )
        self.assertEqual( s, s )
        self.assertEqual( s, s.copy() )
        self.assertEqual( s, IECoreScene.SmoothSkinningData() )
        self.assertRaises( Exception, s.validate() )

    def testIO( self ) :
        # test fileIndexedIO, read and write
        ok_jn = IECore.StringVectorData( [ 'jointA', 'jointB' ] )
        ok_ip = IECore.M44fVectorData( [imath.M44f(),imath.M44f()] )
        ok_pio = IECore.IntVectorData( [0, 2, 4] )
        ok_pic = IECore.IntVectorData( [2, 2, 1] )
        ok_pii = IECore.IntVectorData( [0, 1, 0, 1, 1] )
        ok_piw = IECore.FloatVectorData( [0.5, 0.5, 0.2, 0.8, 1.0] )

        s = IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, ok_pic, ok_pii, ok_piw)
        iface = IECore.IndexedIO.create( "test/IECore/ssd.fio", IECore.IndexedIO.OpenMode.Write )

        s.save( iface, "test" )
        ss = IECore.Object.load( iface, "test" )
        self.assertEqual( s, ss )

    def testDataStorage(self ):
        #test the object can store data
        ok_jn = IECore.StringVectorData( [ 'jointA', 'jointB' ] )
        ok_ip = IECore.M44fVectorData( [imath.M44f(),imath.M44f()] )
        ok_pio = IECore.IntVectorData( [0, 2, 4] )
        ok_pic = IECore.IntVectorData( [2, 2, 1] )
        ok_pii = IECore.IntVectorData( [0, 1, 0, 1, 1] )
        ok_piw = IECore.FloatVectorData( [0.5, 0.5, 0.2, 0.8, 1.0] )
        s = IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, ok_pic, ok_pii, ok_piw)

        self.assertEqual( s.influenceNames() , ok_jn )
        self.assertEqual( s.influencePose() , ok_ip )
        self.assertEqual( s.pointIndexOffsets() , ok_pio )
        self.assertEqual( s.pointInfluenceCounts() , ok_pic )
        self.assertEqual( s.pointInfluenceIndices() , ok_pii )
        self.assertEqual( s.pointInfluenceWeights() , ok_piw  )

        self.assertEqual( s, s )
        self.assertEqual( s, s.copy() )
        self.assertRaises( Exception, s.validate() )

        iface = IECore.IndexedIO.create( "test/IECore/ssd.fio", IECore.IndexedIO.OpenMode.Write )
        s.save( iface, "test" )
        ss = IECore.Object.load( iface, "test" )
        self.assertEqual( s, ss )

    def testValidate(self):

        # good data
        ok_jn = IECore.StringVectorData( [ 'jointA', 'jointB' ] )
        ok_ip = IECore.M44fVectorData( [imath.M44f(),imath.M44f()] )
        ok_pio = IECore.IntVectorData( [0, 2, 4] )
        ok_pic = IECore.IntVectorData( [2, 2, 1] )
        ok_pii = IECore.IntVectorData( [0, 1, 0, 1, 1] )
        ok_piw = IECore.FloatVectorData( [0.5, 0.5, 0.2, 0.8, 1.0] )

        # data with invalid nr of elements
        iv_jn = IECore.StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
        iv_ip = IECore.M44fVectorData( [imath.M44f()] )
        iv_pio1 = IECore.IntVectorData( [0, 2, 4, 666] )
        iv_pic1 = IECore.IntVectorData( [2, 2 ] )
        iv_pii1 = IECore.IntVectorData( [0, 1, 0, 1, 1, 666] )
        iv_piw = IECore.FloatVectorData( [0.5, 0.5, 0.2] )

        # data with invalid ids
        iv_pio2 = IECore.IntVectorData( [0, 2, 666] )
        iv_pii2 = IECore.IntVectorData( [0, 1, 666, 1, 1] )

        # data with invalid counts
        iv_pic2 = IECore.IntVectorData( [2, 0, 1 ] )

        # data with count / index mismatch
        iv_pio3 = IECore.IntVectorData( [0, 3, 4] )
        iv_pic3 = IECore.IntVectorData( [3, 1, 1] )

        # test all is ok
        IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, ok_pic, ok_pii, ok_piw).validate()

        # test wrong nr of influenceNames, influencePose
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(iv_jn, ok_ip, ok_pio, ok_pic, ok_pii, ok_piw).validate )
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, iv_ip, ok_pio, ok_pic, ok_pii, ok_piw).validate )

        # test wrong nr of pointIndexOffsets, pointInfluenceCounts
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, iv_pio1, ok_pic, ok_pii, ok_piw).validate )
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, iv_pic1, ok_pii, ok_piw).validate )

        # test wrong nr of pointInfluenceIndices, pointInfluenceWeights
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, ok_pic, iv_pii1, ok_piw).validate )
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, ok_pic, ok_pii, iv_piw).validate )

        # test invalid ids
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, iv_pio2, ok_pic, ok_pii, ok_piw).validate )
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, ok_pic, iv_pii2, ok_piw).validate )

        # test wrong counts
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, iv_pic2, ok_pii, ok_piw).validate )

        # test count id mismatching
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, iv_pio3, ok_pic, ok_pii, ok_piw).validate )
        self.assertRaises( Exception, IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, iv_pic3, ok_pii, ok_piw).validate )

# todo: add reference test data we are happy with
#    def testRef(self):
#        load reference data we are sure is cool
#        ss = Reader.create( "test/IECore/data/cobFiles/smoothSkinningData.cob" ).read()
#        self.assert_( ss.isValid() );

    def tearDown( self ) :

        if os.path.isfile("test/IECore/ssd.fio"):
            os.remove("test/IECore/ssd.fio")

if __name__ == "__main__":
    unittest.main()

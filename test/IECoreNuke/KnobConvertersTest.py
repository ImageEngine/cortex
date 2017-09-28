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

import math
import unittest
import IECoreNuke
import IECore
import nuke

class KnobConvertersTest( IECoreNuke.TestCase ) :

	@staticmethod
	def __parameters() :

		return IECore.CompoundParameter(
					name = "",	description = "",
					members = [
						IECore.IntParameter(
							name = "A",	description = "",
							defaultValue = 1
						),
						IECore.CompoundParameter(
							name = "B",	description = "",
							members = [

								IECore.DoubleParameter(
									name = "C",	description = "",
									defaultValue = 2
								),

								IECore.FloatParameter(
									name = "D",	description = "",
									defaultValue = 3
								),

							]
						),

						IECore.StringParameter(
							name = "E",	description = "",
							defaultValue = "4"
						),

						IECore.CompoundParameter(
							name = "F",	description = "",
							members = [

								IECore.FloatVectorParameter(
									name = "G",	description = "",
									defaultValue = IECore.FloatVectorData( [ 5,6,7 ] )
								),

								IECore.IntVectorParameter(
									name = "H",	description = "",
									defaultValue = IECore.IntVectorData( [ 8 ] )
								),

								IECore.DoubleVectorParameter(
									name = "I",	description = "",
									defaultValue = IECore.DoubleVectorData()
								),

							]
						),

						IECore.StringVectorParameter(
							name = "J", description = "",
							defaultValue = IECore.StringVectorData( ["10", "11", "12"] )
						),

						IECore.FileNameParameter(
							name = "K", description = "",
							defaultValue = "/tmp/test.dpx",
						),

						IECore.DirNameParameter(
							name = "L", description = "",
							defaultValue = "/tmp"
						),

						IECore.FileSequenceParameter(
							name = "M", description = "",
							defaultValue = "/tmp/test.####.dpx",
						),

						IECore.BoolParameter(
							name = "N", description = "",
							defaultValue = True
						),

						IECore.TransformationMatrixfParameter(
							name = "O", description = "",
							defaultValue = IECore.TransformationMatrixfData( IECore.TransformationMatrixf( IECore.V3f(1), IECore.Eulerf(), IECore.V3f(1) ) ),
							presets = (
								( "preset0", IECore.TransformationMatrixfData(IECore.TransformationMatrixf(IECore.V3f(1), IECore.Eulerf(), IECore.V3f(1))) ),
								( "preset1", IECore.TransformationMatrixfData(IECore.TransformationMatrixf(IECore.V3f(1), IECore.Eulerf(), IECore.V3f(2))) ),
								( "preset2", IECore.TransformationMatrixfData(IECore.TransformationMatrixf(IECore.V3f(1), IECore.Eulerf(), IECore.V3f(3))) ),
								( "preset3", IECore.TransformationMatrixfData(IECore.TransformationMatrixf(IECore.V3f(1), IECore.Eulerf(), IECore.V3f(4))) ),
							),
							presetsOnly = True
						)

					]
				)

	# this class simulates a knob holder, such as nukescripts.PythonPanel or a nuke.Node
	class KnobHolder :

		def __init__( self ) :
			self.__knobs = {}

		def knobs( self ) :
			return self.__knobs

		def addKnob( self, knob ):
			self.__knobs[ knob.name() ] = knob

	def testKnobCreation( self ) :
		knobHolder = KnobConvertersTest.KnobHolder()
		IECoreNuke.createKnobsFromParameter( knobHolder, KnobConvertersTest.__parameters() )
		knobs = knobHolder.knobs()
		self.assertEqual( knobs["parm_A"].getValue(), 1 )
		self.assertEqual( knobs["parm_B_C"].getValue(), 2.0 )
		self.assertEqual( knobs["parm_B_D"].getValue(), 3.0 )
		self.assertEqual( knobs["parm_E"].getText(), "4" )
		self.assertEqual( knobs["parm_F_G"].getText(), "5.0 6.0 7.0" )
		self.assertEqual( knobs["parm_F_H"].getText(), "8" )
		self.assertEqual( knobs["parm_F_I"].getText(), "" )
		self.assertEqual( knobs["parm_J"].getText(), "10\n11\n12" )
		self.assertEqual( knobs["parm_K"].getText(), "/tmp/test.dpx" )
		self.assertEqual( knobs["parm_L"].getText(), "/tmp" )
		self.assertEqual( knobs["parm_M"].getText(), "/tmp/test.%04d.dpx" )
		self.assertEqual( knobs["parm_N"].getValue(), True )
		self.assertEqual( knobs["parm_O"].value(), "preset0" )

	def testSettingKnobValues( self ) :

		params = KnobConvertersTest.__parameters()
		knobHolder = KnobConvertersTest.KnobHolder()
		IECoreNuke.createKnobsFromParameter( knobHolder, params )
		# modify parameters
		params[ 'A' ] = 2
		params[ 'F' ][ 'G' ] = IECore.FloatVectorData()
		params[ 'F' ][ 'I' ] = IECore.DoubleVectorData( [ 9 ] )
		params[ 'M' ] = "/tmp/anotherTest.%d.dpx"
		params[ 'N' ] = False
		params[ 'O' ] = IECore.TransformationMatrixfData(IECore.TransformationMatrixf(IECore.V3f(1), IECore.Eulerf(), IECore.V3f(3)))
		IECoreNuke.setKnobsFromParameter( knobHolder, params )
		knobs = knobHolder.knobs()
		self.assertEqual( knobs["parm_A"].getValue(), 2 )
		self.assertEqual( knobs["parm_F_G"].getText(), "" )
		self.assertEqual( knobs["parm_F_I"].getText(), "9.0" )
		self.assertEqual( knobs["parm_M"].getText(), "/tmp/anotherTest.%d.dpx" )
		self.assertEqual( knobs["parm_N"].getValue(), False )
		self.assertEqual( knobs["parm_O"].value(), "preset2" )

	def testGettingKnobValues( self ) :

		params = KnobConvertersTest.__parameters()
		knobHolder = KnobConvertersTest.KnobHolder()
		IECoreNuke.createKnobsFromParameter( knobHolder, params )
		knobs = knobHolder.knobs()
		IECoreNuke.setParameterFromKnobs( knobHolder, params )
		self.assertEqual( params["A"].getNumericValue(), 1 )
		self.assertEqual( params["B"]["C"].getNumericValue(), 2.0 )
		self.assertEqual( params["B"]["D"].getNumericValue(), 3.0 )

		knobs["parm_A"].setValue( 10 )
		knobs["parm_B_C"].setValue( 20.0 )
		knobs["parm_B_D"].setValue( 30.0 )
		knobs["parm_E"].setValue( "40" )
		knobs["parm_F_G"].setValue( "50 60 70" )
		knobs["parm_F_H"].setValue( "80" )
		knobs["parm_F_I"].setValue( "90" )
		knobs["parm_J"].setValue( "100\n110\n120" )
		knobs["parm_K"].setValue( "/tmp2/test.dpx" )
		knobs["parm_L"].setValue( "/tmp2" )
		knobs["parm_M"].setValue( "/tmp2/test.%02d.dpx" )
		knobs["parm_N"].setValue( False )
		knobs["parm_O"].setValue( "preset3" )
		IECoreNuke.setParameterFromKnobs( knobHolder, params )

		self.assertEqual( params["A"].getNumericValue(), 10 )
		self.assertEqual( params["B"]["C"].getNumericValue(), 20.0 )
		self.assertEqual( params["B"]["D"].getNumericValue(), 30.0 )
		self.assertEqual( params["E"].getTypedValue(), "40" )
		self.assertEqual( params["F"]["G"].getValue(), IECore.FloatVectorData( [ 50,60,70 ] ) )
		self.assertEqual( params["F"]["H"].getValue(), IECore.IntVectorData( [ 80 ] ) )
		self.assertEqual( params["F"]["I"].getValue(), IECore.DoubleVectorData( [ 90 ] ) )
		self.assertEqual( params["J"].getValue(), IECore.StringVectorData( [ "100", "110", "120" ] ) )
		self.assertEqual( params["K"].getTypedValue(), "/tmp2/test.dpx" )
		self.assertEqual( params["L"].getTypedValue(), "/tmp2" )
		self.assertEqual( params["M"].getTypedValue(), "/tmp2/test.##.dpx" )
		self.assertEqual( params["N"].getTypedValue(), False )
		self.assertEqual( params["O"].getValue(), IECore.TransformationMatrixfData(IECore.TransformationMatrixf(IECore.V3f(1), IECore.Eulerf(), IECore.V3f(4))) )
		# raises exception when trying to convert an invalid file sequence syntax
		knobs["parm_M"].setValue( "/tmp2/test.%2d.dpx" )
		self.assertRaises( RuntimeError, IECoreNuke.setParameterFromKnobs, knobHolder, params )


if __name__ == "__main__":
    unittest.main()


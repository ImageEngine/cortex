##########################################################################
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

import hou
import IECore
import IECoreHoudini
import unittest
import os
import glob

class TestCortexRmanInject( unittest.TestCase ):

	# test we can create and assign an rman inject SHOP
	# note the procedural has an expression-driven parameter to
	# check it's being evaluated correctly
	def test_createRmanInject(self):
		procedural = IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 )
		procedural.parm("parm_radius").setExpression("$F")
		assert( procedural )
		rmaninject = hou.node("/shop").createNode("cortexRmanInject")
		assert( rmaninject )
		rmaninject.parm("procedural").set(procedural.path())
		procedural.parent().parm("shop_materialpath").set(rmaninject.path())
		assert( procedural.parent().evalParm("shop_materialpath") == rmaninject.path() )
		assert( hou.node(procedural.parent().evalParm("shop_materialpath")) )
		assert( hou.node(procedural.parent().evalParm("shop_materialpath")).evalParm("procedural") == procedural.path() )

	# test we can render our rmaninject correctly and that the parameters
	# serialise as expected
	def test_renderRmanInject(self):
		if not os.path.exists( "test/test_data" ):
			os.mkdir( "test/test_data" )
		rib_file = "test/test_data/testrman.$F4.rib"
		hou.hipFile.clear(suppress_save_prompt=True)
		self.test_createRmanInject()
		# create a camera
		camera = hou.node("/obj").createNode( "cam", node_name="cam1" )
		assert( camera )
		# create a mantra rop
		rman = hou.node("/out").createNode( "rib", node_name="rman_out" )
		assert( rman )
		rman.parm("camera").set(camera.path())
		# set path
		rman.parm("rib_outputmode").set(True)
		rman.parm("soho_diskfile").set(rib_file)
		# render
		try:
			rman.render( frame_range=(1,10) )
		except:
			assert(False)
		# check ribs made it
		ribs = glob.glob("test/test_data/testrman.????.rib")
		assert( len(ribs)==10 )
		# make sure the procedurals got in there
		procs = []
		for rib in ribs:
			for line in open(rib):
				if 'Procedural "DynamicLoad"' in line:
					procs.append(line.strip())
 		assert( len(procs)==10 )
 		assert( "iePython" in procs[0] )
 		assert( "['-radius', '1', '-theta', '360']" in procs[0] )
 		assert( "['-radius', '2', '-theta', '360']" in procs[1] )
 		assert( "['-radius', '3', '-theta', '360']" in procs[2] )
 		assert( "[-1 1 -1 1 -1 1]" in procs[0])


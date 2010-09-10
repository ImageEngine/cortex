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
import shutil

class TestProceduralHolder( unittest.TestCase ):

	def testProceduralHolder(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder" )
		assert( proc )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		assert( fn )
		return fn

	def testLoadProcedural(self):
		fn = self.testProceduralHolder()
		cl = IECore.ClassLoader.defaultProceduralLoader().load( "sphereProcedural", 0 )()
		fn.setParameterised( cl )
		assert( fn.getParameterised()!=None )
		assert( fn.getParameterised()==cl )
		return fn

	def testProceduralParameters(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder" )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		cl = IECore.ClassLoader.defaultProceduralLoader().load( "parameterTypes", 1 )()
		fn.setParameterised( cl )

		# set a lot of parameters via houdini
		proc.parmTuple("parm_a").set( [123] )
		proc.parmTuple("parm_d").set( ["hello"] )
		proc.parmTuple("parm_g").set( (2,4) )
		proc.parmTuple("parm_h").set( (1,4,8) )
		proc.parmTuple("parm_i").set( (2,4) )
		proc.parmTuple("parm_i_3").set( (1, 2, 3, 4,
										5, 6, 7, 8,
										9, 10, 11, 12,
										13, 14, 15, 16 ) )
		proc.parmTuple("parm_i_4").set( (1, 2, 3, 4,
										5, 6, 7, 8,
										9, 10, 11, 12,
										13, 14, 15, 16 ) )
		proc.parmTuple("parm_compound_parm_j").set( (1,4,8) )
		proc.parmTuple("parm_compound_parm_k").set( (1, 2, 3, 4,
										5, 6, 7, 8,
										9, 10, 11, 12,
										13, 14, 15, 16 ) )
		proc.parmTuple("parm_l").set( (1,0,0) )
		proc.parmTuple("parm_m").set( (1,1,0,1) )
		proc.parmTuple("parm_o").set( ["myFile.tif"] )
		proc.parmTuple("parm_p").set( [os.getcwd()] )
		proc.parmTuple("parm_q").set( [True] )
		proc.parmTuple("parm_r").set( ["mySequence.####.tif"] )
		proc.parmTuple("parm_s").set( [-1, -2, 10, 20] )
		proc.parmTuple("parm_s_1").set( [-1, -2, 10, 20] )
		proc.parmTuple("parm_s_2").set( [-1, -2, -3, 10, 20, 30] )
		proc.parmTuple("parm_t").set( [-1, -2, -3, 10, 20, 30] )
		proc.parmTuple("parm_u").set( (64, 128) )
		proc.parmTuple("parm_v").set( (25,26,27) )

		# flush our parameters through to our parameterised procedural
		proc.cook(force=True)

		# generate our bounds
		box = cl.bound()
		assert( box==IECore.Box3f( IECore.V3f(0,0,0), IECore.V3f(1,1,1) ) )
		return ( proc, cl )

	def testLotsQuickly(self):
		n = []
		for i in range(1000):
			n.append( IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 ) )
		for _n in n:
			_n.destroy()

	def testSaveAndLoad(self):
		save_file = "test/proceduralHolder_testData/proceduralSave_test.hip"

		# create a few procedurals
		n = []
		for i in range( 10 ):
			n.append( IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 ) )
		for i in range( 10 ):
			n.append( IECoreHoudini.FnProceduralHolder.create( "cortex_params", "parameterTypes", 1 ) )

		# set some values
		path1 = n[0].path()
		n[0].parm("parm_radius").set(10)
		n[0].parm("parm_theta").set(90)
		path2 = n[9].path()
		n[9].parm("parm_radius").set(5)
		n[9].parm("parm_theta").set(45)

		# save scene
		hou.hipFile.save(save_file)

		# new scene
		hou.hipFile.clear(suppress_save_prompt=True)

		# open scene
		hou.hipFile.load(save_file)

		# check parameters
		proc = hou.node(path1)
		assert( proc )
		assert( proc.evalParm("__opType")=="sphereProcedural" )
		assert( proc.evalParm("__opVersion")=="1" )
		assert( proc.evalParm("parm_radius")==10 )
		assert( proc.evalParm("parm_theta")==90 )
		proc = hou.node(path2)
		assert( proc )
		assert( proc.evalParm("__opType")=="sphereProcedural" )
		assert( proc.evalParm("__opVersion")=="1" )
		assert( proc.evalParm("parm_radius")==5 )
		assert( proc.evalParm("parm_theta")==45 )

	def testObjectWasDeleted(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder" )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		cl = IECore.ClassLoader.defaultProceduralLoader().load( "sphereProcedural", 1 )()
		proc.destroy()
		assert( fn.hasParameterised() == False )
		fn.setParameterised(cl)

	def testProceduralReloadParameters(self):
		sphere = IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 )

		# check the reload button doesn't clear expressions
		sphere.parm("parm_radius").setExpression("sin($FF)")
		hou.setFrame(0)
		rad = sphere.evalParm("parm_radius")
		assert( rad>0 )
		hou.setFrame(100)
		rad = sphere.evalParm("parm_radius")
		assert( rad>0.984 )
		assert( rad<0.985 )
		sphere.parm("__opReloadBtn").pressButton()
		rad = sphere.evalParm("parm_radius")
		assert( rad>0.984 )
		assert( rad<0.985 )
		assert( sphere.parm("parm_radius").expression()=="sin($FF)" )
		hou.setFrame(0)
		rad = sphere.evalParm("parm_radius")
		assert( rad>0 )

		# now change the version to v2 and check things are still ok
		sphere.parm("__opVersion").set("2")
		# if we're changing the menu programatically then we need to call pressButton()!!
		sphere.parm("__opVersion").pressButton()
		assert( not sphere.evalParm("parm_extra") )
		sphere.parm("parm_extra").set(True)
		assert( sphere.evalParm("parm_extra") )
		rad = sphere.evalParm("parm_radius")
		assert( rad<0.015 )
		hou.setFrame(100)
		rad = sphere.evalParm("parm_radius")
		assert( rad>0.984 )
		assert( rad<0.985 )

	def testHiddenParameters( self ):
		( proc, cl ) = self.testProceduralParameters()
		# check the hidden userData works
		assert( proc.parmTuple("parm_a").parmTemplate().isHidden()==True )
		assert( proc.parmTuple("parm_b").parmTemplate().isHidden()==False )
		# check setting the parameter still works
		proc.parmTuple("parm_a").set( [123] )
		proc.cook(force=True)
		assert( cl['a'].getValue().value == 123 )

	def setUp( self ) :
                os.environ["IECORE_PROCEDURAL_PATHS"] = "test/procedurals"
                if not os.path.exists( "test/proceduralHolder_testData" ):
			os.mkdir( "test/proceduralHolder_testData" )

	def tearDown( self ) :
                if os.path.exists( "test/proceduralHolder_testData" ):
			shutil.rmtree( "test/proceduralHolder_testData" )

if __name__ == "__main__":
    unittest.main()

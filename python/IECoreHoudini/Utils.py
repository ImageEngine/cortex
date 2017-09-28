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

import hou, IECore, IECoreHoudini

'''
This contains utility methods for doing useful stuff in the IECoreHoudini
python module.
'''

# returns an instance of a procedural loaded using the defaultProceduralLoader
def proc(type, ver):
	return IECore.ClassLoader.defaultProceduralLoader().load(type,ver)()

# returns an instance of an op loaded using the defaultOpLoader
def op(type, ver):
	return IECore.ClassLoader.defaultOpLoader().load(type,ver)()

# sets a houdini parameter based on the value from it's corresponding cortex parameter
def setHoudiniParm( node, p ):
	value = p.getValue().value

	if p.typeId()==IECore.TypeId.IntParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )
	if p.typeId()==IECore.TypeId.V2iParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.V3iParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

	# float, V2f, V3f
	if p.typeId()==IECore.TypeId.FloatParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )
	if p.typeId()==IECore.TypeId.V2fParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.V3fParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

	# double
	if p.typeId()==IECore.TypeId.DoubleParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )
	if p.typeId()==IECore.TypeId.V2dParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.V3dParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

	# bool
	if p.typeId()==IECore.TypeId.BoolParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )

	# string
	if p.typeId()==IECore.TypeId.StringParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )

	# path, dirname, filename, filesequence
	if p.typeId()==IECore.TypeId.PathParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )

	if p.typeId()==IECore.TypeId.DirNameParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )

	if p.typeId()==IECore.TypeId.FileNameParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )

	if p.typeId()==IECore.TypeId.FileSequenceParameter:
		node.parmTuple( "parm_%s" % p.name ).set( [value] )

	# color3f
	if p.typeId()==IECore.TypeId.Color3fParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

	# color4f
	if p.typeId()==IECore.TypeId.Color4fParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

	# M44f, M44d
	if p.typeId()==IECore.TypeId.M44fParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.M44dParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

	# Box2i, Box2f, Box2d
	if p.typeId()==IECore.TypeId.Box2iParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.Box2fParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.Box2dParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

	# Box3i, Box3f, Box3d
	if p.typeId()==IECore.TypeId.Box3iParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.Box3fParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )
	if p.typeId()==IECore.TypeId.Box3dParameter:
		node.parmTuple( "parm_%s" % p.name ).set( list(value) )

# updates all the houdini parameters based on an Op/Procedural's parameter values
def syncSopParametersWithProcedural(n):
	fn = IECoreHoudini.FnProceduralHolder( n )
	parms = fn.getParameterised().parameters().values()
	for p in parms:
		if n.parm("parm_%s"%p.name):
			setHoudiniParm( n, p )

def syncSopParametersWithOp(n):
	fn = IECoreHoudini.FnOpHolder( n )
	parms = fn.getParameterised().parameters().values()
	for p in parms:
		if n.parm("parm_%s"%p.name):
			setHoudiniParm( n, p )

# reloads a procedural based on the values of the type/version parameters
# \todo: this can be combined with the reloadOp code
def reloadProcedural():
	n = hou.node(".")
	type = n.evalParm("__opType")
	ver = n.evalParm("__opVersion")
	if type=="" or ver=="":
		return
	ver = int(ver)
	fn = IECoreHoudini.FnProceduralHolder(n)
	IECore.ClassLoader.defaultProceduralLoader().refresh()
	cl = IECoreHoudini.proc( type, ver )

	# cache our existing parameters
	parms = fn.getParameterised().parameters().values()
	saved_parms = {}
	for p in parms:
		saved_parms[p.name] = p.getValue().value

	# reload parameter interface
	fn.setParameterised(cl)

	# restore parameter values
	for p in saved_parms.keys():
		hparm = n.parm("parm_%s" % p)
		if hparm:
			hparm.set( saved_parms[p] )

# reloads an op based on the values of the type/version parameters
# \todo: this can be combined with the reloadProc code
def reloadOp():
	n = hou.node(".")
	type = n.evalParm("__opType")
	ver = n.evalParm("__opVersion")
	if type=="" or ver=="":
		return
	ver = int(ver)
	fn = IECoreHoudini.FnOpHolder(n)
	IECore.ClassLoader.defaultOpLoader().refresh()
	cl = IECoreHoudini.op( type, ver )

	# cache our existing parameters
	parms = fn.getParameterised().parameters().values()
	saved_parms = {}
	for p in parms:
		saved_parms[p.name] = p.getValue().value

	# reload parameter interface
	fn.setParameterised(cl)

	# restore parameter values
	for p in saved_parms.keys():
		hparm = n.parm("parm_%s" % p)
		if hparm:
			hparm.set( saved_parms[p] )



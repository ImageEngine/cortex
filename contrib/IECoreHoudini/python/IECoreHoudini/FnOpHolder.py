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
from IECoreHoudini.FnParameterisedHolder import FnParameterisedHolder

class FnOpHolder(FnParameterisedHolder):

	# create our function set and stash which node we're looking at
	def __init__(self, node=None):
		FnParameterisedHolder.__init__(self, node)

	@classmethod
	def create(cls, name, type, version, path="IECORE_OP_PATHS"):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", node_name=name, run_init_scripts=False)
		proc = geo.createNode( "ieOpHolder", node_name=name )
		fn = IECoreHoudini.FnOpHolder( proc )
		cl = IECore.ClassLoader.defaultLoader( path ).load( type, version )
		fn.setParameterised( cl() )
		return proc

	# do we have a valid parameterised instance?
	def hasParameterised(self):
		if not self.nodeValid():
			return False
		return IECoreHoudini._IECoreHoudini._FnProceduralHolder(self.node()).hasParameterised()

	# this sets a procedural on our node and then updates the parameters
	def setParameterised(self, procedural, refresh_gui=True ):
		if not self.nodeValid():
			return
		fn = IECoreHoudini._IECoreHoudini._FnOpHolder(self.node())

		# get our procedural type/version which is added by ClassLoader
		type = procedural.typeName()
		version = 0
		if hasattr(procedural, "version"):
			version = procedural.version

		# update the procedural on our SOP & refresh the gui
		fn.setParameterised( procedural, type, version )

		# refresh our parameters
		if refresh_gui:
			self.updateParameters( procedural )
			
	# this returns the procedural our node is working with
	def getParameterised(self):
		if self.nodeValid():
			if IECoreHoudini._IECoreHoudini._FnOpHolder(self.node()).hasParameterised():
				return IECoreHoudini._IECoreHoudini._FnOpHolder(self.node()).getParameterised()
		return None

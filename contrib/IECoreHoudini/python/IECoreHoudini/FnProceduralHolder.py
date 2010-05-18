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

class FnProceduralHolder():

	#=====
	# create our function set and stash which node we're looking at
	def __init__(self, node=None):
		self.__node = node

	@classmethod
	def create(cls, name, type, version, path="IECORE_PROCEDURAL_PATHS"):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", node_name=name, run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder", node_name=name )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		cl = IECore.ClassLoader.defaultLoader( path ).load( type, version )
		fn.setParameterised( cl() )
		return proc

	# check this node is still valid
	def nodeValid(self):
		try:
			p = self.__node.path()
			return True
		except hou.ObjectWasDeleted:
			return False

	#=====
	# this method removes all spare parameters from the "Parameters" folder
	def removeParameters(self):
		if not self.nodeValid():
			return
		fn = self.__node.parmTuplesInFolder

		# how many spare parameters?
		spares = 0
		for p in fn(['Parameters']):
			if p.isSpare():
				spares += 1

		# attempt to remove all spares
		for i in range(spares):
			found = False
			while not found:
				for p in fn(['Parameters']):
					if p.isSpare():
						self.__node.removeSpareParmTuple(p)
						found = True
						break

	#=====
	# update the parameters on our node to reflect our Procedural
	def addRemoveParameters(self, procedural):
		if not self.nodeValid():
			return

		# clear spare parameters from "Parameters"
		self.removeParameters()

		# add parameters
		parm_names = []
		parm_update_gui = []
		for p in procedural.parameters().values():
			# create our houdini parameter
			parm = IECoreHoudini.ParmTemplates.createParm( p )
			if parm:
				# add name of parameter that we can call via our update expression
				parm_names.append( parm["houdini_name"] )
				# add the tuple as a spare parameter
				self.__node.addSpareParmTuple(parm["houdini_tuple"], in_folder=['Parameters'], create_missing_folders=True)

		# update the nodes parameter evaluation expression
		# this creates cook dependencies on the parameters
		expr = ""
		for p in parm_names:
			expr += "if parmTuple('%s'):\n\t%s = evalParmTuple('%s')\n" % ( p, p, p )
		expr += "return 1"
		if len(parm_names)==0:
			expr = "1"

		eval_parm = self.__node.parm( "__opParmEval" )
		eval_parm.lock(False)
		eval_parm.setExpression( expr, language=hou.exprLanguage.Python, replace_expression=True )
		eval_parm.lock(True)

	#=====
	# do we have a valid parameterised instance?
	def hasParameterised(self):
		if not self.nodeValid():
			return False
		return IECoreHoudini._IECoreHoudini._FnProceduralHolder(self.__node).hasParameterised()

	#=====
	# this sets a procedural on our node and then updates the parameters
	def setParameterised(self, procedural, refresh_gui=True ):
		if not self.nodeValid():
			return
		fn = IECoreHoudini._IECoreHoudini._FnProceduralHolder(self.__node)

		# get our procedural type/version which is added by ClassLoader
		type = "Unknown"
		version = "Unknown"
		if hasattr(procedural, "path"):
			type = procedural.path
		if hasattr(procedural, "version"):
			version = procedural.version

		# update the procedural on our SOP
		fn.setParameterised( procedural, type, version )

		# refresh our parameters
		if refresh_gui:
			self.addRemoveParameters( procedural )

	#=====
	# this returns the procedural our node is working with
	def getParameterised(self):
		if self.nodeValid():
			if IECoreHoudini._IECoreHoudini._FnProceduralHolder(self.__node).hasParameterised():
				return IECoreHoudini._IECoreHoudini._FnProceduralHolder(self.__node).getParameterised()
		return None
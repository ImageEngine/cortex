##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

import hou
import IECore
import IECoreHoudini

class FnParameterisedHolder():

	# create our function set and stash which node we're looking at
	def __init__(self, node=None):
		self.__node = node

	# check this node is still valid
	def nodeValid(self):
		if not self.__node:
			raise "FnParameterisedHolder does not have a node to operate on."
		try:
			p = self.__node.path()
			return True
		except hou.ObjectWasDeleted:
			return False
		
	# return the node we're currently wrapping
	def node(self):

		return self.__node if self.nodeValid() else None

	# do we have a valid parameterised instance?
	def hasParameterised( self ) :

		return IECoreHoudini._IECoreHoudini._FnParameterisedHolder( self.node() ).hasParameterised() if self.nodeValid() else False

	# this sets a parameterised object on our node and then updates the parameters
	def setParameterised( self, classNameOrParameterised, classVersion=None, envVarName=None, updateGui=True ) :
	
		if not self.nodeValid() :
			return
		
		if isinstance( classNameOrParameterised, str ) :
			if classVersion is None or classVersion < 0 :
				classVersions = IECore.ClassLoader.defaultLoader( envVarName ).versions( classNameOrParameterised )
				classVersion = classVersions[-1] if classVersions else 0 
			IECoreHoudini._IECoreHoudini._FnParameterisedHolder( self.node() ).setParameterised( classNameOrParameterised, classVersion, envVarName )
		else :
			IECoreHoudini._IECoreHoudini._FnParameterisedHolder( self.node() ).setParameterised( classNameOrParameterised )
		
		parameterised = self.getParameterised()

		if updateGui and parameterised :
			self.updateParameters( parameterised )

	# this returns the parameterised object our node is working with
	def getParameterised( self ) :
		
		return IECoreHoudini._IECoreHoudini._FnParameterisedHolder( self.node() ).getParameterised() if self.hasParameterised() else None

	# get our list of class names based on matchString
	def classNames( self ) :
		
		if not self.nodeValid() :
			return []
		
		matchString = self.__node.parm( "__classMatchString" ).eval()
		searchPathEnvVar = self.__node.parm( "__classSearchPathEnvVar" ).eval()
		return IECore.ClassLoader.defaultLoader( searchPathEnvVar ).classNames( matchString )

	# takes a snapshot of the parameter values & expressions on our node so 
	# that if we change the procedural/op we can restore the parameters afterwards.
	def cacheParameters(self):
		cached_parameters = {}
		for p in self.__node.parmTuplesInFolder(['Parameters']):
			if p.isSpare():
				data = {}
				data['value'] = p.eval()
				expressions = []
				for i in range(len(p)):
					try:
						expr = p[i].expression()
						lang = p[i].expressionLanguage()
						expressions.append( ( expr, lang ) )
					except:
						expressions.append( ( None, None ) )
				data['expressions'] = expressions
				cached_parameters[p.name()] = data
		return cached_parameters
	
	# resores parameter values/expressions from those cached by cacheParameters
	def restoreCachedParameters(self, cached):
		for p in self.__node.parmTuplesInFolder(['Parameters']):
			if p.name() in cached:
				cached_data = cached[p.name()]
				p.set( cached_data['value'] )
				for i in range(len(p)):
					if cached_data['expressions'][i][0]:
						expr = cached_data['expressions'][i][0]
						lang = cached_data['expressions'][i][1]
						p[i].setExpression( expr, lang )
						
	# count the number of spare parameters under the "Parameters" tab
	def countSpareParameters(self):
		num_folders = 0
		for p in self.__node.spareParms():
			if 'Parameters' in p.containingFolders():
				num_folders += 1
		return num_folders		

	# this method removes all spare parameters from the "Parameters" folder
	def removeParameters(self):
		if not self.nodeValid():
			return
		
		# remove regular parameters
		num_spares = len(self.__node.parmTuplesInFolder(["Parameters"]))
		for i in range(num_spares):
			found = False
			while not found:
				for p in self.__node.parmTuplesInFolder(["Parameters"]):
					self.__node.removeSpareParmTuple(p)
					found = True
					break
				
		# remove leftovers (probably folders)
		num_leftovers = self.countSpareParameters()
		while( num_leftovers>0 ):
			found = False
			while not found:
				for p in self.__node.spareParms():
					if 'Parameters' in p.containingFolders():
						self.__node.removeSpareParmTuple( p.tuple() )
						found = True
						break
			num_leftovers = self.countSpareParameters()
	
	# add/remove parameters on our node so we correctly reflect our Procedural
	def updateParameters( self, parameterised ) :
		if not self.nodeValid():
			return

		# cache parameters & then remove them
		cached_parameters = self.cacheParameters()
		self.removeParameters()
		if not parameterised:
			return
		
		# get a list of our parm templates by calling createParm on our top-level CompoundParameter
		# and add them as spare parameter
		parms = IECoreHoudini.ParmTemplates.createParm( parameterised.parameters(), top_level=True )
		parm_names = []
		for p in parms:
			parm_names.append( p['name'] )
			self.__node.addSpareParmTuple(p['tuple'], in_folder=p['folder'], create_missing_folders=True)
		
		# restore our cached parameters	
		self.restoreCachedParameters( cached_parameters )

		# update the nodes parameter evaluation expression
		# this creates cook dependencies on the parameters
		expr = ""
		for p in parm_names:
			expr += "if parmTuple('%s'):\n\t%s = evalParmTuple('%s')\n" % ( p, p, p )
		expr += "return 1"
		if len(parm_names)==0:
			expr = "1"
		eval_parm = self.__node.parm( "__evaluateParameters" )
		eval_parm.lock(False)
		eval_parm.setExpression( expr, language=hou.exprLanguage.Python, replace_expression=True )
		eval_parm.lock(True)

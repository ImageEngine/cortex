
# require HOM
import hou

# require IECore
import IECore

# our c++ module components
from _IECoreHoudini import *

# function sets
from FnParameterisedHolder import FnParameterisedHolder


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
			parm = self.__node.addSpareParmTuple( p['tuple'], in_folder=p['folder'], create_missing_folders=True )
			parm.set( p['initialValue'] )
		
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

from FnOpHolder import FnOpHolder


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
		IECoreHoudini.FnOpHolder( proc ).setParameterised( type, version, path )
		return proc
	
	## Convenience method to call setParameterised with the environment variable
	# for the searchpaths set to "IECORE_OP_PATHS".
	def setOp( self, className, classVersion=None, updateGui=True ) :

		self.setParameterised( className, classVersion, "IECORE_OP_PATHS", updateGui )

	def getOp( self ) :
		
		return self.getParameterised()

from FnProceduralHolder import FnProceduralHolder


import hou
import IECore
import IECoreHoudini
from IECoreHoudini.FnParameterisedHolder import FnParameterisedHolder

class FnProceduralHolder(FnParameterisedHolder):

	# create our function set and stash which node we're looking at
	def __init__(self, node=None):
		FnParameterisedHolder.__init__(self, node)

	@classmethod
	def create(cls, name, type, version, path="IECORE_PROCEDURAL_PATHS"):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", node_name=name, run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder", node_name=name )
		IECoreHoudini.FnProceduralHolder( proc ).setParameterised( type, version, path )
		return proc

	## Convenience method to call setParameterised with the environment variable
	# for the searchpaths set to "IECORE_PROCEDURAL_PATHS".
	def setProcedural( self, className, classVersion=None, updateGui=True ) :

		self.setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS", updateGui )

	def getProcedural( self ) :
		
		return self.getParameterised()


# misc utility methods
from TestCase import TestCase


import hou
import unittest

## A class to help implement unit tests for Houdini functionality. It
# implements setUp() to create a new houdini scene to perform the test in.
class TestCase( unittest.TestCase ) :

	## Derived classes may override this, but they should call the
	# base class implementation too.
	def setUp( self ) :
		
		hou.hipFile.clear( True )

from TestProgram import TestProgram


import sys
import unittest

## A test program which initializes Houdini before running the test suite.
class TestProgram( unittest.TestProgram ) :

	def __init__( self, module='__main__', defaultTest=None, argv=None, testRunner=None, testLoader=unittest.defaultTestLoader ) :

		unittest.TestProgram.__init__( self, module, defaultTest, argv, testRunner, testLoader )

	def runTests( self ) :
	
		if not self.testRunner :
			self.testRunner = unittest.TextTestRunner( verbosity = 2 )

		result = self.testRunner.run( self.test )

		sys.exit( int( not result.wasSuccessful() ) )

import ParmTemplates
import Utils

from ActiveTake import ActiveTake


import hou

## A context object intended for use with python's "with" syntax. It ensures
# that all operations in the with block are performed in the given take,
# and that the previous take is restored if it still exists when the block exits.
class ActiveTake :
	
	def __init__( self, take ) :

		self.__take = take
		self.__prevTake = ActiveTake.name()

	def __enter__( self ) :

		if self.__take in ActiveTake.ls() :
			hou.hscript( "takeset %s" % self.__take )

	def __exit__( self, type, value, traceBack ) :
		
		if self.__prevTake in ActiveTake.ls() :
			hou.hscript( "takeset %s" % self.__prevTake )
	
	## \todo: remove this method when the hscript take commands are available in python
	@staticmethod
	def name() :
		
		return hou.hscript( "takeset" )[0].strip()
	
	## \todo: remove this method when the hscript take commands are available in python
	@staticmethod
	def ls() :
		
		return [ x.strip() for x in hou.hscript( "takels" )[0].strip().split( "\n" ) ]

from TemporaryParameterValues import TemporaryParameterValues


import hou

import IECore

## A context manager for controlling houdini parameter values in with statements.
# It sets parameters to requested values on entering the block and resets them to
# their previous values on exiting the block.
class TemporaryParameterValues :

	def __init__( self, parametersAndValues = {}, **kw ) :

		self.__parametersAndValues = parametersAndValues
		self.__parametersAndValues.update( kw )

	def __enter__( self ) :
		
		handlers = {
			"Int" : self.__simpleParmHandler,
			"Float" : self.__simpleParmHandler,
			"String" : self.__simpleParmHandler,
		}

		self.__restoreCommands = []
		for parmName, value in self.__parametersAndValues.items() :
			
			# check we can handle this type
			parm = hou.parm( parmName ) or hou.parmTuple( parmName )
			if not parm :
				raise TypeError( "Parameter \"%s\" does not exist." % parmName )
			
			parmType = parm.parmTemplate().dataType().name()
			handler = handlers.get( parmType, None )
			if not handler :
				raise TypeError( "Parameter \"%s\" has unsupported type \"%s\"." % ( parmName, parmType ) )

			# store a command to restore the parameter value later
			self.__restoreCommands.append( parm.asCode() )

			# and change the parameter value
			handler( parm, value )

	def __exit__( self, type, value, traceBack ) :
		
		for command in self.__restoreCommands :
			exec( command )

	def __simpleParmHandler( self, parm, value ) :
		
		if isinstance( parm, hou.ParmTuple ) and not isinstance( value, tuple ) :
			value = value,
		
		parm.deleteAllKeyframes()
		parm.set( value )


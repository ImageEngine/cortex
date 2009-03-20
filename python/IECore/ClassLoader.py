##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

import os
import imp
import glob
import os.path
from fnmatch import fnmatch
from IECore import Msg, msg, SearchPath

## This class defines methods for creating instances of classes
# defined in python modules on disk. We could just use the standard
# import mechanism for this but this gives us queries over what is
# available and versioning and suchlike, and uses a different set
# of searchpaths to the standard python paths. It's intended for
# loading classes derived from Op, ParameterisedProcedural and similar
# extension classes, and allows us to create small versioned units
# of functionality for use all over the place - the ieCore "do" script
# uses the ClassLoader to find operations it can perform for instance.
class ClassLoader :

	## Creates a ClassLoader which will load
	# classes found on the SearchPath object passed
	# in.
	def __init__( self, searchPaths ) :
	
		self.__searchPaths = searchPaths
		self.__classes = {}
		self.refresh()
		
	## Returns an alphabetically sorted list
	# of all the classes found
	# on the searchpaths. The optional matchString
	# performs glob style matching to narrow down
	# the set of names returned.
	def classNames( self, matchString = "*" ) :
			
		n = [ x for x in self.__classes.keys() if fnmatch( x, matchString ) ]
		n.sort()
		return n
		
	## Returns the available versions of the specified
	# class as a list of ints, with the latest version
	# last. If the class doesn't exist returns an empty
	# list.
	def versions( self, name ) :
	
		if not name in self.__classes :
			return []
			
		return self.__classes[name]["versions"]
	
	## Sets the default	version for the named class.
	# This is the version that is loaded if no version
	# is specified in the load() method.
	def setDefaultVersion( self, name, version ) :
	
		self.__validateVersion( version )
	
		if not name in self.__classes :
			raise RuntimeError( "Class \"%s\" doesn't exist." % name )
		
		c = self.__classes[name]
			
		if not version in c["versions"] :
			raise RuntimeError( "Class \"%s\" has no version %d." % (name, version) )
			
		c["defaultVersion"] = version
	
	## Returns the default version for the named class.
	# This is the version that is loaded if no version
	# is specified in the load() method. If it has not
	# been set explicitly with setDefaultVersion() then
	# it defaults to the highest available version.
	def getDefaultVersion( self, name ) :
	
		if not name in self.__classes :
			raise RuntimeError( "Class \"%s\" doesn't exist." % name )
		
		c = self.__classes[name]
		
		v = c.get( "defaultVersion", c["versions"][-1] )
		if not v in c["versions"] :
			msg( Msg.Level.Warning, "ClassLoader.getDefaultVersion", "Version %d doesn't exist, reverting to version %d." % ( v, c["versions"][-1] ) )
			v = c["versions"][-1]
			c["defaultVersion"] = v
			
		return v
					
	## Loads the specified version of the named class.
	# Version defaults to getDefaultVersion( name ) if
	# not specified. Note that this returns the actual class
	# object itself rather than an instance of that class.
	# It also adds two class attributes named "path" and "version"
	# with the info necessary to reload the Op from ClassLoader.
	def load( self, name, version = None ) :

		if not name in self.__classes :
			raise RuntimeError( "Class \"%s\" doesn't exist." % name )
			
		if not version :
			version = self.getDefaultVersion( name )
					
		if not version in self.versions( name ) :
			raise RuntimeError( "Class \"%s\" has no version %d." % (name, version) )

		c = self.__classes[name]
		if version in c["imports"] :
			return c["imports"][version]
		
		nameTail = os.path.basename( name )
		fileName = os.path.join( name, str(version), nameTail + ".py" )
		fileName = self.__searchPaths.find( fileName )
		if fileName=="" :
			raise IOError( "Unable to find implementation file for class \"%s\" version %d." % (name, version) )
			
		fileForLoad = open( fileName, "r" )
		try :
			module = imp.load_module( "IECoreClassLoader" + name.replace( ".", "_" ) + str( version ), fileForLoad, fileName, ( ".py", "r", imp.PY_SOURCE ) )
		finally :
			fileForLoad.close()
			
		if not getattr( module, nameTail, None ) :
			raise IOError( "File \"%s\" does not define a class named \"%s\"." % ( fileName, nameTail ) )
	
		result = getattr( module, nameTail )

		result.path = name
		result.version = version

		c["imports"][version] = result
		
		return result
		
	## The ClassLoader uses a caching mechanism to speed
	# up frequent reloads of the same class. This method
	# can be used to force an update of the cache to
	# reflect changes on the filesystem.
	def refresh( self ) :
	
		# remember any old defaultVersions
		defaultVersions = {}
		for k, v in self.__classes.items() :
			defaultVersions[k] = v.get( "defaultVersion", None )
	
		# __classes is a dictionary mapping from a class name
		# to information for that class in the following form
		# {
		#		"versions" : [], # a list containing all the available versions for that class
		# 		"defaultVersion" : int, # the default version if it has been set
		#		"imports" : {}, # a dictionary mapping from version numbers to the actual class definition
		#						# this is filled in lazily by load()
		# }
		self.__classes = {}
		for path in self.__searchPaths.paths :
		
			for root, dirs, files in os.walk( path ) :
			
				dirsToPrune = set()
				for d in dirs :
				
					gf = glob.glob( os.path.join( root, d, "*", d + ".py" ) )
					for f in gf :
						
						head, tail = os.path.split( f )
						head, version = os.path.split( head )
						
						if path.endswith( '/' ) :						
							name = head[len(path):]
						else :						
							name = head[len(path) + 1:]
													
						try :
							version = int( version )
						except :
							continue
							
						c = self.__classes.setdefault( name, { "versions" : [], "imports" : {} } )
						
						if not version in c["versions"]:
							c["versions"].append( version )						
						
						dirsToPrune.add( d )
					
				for d in dirsToPrune :
					dirs.remove( d )
					
		# sort versions
		for c in self.__classes.values() :
			c["versions"].sort()
				
		# restore old default versions
		for k, v in defaultVersions.items() :
			if k in self.__classes and not v is None :
				self.setDefaultVersion( k, v )
		
	__defaultLoaders = {}
	## Returns a ClassLoader configured to load from the paths defined by the
	# specified environment variable. The same object is returned each time,
	# allowing one loader to be shared by many callers.
	@classmethod
	def defaultLoader( cls, envVar ) :
	
		loader = cls.__defaultLoaders.get( envVar, None )
		if loader :
			return loader
		
		sp = ""
		if envVar in os.environ :
			sp = os.environ[envVar]
		else :
			msg( Msg.Level.Warning, "ClassLoader.defaultLoader", "Environment variable %s not set." % envVar )
			
		loader = cls( SearchPath( os.path.expandvars( sp ), ":" ) )
		cls.__defaultLoaders[envVar] = loader
		
		return loader
	
	## Returns a ClassLoader configured to load from the
	# paths defined by the IECORE_OP_PATHS environment variable. The
	# same object is returned each time, allowing one loader to be
	# shared by many callers.			
	@classmethod
	def defaultOpLoader( cls ) :
		
		return cls.defaultLoader( "IECORE_OP_PATHS" )
		
	## Returns a ClassLoader configured to load from the
	# paths defined by the IECORE_PROCEDURAL_PATHS environment variable. The
	# same object is returned each time, allowing one loader to be
	# shared by many callers.			
	@classmethod
	def defaultProceduralLoader( cls ) :
		
		return cls.defaultLoader( "IECORE_PROCEDURAL_PATHS" )
	
	# throws an exception if the version is no good
	@staticmethod
	def __validateVersion( version ) :
	
		if not type( version ) is int :
			raise TypeError( "Version must be an integer" )
	

##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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
import glob
import re
import os.path
import threading
from fnmatch import fnmatch

import six
if six.PY3 :
	import importlib.util
else :
	import imp

from IECore import Msg, msg, SearchPath, warning

## This class defines methods for creating instances of classes
# defined in python modules on disk. We could just use the standard
# import mechanism for this but this gives us queries over what is
# available and versioning and suchlike, and uses a different set
# of searchpaths to the standard python paths. It's intended for
# loading classes derived from Op, and similar extension classes,
# and allows us to create small versioned units of functionality
# for use all over the place - the ieCore "do" script
# uses the ClassLoader to find operations it can perform for instance.
# This class will find files with the following template path:
# <any path>/<className>/<className>-<version>.py
# Where [] represents optional field.
# And for performance sake, it will not explore directories which
# contain files that match this:
# <any path>/<className>/<className>*.*
class ClassLoader :

	## Creates a ClassLoader which will load
	# classes found on the SearchPath object passed
	# in.
	def __init__( self, searchPaths ) :

		self.__searchPaths = searchPaths
		self.__defaultVersions = {}
		self.__loadMutex = threading.RLock()
		self.refresh()

	## Returns a copy of the searchpath used to find classes.
	def searchPath( self ) :

		return SearchPath( self.__searchPaths )

	## Returns an alphabetically sorted list
	# of all the classes found
	# on the searchpaths. The optional matchString
	# performs glob style matching to narrow down
	# the set of names returned.
	def classNames( self, matchString = "*" ) :

		self.__findAllClasses()

		### \todo Support re, and allow exclusions, etc...
		n = [ x for x in self.__classes.keys() if fnmatch( x, matchString ) ]
		n.sort()
		return n

	## Returns the available versions of the specified
	# class as a list of ints, with the latest version last.
	# If the class doesn't exist returns an empty list.
	def versions( self, name ) :
		try :
			c = self.__findClass( name )
			return c["versions"]
		except :
			return []

	## Sets the default	version for the named class.
	# This is the version that is loaded if no version
	# is specified in the load() method.
	def setDefaultVersion( self, name, version ) :

		self.__validateVersion( version )

		c = self.__findClass( name )

		if not version in c["versions"] :
			raise RuntimeError( "Class \"%s\" has no version %d." % (name, version) )

		self.__defaultVersions[name] = version

	## Returns the default version for the named class.
	# This is the version that is loaded if no version
	# is specified in the load() method. If it has not
	# been set explicitly with setDefaultVersion() then
	# it defaults to the highest available version.
	def getDefaultVersion( self, name ) :

		c = self.__findClass( name )

		v = self.__defaultVersions.get( name, c["versions"][-1] )
		if not v in c["versions"] :
			msg( Msg.Level.Warning, "ClassLoader.getDefaultVersion", "Version %d doesn't exist, reverting to version %d." % ( v, c["versions"][-1] ) )
			v = c["versions"][-1]
			self.__defaultVersions[name] = v

		return v

	## Loads the specified version of the named class.
	# Version defaults to getDefaultVersion( name ) if
	# not specified. Note that this returns the actual class
	# object itself rather than an instance of that class.
	# It also adds two class attributes named "path" and "version"
	# with the info necessary to reload the Op from ClassLoader.
	def load( self, name, version = None ) :

		with self.__loadMutex:

			c = self.__findClass( name )

			if not version :
				version = self.getDefaultVersion( name )

			if not version in self.versions( name ) :
				raise RuntimeError( "Class \"%s\" has no version %d." % (name, version) )

			if version in c["imports"] :
				return c["imports"][version]

			nameTail = os.path.basename( name )
			fileName = os.path.join( name, nameTail + "-" + str(version) + ".py" )
			fileName = self.__searchPaths.find( fileName )
			if fileName=="" :
				raise IOError( "Unable to find implementation file for class \"%s\" version %d." % (name, version) )

			moduleName = "IECoreClassLoader" + name.replace( ".", "_" ) + str( version )

			if six.PY3 :
				spec = importlib.util.spec_from_file_location( moduleName, fileName )
				module = importlib.util.module_from_spec( spec )
				spec.loader.exec_module( module )
			else :
				with open( fileName, "r" ) as fileForLoad :
					module = imp.load_module( moduleName, fileForLoad, fileName, ( ".py", "r", imp.PY_SOURCE ) )

			if not getattr( module, nameTail, None ) :
				raise IOError( "File \"%s\" does not define a class named \"%s\"." % ( fileName, nameTail ) )

			result = getattr( module, nameTail )

			if getattr( result, 'staticTypeName', None ) == getattr( result.__bases__[0], 'staticTypeName', None ) :
				warning( "Class \"%s\" has the same staticTypeName as its Base Class. Perhaps you should call registerRunTimeTyped." % name )

			result.path = name
			result.version = version

			c["imports"][version] = result

			return result

	## The ClassLoader uses a caching mechanism to speed
	# up frequent reloads of the same class. This method
	# can be used to force an update of the cache to
	# reflect changes on the filesystem.
	def refresh( self ) :

		# __classes is a dictionary mapping from a class name
		# to information for that class in the following form
		# {
		#		"versions" : [], # a list containing all the available versions for that class
		#		"imports" : {}, # a dictionary mapping from version numbers to the actual class definition
		#						# this is filled in lazily by load()
		# }
		# this will be filled in lazily by __findClass and __findAllClasses

		self.__classes = {}
		self.__foundAllClasses = False

	__defaultLoaders = {}
	__defaultLoaderMutex = threading.Lock()
	## Returns a ClassLoader configured to load from the paths defined by the
	# specified environment variable. The same object is returned each time,
	# allowing one loader to be shared by many callers.
	@classmethod
	def defaultLoader( cls, envVar ) :

		with cls.__defaultLoaderMutex:

			loader = cls.__defaultLoaders.get( envVar, None )
			if loader :
				return loader

			sp = ""
			if envVar in os.environ :
				sp = os.environ[envVar]
			else :
				msg( Msg.Level.Warning, "ClassLoader.defaultLoader", "Environment variable %s not set." % envVar )

			loader = cls( SearchPath( os.path.expandvars( sp ) ) )
			cls.__defaultLoaders[envVar] = loader

			return loader

	## Returns a ClassLoader configured to load from the
	# paths defined by the IECORE_OP_PATHS environment variable. The
	# same object is returned each time, allowing one loader to be
	# shared by many callers.
	@classmethod
	def defaultOpLoader( cls ) :

		return cls.defaultLoader( "IECORE_OP_PATHS" )

	def __updateClassFromSearchPath( self, searchPath, name ) :

		pattern = re.compile( r".*-(\d+).py$" )
		pruneDir = False
		nameTail = os.path.split( name )[-1]

		# globbing for any extension rather than .py to avoid exploring shader
		# directories without Python files. Function returns true on those cases.
		gf = glob.glob( os.path.join( searchPath, name, nameTail + "*.*" ) )
		for f in gf :

			pruneDir = True

			m = re.match( pattern, f )
			try :
				version = int( m.group( 1 ) )
			except :
				continue

			c = self.__classes.setdefault( name, { "versions" : [], "imports" : {} } )

			if not version in c["versions"]:
				c["versions"].append( version )
				c["versions"].sort()

		return pruneDir

	def __findClass( self, name ) :

		if not name in self.__classes and not self.__foundAllClasses :
			for path in self.__searchPaths.paths :
				self.__updateClassFromSearchPath( path, name )

		if name in self.__classes :
			return self.__classes[name]
		else :
			raise RuntimeError( "Class \"%s\" doesn't exist." % name )

	def __findAllClasses( self ) :

		if self.__foundAllClasses :
			return

		self.__classes = {}
		for path in self.__searchPaths.paths :

			for root, dirs, files in os.walk( path ) :

				if path.endswith( os.path.sep ) :
					nameBase = root[len(path):]
				else :
					nameBase = root[len(path)+1:]

				dirsToPrune = set()
				for d in dirs :

					if self.__updateClassFromSearchPath( path, os.path.join( nameBase, d ) ) :
						dirsToPrune.add( d )

				for d in dirsToPrune :
					dirs.remove( d )

		self.__foundAllClasses = True

	# throws an exception if the version is no good
	@staticmethod
	def __validateVersion( version ) :

		if not type( version ) is int :
			raise TypeError( "Version must be an integer" )



import re
import fnmatch

import IECore

## This file implements functions which are useful but don't make sense
# as methods of any particular Parameter class.

## Recurses down from rootParameter, searching for classes held by
# ClassParameter and ClassVectorParameter instances. Returns a list
# of dictionaries of the following form :
#
# {
#	"parent" : classParameter | classVectorParameter
#	"parameterPath" : [ "path", "to", "parameter" ],
#	"uiPath" : [ "path", "using", "labels" ],
#	"classInstance" : instance
# }
#
# The classTypeFilter parameter can specify an optional tuple of class types
# which are used to return only a subset of types - this will passed
# to isinstance() calls.
#
# The classNameFilter parameter specified an optional string which will be
# used with fnmatch to filter based on the class path.
def findClasses( rootParameter, classTypeFilter=(IECore.Parameterised,), classNameFilter="*" ) :

	result = []
	__findClassesWalk( rootParameter, [], [], classTypeFilter, re.compile( fnmatch.translate( classNameFilter ) ), result )
	return result

def __findClassesWalk( parameter, parameterPath, uiPath, classTypeFilter, classNameFilter, result ) :

	if isinstance( parameter, IECore.ClassParameter ) :
		cl = parameter.getClass()
		if cl and isinstance( cl, classTypeFilter ) and classNameFilter.match( cl.path ) :
			result.append(
				{
					"parent" : parameter,
					"parameterPath" : parameterPath,
					"uiPath" : uiPath,
					"classInstance" : cl
				}
			)
	elif isinstance( parameter, IECore.ClassVectorParameter ) :
		cls = parameter.getClasses( True )
		for cl in cls :
			if isinstance( cl[0], classTypeFilter ) and classNameFilter.match( cl[0].path )  :
			
				label = cl[1]
				if cl[0].parameters().has_key( "label" ) :
					label = cl[0]["label"].getTypedValue()
			
				result.append(
					{
						"parent" : parameter,
						"parameterPath" : parameterPath + [ cl[1] ],
						"uiPath" : uiPath + [ label ],
						"classInstance" : cl[0]
					}
				)
	
	if isinstance( parameter, IECore.CompoundParameter ) :
		for n, p in parameter.items() :
			newParameterPath = parameterPath[:] + [ n ]
			newUIPath = parameterPath[:] + [ n ]
			__findClassesWalk( p, newParameterPath, newUIPath, classTypeFilter, classNameFilter, result )

## Recurses down from srcParameter and dstParameter simultaneously, syncing the dstParameter tree to 
# srcParameter by making sure the ClassParameters and ClassVectorParameters there hold instances of the same classes
# that are held on the srcParameter side.
def copyClasses( srcParameter, dstParameter ) :

	if isinstance( srcParameter, IECore.ClassParameter ) and isinstance( dstParameter, IECore.ClassParameter ) :
		c = srcParameter.getClass( True )
		dstParameter.setClass( *c[1:] )
		
	if isinstance( srcParameter, IECore.ClassVectorParameter ) and isinstance( dstParameter, IECore.ClassVectorParameter ) :
		c = srcParameter.getClasses( True )
		dstParameter.setClasses( [ cc[1:] for cc in c ] )

	if isinstance( srcParameter, IECore.CompoundParameter ) and isinstance( dstParameter, IECore.CompoundParameter ) :
		for n, p in srcParameter.items() :
			if dstParameter.has_key( n ) :
				copyClasses( p, dstParameter[n] )

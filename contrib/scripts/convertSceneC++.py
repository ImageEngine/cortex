import re
import os
import sys
import glob
import inspect
import argparse

import IECore
import IECoreScene

parser = argparse.ArgumentParser(
	description = inspect.cleandoc(
	"""
	Attempts to modify C++ source files to assist
	in the migration from IECore to IECoreScene :

	 - Replaces #includes
	 - Replaces IECore:: namespace qualifier with IECoreScene::

	This is a rough and (hopefully) ready script that does
	very little validation. It is recommended that you run
	it in a clean source repository and use `git diff` to
	manually verify the changes that have been made.
	""" ),
	formatter_class = argparse.RawTextHelpFormatter
)

parser.add_argument(
	"--cortex-include-dir",
	help = "The directory where the Cortex headers are installed",
	required = True,
)

parser.add_argument(
	"source-directory",
	help = "A directory containing C++ source files. This will be searched recursively.",
	nargs = "?",
	default = "./",
)

args = parser.parse_args()

includeMap = {}
for include in glob.glob( args.cortex_include_dir + "/IECoreScene/*.h" ) :
	include = include.replace( args.cortex_include_dir + "/", "" )
	include = "#include \"" + include + "\""
	oldInclude = include.replace( "IECoreScene", "IECore" )
	includeMap[oldInclude] = include

def convertIncludes( lines ) :

	newLines = []
	sceneIncludes = []
	lastCoreInclude = 0
	for line in lines :
		newLine = line
		for old, new in includeMap.items() :
			newLine = newLine.replace( old, new )
		if newLine != line :
			sceneIncludes.append( newLine )
		else :
			newLines.append( line )
		if line.startswith( "#include \"IECore/" ) :
			lastCoreInclude = len( newLines )

	if sceneIncludes :
		newLines[lastCoreInclude:lastCoreInclude] = sceneIncludes

	return newLines

ieCoreSceneTypes = [ x for x in dir( IECoreScene ) if x[0].isupper() ]
ieCoreSceneTypes += [ x + "Ptr" for x in ieCoreSceneTypes ] + [ "Const" + x + "Ptr" for x in ieCoreSceneTypes ]
ieCoreSceneTypes = sorted( ieCoreSceneTypes, key = len, reverse = True )

replacements = [
	( re.compile( "IECore::" + x + "(?![A-Za-z])" ), "IECoreScene::" + x )
	for x in ieCoreSceneTypes
]

def convertNamespaces( lines ) :

	newLines = []
	for line in lines :
		newLine = line
		if not line.strip().startswith( "/" ) :
			for old, new in replacements :
				newLine = old.sub( new, newLine )
		newLines.append( newLine )

	return newLines

def convertUsingNamespace( lines ) :

	haveScene = False
	for line in lines :
		if "IECoreScene" in line :
			haveScene = True
			break

	if not haveScene :
		return lines

	for i, line in enumerate( lines ) :
		if line == "using namespace IECore;\n" :
			lines.insert( i + 1, "using namespace IECoreScene;\n" )
			break

	return lines

def convertFile( filename ) :

	lines = open( filename ).readlines()
	lines = convertIncludes( lines )
	lines = convertNamespaces( lines )
	lines = convertUsingNamespace( lines )

	with open( filename, "w" ) as f :
		for line in lines :
			f.write( line )

directory = vars( args )["source-directory"]
for root, dirs, files in os.walk( directory ) :
	for file in files :
		if os.path.splitext( file )[1] in ( ".h", ".inl", ".cpp" ) :
			convertFile( os.path.join( root, file ) )

##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
import sys
import inspect
import argparse
import collections

parser = argparse.ArgumentParser(
	description = inspect.cleandoc(
	"""
	Rewrites source files to follow convention regarding include
	order, and to use IECORE_PUSH_DEFAULT_VISIBILITY where
	necessary.

	> Warning : This modifies files in place! It is expected that
	> you will run this on files which are in source control already
	> and verify the results before committing.
	""" ),
	formatter_class = argparse.RawTextHelpFormatter
)

parser.add_argument(
	"files",
	help = inspect.cleandoc(
		"""
		The files to fix. If a directory is passed then it will
		be recursed.
		""",
	),
	nargs = "*",
	default = [ "./" ],
)

args = parser.parse_args()

includeOrder = [
	"boostpython",
	"self",
	"local",
	"GafferArnold",
	"GafferAppleseed",
	"GafferDelight",
	"GafferOSL",
	"GafferVDB",
	"GafferSceneUI",
	"GafferSceneTest",
	"GafferScene",
	"GafferImageUI",
	"GafferImage",
	"GafferCortex",
	"GafferDispatchBindings",
	"GafferDispatch",
	"GafferUIBindings",
	"GafferUI",
	"GafferBindings",
	"GafferTest",
	"Gaffer",
	"IECoreMaya",
	"IECoreHoudini",
	"IECoreNuke",
	"IECoreAppleseed",
	"IECoreArnold",
	"IECoreGL",
	"IECoreAlembic",
	"IECoreScene",
	"IECoreImage",
	"IECorePython",
	"IECore",
	"maya",
	"DDImage",
	"pxr",
	"appleseed",
	"openvdb",
	"Alembic",
	"OSL",
	"OpenImageIO",
	"OpenEXR",
	"OpenColorIO",
	"boost",
	"arnold",
	"tbb",
	"QtOpenGL",
	"QtCore",
	"freetypeBuild",
	"freetype",
	"GL",
	"OpenGL",
	"unknown",
	"stdc++",
	"stdc"
]

requiresDefaultVisibility = set( [
	"boost/intrusive_ptr.hpp",
	"boost/date_time/posix_time/posix_time.hpp",
	"boost/date_time/posix_time/ptime.hpp",
	"boost/test/unit_test.hpp",
	"OpenEXR/half.h",
	"OpenEXR/ImathVec.h",
	"OpenEXR/ImathBox.h",
	"OpenEXR/ImathQuat.h",
	"OpenEXR/ImathColor.h",
	"OpenEXR/ImathMatrix.h",
	"OpenEXR/ImathPlane.h",
	"OpenEXR/ImathQuat.h",
	"OpenEXR/ImathLine.h",
	"OpenEXR/ImathEuler.h",
	"OpenEXR/ImfTimeCode.h",
] )

def info( filename, lineNumber, message ) :

	sys.stderr.write( "INFO : {0} line {1} : {2}\n".format( filename, lineNumber + 1, message ) )

def warning( filename, lineNumber, message ) :

	sys.stderr.write( "WARNING : {0} line {1} : {2}\n".format( filename, lineNumber + 1, message ) )

def category( includeFile, filename ) :

	if os.path.splitext( filename )[1] == ".cpp" :
		name, ext = os.path.splitext( os.path.basename( includeFile ) )
		if ext == ".h" and name == os.path.splitext( os.path.basename( filename ) )[0] :
			return "self"

	if includeFile == "boost/python.hpp" :
		return "boostpython"

	if "/" in includeFile :
		start = includeFile.partition( "/" )[0]
		if start == "sys" :
			return "stdc"
		elif start in includeOrder :
			return start
		elif start in ( "foundation", "renderer" ) :
			return "appleseed"
		else :
			return "unknown"

	if includeFile == "ft2build.h" :
		return "freetypeBuild"
	elif includeFile.startswith( "FT_" ) and includeFile.endswith( "_H" ) :
		return "freetype"

	if not includeFile.islower() :
		return "local"

	if includeFile == "ai.h" or includeFile.startswith( "ai_" ) :
		return "arnold"

	if includeFile.endswith( ".h" ) :
		return "stdc"
	else :
		return "stdc++"

def formattedInclude( includeFile, category ) :

	if includeFile.startswith( "FT_" ) and includeFile.endswith( "_H" ) :
		openBracket = ""
		closeBracket = ""
	else :
		isStd = category in ( "stdc", "stdc++" )
		openBracket = "<" if isStd else '"'
		closeBracket = ">" if isStd else '"'


	return "#include {0}{1}{2}\n".format(
		openBracket,
		includeFile,
		closeBracket
	)

def fixFile( filename ) :

	lines = open( filename ).readlines()

	includes = collections.defaultdict( set )
	defaultVisibility = False
	firstInclude, lastInclude = None, None
	nonBlankLines = set()
	for lineNumber, line in enumerate( lines ) :

		line = line.rstrip()

		if line == "IECORE_PUSH_DEFAULT_VISIBILITY" :

			if defaultVisibility :
				warning( filename, lineNumber, "Bad default visibility nesting detected" )
			defaultVisibility = True

		elif line == "IECORE_POP_DEFAULT_VISIBILITY" :

			if not defaultVisibility :
				warning( filename, lineNumber, "Bad default visibility nesting detected" )
			defaultVisibility = False

		elif line.startswith( "#include" ) :

			includeFile = line.split()[1].strip( '"' ).lstrip( "<" ).rstrip( ">" )

			if includeFile in requiresDefaultVisibility :
				includes["IECore"].add( "IECore/Export.h" )
				if not defaultVisibility :
					info( filename, lineNumber, "Include {0} will have default visibility added".format( includeFile ) )
			else :
				if defaultVisibility :
					warning( filename, lineNumber, "Include {0} will have default visibility removed".format( includeFile ) )

			if line.endswith( '.inl"' ) and os.path.splitext( filename )[1] != ".cpp" :
				continue

			c = category( includeFile, filename )
			if c == "unknown" :
				warning( filename, lineNumber, "Include {0} is uncategorised".format( includeFile ) )

			if c == "self" and len( includes[c] ) :
				warning( filename, lineNumber, "Found multiple self includes - not making any changes" )
				return

			includes[c].add( includeFile )

		elif line != "" :

			nonBlankLines.add( lineNumber )
			continue

		else :

			continue

		if firstInclude is None :
			firstInclude = lastInclude = lineNumber
		else :
			lastInclude = lineNumber

	if not firstInclude :
		return

	for lineNumber in nonBlankLines :
		if lineNumber > firstInclude and lineNumber < lastInclude :
			warning( filename, lineNumber, "Line will be lost".format( includeFile ) )

	formattedIncludes = []
	for c in includeOrder :

		if not includes[c] :
			continue

		defaultVisibilityIncludes = [ x for x in includes[c] if x in requiresDefaultVisibility ]
		if defaultVisibilityIncludes :

			if len( formattedIncludes ) :
				formattedIncludes.append( "\n" )

			formattedIncludes.append( "IECORE_PUSH_DEFAULT_VISIBILITY\n" )
			for include in sorted( defaultVisibilityIncludes ) :
				formattedIncludes.append( formattedInclude( include, c ) )
			formattedIncludes.append( "IECORE_POP_DEFAULT_VISIBILITY\n" )

		regularIncludes = [ x for x in includes[c] if x not in requiresDefaultVisibility ]
		if regularIncludes :

			if len( formattedIncludes ) :
				formattedIncludes.append( "\n" )

			for include in sorted( regularIncludes ) :
				formattedIncludes.append( formattedInclude( include, c ) )

	if (
		lines[firstInclude-1].startswith( "//" ) and
		formattedIncludes[0] != lines[firstInclude]
	) :
		warning( filename, firstInclude-1, "Comment will be reassociated with different include" )

	lines[firstInclude:lastInclude+1] = formattedIncludes

	with open( filename, "w" ) as f :
		for line in lines :
			f.write( line )

for f in vars( args )["files"] :

	if os.path.isdir( f ) :

		for root, dirs, files in os.walk( f ) :
			for ff in files :
				if os.path.splitext( ff )[1] in ( ".h", ".inl", ".cpp" ) :
					fixFile( os.path.join( root, ff ) )

	else :

		fixFile( f )

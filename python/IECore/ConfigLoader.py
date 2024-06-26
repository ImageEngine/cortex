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

import re
import os
import os.path
import sys
import traceback
import IECore

## This function provides an easy means of providing a flexible configuration
# mechanism for any software. It works by executing all .py files found on
# a series of searchpaths. A copy of the `contextDict` is used as the locals
# dictionary for the execution of each config file; this is typically used
# to pass objects which the config files will manipulate.
# \ingroup python
def loadConfig( searchPaths, contextDict = {}, raiseExceptions = False, subdirectory = "" ) :

	if isinstance( searchPaths, str ) :
		searchPaths = IECore.SearchPath( os.environ.get( searchPaths, "" ) )

	paths = searchPaths.paths
	paths.reverse()

	visitedPaths = set()
	for path in paths :

		if path in visitedPaths :
			continue
		else :
			visitedPaths.add( path )

		pyExtTest = re.compile( r"^[^~].*\.py$" )
		for dirPath, dirNames, fileNames in os.walk( os.path.join( path, subdirectory ) ) :
			for fileName in filter( pyExtTest.search, sorted( fileNames ) ) :
				fullFileName = os.path.abspath( os.path.join( dirPath, fileName ) )

				IECore.msg( IECore.Msg.Level.Debug, "IECore.loadConfig", "Loading file \"%s\"" % fullFileName )

				fileContextDict = contextDict.copy()
				fileContextDict["__file__"] = fullFileName

				try :
					with open( fullFileName ) as f :
						exec(
							compile( f.read(), fullFileName, "exec" ),
							fileContextDict, fileContextDict
						)
				except Exception as m :
					if raiseExceptions :
						raise
					else :
						stacktrace = traceback.format_exc()
						IECore.msg( IECore.Msg.Level.Error, "IECore.loadConfig", "Error executing file \"%s\" - \"%s\".\n %s" % ( fullFileName, m, stacktrace ) )

				del fileContextDict["__file__"]

loadConfig( "IECORE_CONFIG_PATHS", { "IECore" : IECore } )

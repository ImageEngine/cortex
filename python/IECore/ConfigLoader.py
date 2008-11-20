##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
import IECore

## This function provides an easy means of providing a flexible configuration
# mechanism for any software. It works by executing all .py files found on
# a series of searchpaths. It is expected that these files will then make appropriate
# calls to objects passed in via the specified localsDict.
# \ingroup python
def loadConfig( searchPaths, localsDict, raiseExceptions = False ) :

	paths = searchPaths.paths
	paths.reverse()
	for path in paths :
		# \todo Perhaps filter out filenames that begin with "~", also? This would
		# exclude certain types of auto-generated backup files.
		pyExtTest = re.compile( "\.py$" )
		for dirPath, dirNames, fileNames in os.walk( path ) :
			for fileName in filter( pyExtTest.search, fileNames ) :
				fullFileName = os.path.join( dirPath, fileName )
				if raiseExceptions:
					execfile( fullFileName, globals(), localsDict )
				else:
					try :
						execfile( fullFileName, globals(), localsDict )
					except Exception, m :
						IECore.debugException("loading config file")
						IECore.msg( IECore.Msg.Level.Error, "IECore.loadConfig", "Error executing file \"%s\" - \"%s\"." % ( fullFileName, m ) )

loadConfig( IECore.SearchPath( os.environ.get( "IECORE_CONFIG_PATHS", "" ), ":" ), { "IECore" : IECore } )

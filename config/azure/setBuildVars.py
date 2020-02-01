#!/usr/bin/env python
##########################################################################
#
#  Copyright (c) 2019, Cinesite VFX Ltd. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
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

import datetime
import os
import sys

# Azure pipeline variables can be populated at run-time by echoing special
# strings to a process output. The following allows vars to be set:
#
#    ##vso[task.setvariable variable=<name>;]<value>
#
# We make use of this mechanism to allow custom logic to define the build name
# as well as determine the correct commit hash depending on the nature of the
# trigger. These variables can be referenced in a pipeline yaml file downstream
# of the step that runs this script.

## Source Commit Hash

commit = os.environ.get( 'BUILD_SOURCEVERSION' )
# Azure merges the branch into its target in PR build, so
# BUILD_SOURCEVERSION isn't correct as it references the ephemeral merge.
if os.environ.get( 'BUILD_REASON', '' ) == 'PullRequest' :
	commit = os.environ.get( 'SYSTEM_PULLREQUEST_SOURCECOMMITID' )

## Source Branch

sourceBranch = os.environ.get( "BUILD_SOURCEBRANCH", "" )

## Build Name

formatVars = {
	"buildTypeSuffix" : "-debug" if os.environ.get( "BUILD_TYPE", "" ) == "DEBUG" else "",
	"platform" : { "darwin": "macos", "win32": "windows" }.get( sys.platform, "linux" ),
	"timestamp" : datetime.datetime.now().strftime( "%Y%m%d%H%M" ),
	"pullRequest" : os.environ.get( "SYSTEM_PULLREQUEST_PULLREQUESTNUMBER", "UNKNOWN" ),
	"shortCommit" : commit[:8]
}

nameFormats = {
	"default" : "cortex-{timestamp}-{shortCommit}-{platform}{buildTypeSuffix}",
	"PullRequest" : "cortex-pr{pullRequest}-{shortCommit}-{platform}{buildTypeSuffix}",
}

trigger = os.environ.get( 'BUILD_REASON', 'Manual' )

buildName = nameFormats.get( trigger, nameFormats['default'] ).format( **formatVars )

## Azure Pipeline Vars

print( "Setting $(Cortex.Build.Name) to %s" % buildName )
print( "##vso[task.setvariable variable=Cortex.Build.Name;]%s" % buildName )

dependencies = "dependencies"
print( "Setting $(Cortex.Dependencies.Dir) to %s" % dependencies )
print( "##vso[task.setvariable variable=Cortex.Dependencies.Dir;]%s" % dependencies )

# To make sure our publish always matches the one we use in the build name
print( "Setting $(Cortex.Source.Commit) to %s" % commit )
print( "##vso[task.setvariable variable=Cortex.Source.Commit;]%s" % commit )


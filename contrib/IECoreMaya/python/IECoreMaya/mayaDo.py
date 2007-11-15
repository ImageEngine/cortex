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

import IECore
import os, sys

"""
Utility function that provides easy access to Ops inside maya using a similar interface as the DO command line.
It tries to find the actions in the following contexts: (1) maya/<MAYA_MAJOR_VERSION>/<OP>, (2) maya/<MAYA_MAJOR_VERSION>/*/<OP>, (3) maya/<OP> and lastly, (4) */<OP> 
Returns True if the Op was executes without exceptions and False for any kind of error.
"""
def mayaDo( opName, opVersion = None, help = False, **opArgs ):

	loader = IECore.ClassLoader.defaultOpLoader()

	try:

		plugins = loader.classNames( "maya/%s/%s" % (os.environ["MAYA_MAJOR_VERSION"], opName) )
		if len( plugins ) == 0:
			plugins = loader.classNames( "maya/%s/*/%s" % (os.environ["MAYA_MAJOR_VERSION"], opName) )
			if len( plugins ) == 0:
				plugins = loader.classNames( "maya/%s" % (opName) )
				if len( plugins ) == 0:
					plugins = loader.classNames( "*/%s" % opName )
					# eliminate actions in maya because they may be for other maya versions.
					plugins = filter( lambda x: not x.startswith( "maya/" ), plugins )
			
		if not len( plugins ) :
			IECore.error( "Action \"%s\" does not exist.\n" % opName )
			return None
		elif len( plugins )>1 :
			IECore.error( "Action name \"%s\" is ambiguous - could be any of the following : \n\t%s" % ( opName, "\n\t".join( plugins ) ) )
			return None

		actionName = plugins[0]

		actionVersions = loader.versions( actionName )
		if opVersion is None :
			opVersion = actionVersions[-1]
		else:
			if not opVersion in actionVersions :
				IECore.error( "Version \"%s\" of action \"%s\" does not exist.\n" % (opVersion, actionName) )
				return None

		opType = loader.load( actionName, opVersion )
		myOp = opType()

	except Exception, e:
		IECore.debugException( "Failed on trying to load ", opName )
		IECore.error( "Error loading", opName, ":", str(e) )
		return None

	if help:
		formatter = IECore.WrappedTextFormatter( sys.stdout )
		formatter.paragraph( "Name    : " + myOp.name + "\n" )
		formatter.paragraph( myOp.description + "\n" )
		formatter.heading( "Parameters" )
		formatter.indent()
		for p in myOp.parameters().values() :
			IECore.formatParameterHelp( p, formatter )
		formatter.unindent()
		return None

	try:
		res = myOp( **opArgs )
	except Exception, e:
		IECore.error( 'Error executing Op', myOp.name, ':', str(e) )
	else:
		try:
			if myOp.userData()['UI']['showResult'].value:
				print res
		except:
			pass

	return res

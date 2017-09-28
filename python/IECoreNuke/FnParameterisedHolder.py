##########################################################################
#
#  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

import nuke

import IECore
from _IECoreNuke import _parameterisedHolderGetParameterisedResult
from _IECoreNuke import _parameterisedHolderSetModifiedParametersInput

class FnParameterisedHolder :

	def __init__( self, node ) :

		if isinstance( node, basestring ) :
			self.__node = nuke.toNode( node )
		else :
			self.__node = node

	def node( self ) :

		return self.__node

	def setParameterised( self, className, classVersion, searchPathEnvVar ) :

		if classVersion is None or classVersion < 0 :
			classVersions = IECore.ClassLoader.defaultLoader( searchPathEnvVar ).versions( className )
			classVersion = classVersions[-1] if classVersions else 0

		d = self.__node.knob( "classSpecifier" ).getValue()
		if d is None :
			d = IECore.CompoundObject()

		d["className"] = IECore.StringData( className )
		d["classVersion"] = IECore.IntData( classVersion )
		d["classSearchPathEnvVar"] = IECore.StringData( searchPathEnvVar )

		self.__node.knob( "classSpecifier" ).setValue( d )

	## Returns a tuple of the form ( parameterised, className, classVersion, searchPathEnvVar ).
	# Note that in Nuke a single node may hold many DD::Image::Ops, each for a different output
	# context. Each of these will store a different Parameterised instance, so there's no such thing
	# as a single instance to be returned. For this reason a brand new instance is returned, representing
	# the current time. This can be manipulated as desired without affecting the instances held on the node.
	# Please note that currently any Parameters which are represented as node inputs rather than as knobs
	# will not have their values set by this function.
	def getParameterised( self ) :

		self.__node.knob( "__getParameterised" ).execute()
		p = _parameterisedHolderGetParameterisedResult()

		d = self.__node.knob( "classSpecifier" ).getValue()
		return (
			p,
			d["className"].value if d else "",
			d["classVersion"].value if d else 0,
			d["classSearchPathEnvVar"].value if d else "",
		)

	## Returns a context manager for use with the with statement. This can be used to scope edits
	# to Parameter values in such a way that they are automatically transferred onto the nuke
	# knobs on exit from the with block.
	def parameterModificationContext( self ) :

		return _ParameterModificationContext( self )

class _ParameterModificationContext :

	def __init__( self, fnPH ) :

		self.__fnPH = fnPH

	def __enter__( self ) :

		self.__parameterised = self.__fnPH.getParameterised()[0]

		nuke.Undo.begin()

		return self.__parameterised

	def __exit__( self, type, value, traceBack ) :

		_parameterisedHolderSetModifiedParametersInput( self.__parameterised )
		self.__fnPH.node().knob( "__modifiedParameters" ).execute()

		nuke.Undo.end()

##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

## \file FormattedParameterHelp.py
# Defines functions for outputting Parameter help.
#
# \ingroup python

import IECore

## This function formats helpful descriptions of parameters, using
# a Formatter object.
def formatParameterHelp( parm, formatter ) :

	fn = __formatters.get( type( parm ), __formatParameter )
	fn( parm, formatter )

def __formatNumericParameter( parm, formatter ) :

	## \todo Could this be put somewhere generally useful?
	def formatFloat( f ) :
	
		f = "%.10f" % f
		f = f.rstrip( "0" )
		f = f.rstrip( "." )
		return f

	__formatParameter( parm, formatter )
	
	formatter.indent()

	if not parm.presetsOnly :
	
		if isinstance( parm, IECore.IntData ) :
			minValue = str( parm.minValue )
			maxValue = str( parm.maxValue )
		else :
			minValue = formatFloat( parm.minValue )
			maxValue = formatFloat( parm.maxValue )
			
		if parm.hasMinValue() and parm.hasMaxValue() :
			formatter.paragraph( "Range : %s - %s" % ( minValue, maxValue ) )
		elif parm.hasMinValue() :
			formatter.paragraph( "Min : %s" % minValue )
		elif parm.hasMaxValue() :
			formatter.paragraph( "Max : %s" % maxValue )
		
		formatter.unindent()

def __formatFileNameParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.indent()
	
	e = parm.extensions
	if len( e ) :
		formatter.paragraph( "Valid extensions : " + " ".join( e ) )
		
	if parm.mustExist :
		formatter.paragraph( "File must exist" )
		
	if parm.mustNotExist :
		formatter.paragraph( "File must not exist" )	
	
	if not parm.allowEmptyString :
		formatter.paragraph( "File must be specified" )
	
	formatter.unindent()
	
def __formatDirNameParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.indent()
		
	if parm.mustExist :
		formatter.paragraph( "Directory must exist" )
		
	if parm.mustNotExist :
		formatter.paragraph( "Directory must not exist" )	
	
	if not parm.allowEmptyString :
		formatter.paragraph( "Directory must be specified" )
	
	formatter.unindent()	
	
def __formatFileSequenceParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.indent()
		
	if parm.mustExist :
		formatter.paragraph( "Sequence must exist" )
		
	if parm.mustNotExist :
		formatter.paragraph( "Sequence must not exist" )	
	
	if not parm.allowEmptyString :
		formatter.paragraph( "Sequence must be specified" )
		
	formatter.paragraph( "Values must be of the form \"something.###.ext\"" )
	
	formatter.unindent()	

def __formatCompoundParameter( parm, formatter ) :

	__formatParameter( parm, formatter )
	formatter.heading( "Members" )
	formatter.indent()
	for p in parm.values() :
		formatParameterHelp( p, formatter )
	formatter.unindent()

def __formatParameter( parm, formatter ) :

	formatter.heading( parm.name + " (" + parm.__class__.__name__.replace( "Parameter", "" ) + ")" )
	formatter.paragraph( parm.description )
	
	formatter.indent()
	
	d = parm.defaultValue
	defaultPresetName = None
	for k, v in parm.presets().items() :
		if d == v :
			defaultPresetName = k
			break
	
	if defaultPresetName :
		formatter.paragraph( "Default : " + defaultPresetName )
	elif isinstance( d, IECore.Data ) and hasattr( d, "value" ) :
		formatter.paragraph( "Default : " + str( d.value ) )
	
	if len( parm.presetNames() ) :
		formatter.paragraph( "Presets : " )
		formatter.indent()
		formatter.paragraph( "\n".join( parm.presetNames() ) )
		formatter.unindent()
		
	formatter.unindent()

__formatters = {
	IECore.CompoundParameter : __formatCompoundParameter,
	IECore.FloatParameter : __formatNumericParameter,
	IECore.DoubleParameter : __formatNumericParameter,
	IECore.IntParameter : __formatNumericParameter,
	IECore.FileNameParameter : __formatFileNameParameter,
	IECore.DirNameParameter : __formatDirNameParameter,
	IECore.FileSequenceParameter : __formatFileSequenceParameter,
}

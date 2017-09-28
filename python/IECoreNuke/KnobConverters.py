##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement
import nuke
import IECore
from StringUtil import nukeFileSequence, ieCoreFileSequence

__parameterKnobConverters = []

## Registers knob converter functions for IECore data types.
# @param paramClass Parameter class type used on the following converters
# @param knobCreator Function that creates a knob. Function parameters: knobHolder, parameter, knobName, knobLabel
# @param toKnobConverter Function that sets the value of knobs for a given parameter. Function parameters: knobHolder, parameter, knobName
# @param fromKnobConverter Function that sets a Parameter value from it's knobs. Function parameters: knobHolder, parameter, knobName
def registerParameterKnobConverters( paramClass, knobCreator, toKnobConverter, fromKnobConverter ) :
	__parameterKnobConverters.append( ( paramClass, knobCreator, toKnobConverter, fromKnobConverter ) )

## Create new knobs on a given knob holder for a given parameter.
# @param knobHolder The knob holder must support only two methods: knobs() and addKnob()
# @param parameter That's the IECore.Parameter object which you want a knob for.
# @param knobName Specifies the knob name for the given parameter object. Child parameters will have knobs named with that same prefix.
def createKnobsFromParameter( knobHolder, parameter, knobName = "parm" ) :

	if knobName:
		knobLabel = IECore.CamelCase.toSpaced( parameter.name )

	if parameter.presetsOnly :

		knob = nuke.Enumeration_Knob( knobName, knobLabel, [] )
		knob.setValues( parameter.presetNames() )
		knob.setValue( parameter.getCurrentPresetName() )
		knobHolder.addKnob( knob )

	else :

		for (paramClass,knobCreator,toKnobConverter,fromKnobConverter) in __parameterKnobConverters :

			if isinstance( parameter, paramClass ) :
				knob = knobCreator( knobHolder, parameter, knobName, knobLabel )
				if knob :
					knobHolder.addKnob( knob )
					toKnobConverter( knobHolder, parameter, knobName )
				break
		else :

			knob = nuke.Text_Knob( knobName, "Not implemented!" )
			knobHolder.addKnob( knob )

## Set knob values on a given knob holder for a given parameter.
# @param knobHolder The knob holder must support only two methods: knobs() and addKnob()
# @param parameter That's the IECore.Parameter object that contains the value to be set on the knob.
# @param knobName Specifies the knob name for the given parameter object. Child parameters will have knobs named with that same prefix.
def setKnobsFromParameter( knobHolder, parameter, knobName = "parm" ) :

	if parameter.presetsOnly :

		knobHolder.knobs()[ knobName ].setValue( parameter.getCurrentPresetName() )

	else :

		for (paramClass,knobCreator,toKnobConverter,fromKnobConverter) in __parameterKnobConverters :

			if isinstance( parameter, paramClass ) :
				toKnobConverter( knobHolder, parameter, knobName )
				break


## Set parameter values from knobs on a given knob holder.
# @param knobHolder The knob holder must support only two methods: knobs() and addKnob()
# @param parameter That's the IECore.Parameter object that will get the knob value.
# @param knobName Specifies the knob name for the given parameter object. Child parameters will have knobs named with that same prefix.
def setParameterFromKnobs( knobHolder, parameter, knobName = "parm" ) :

	if parameter.presetsOnly :
		parameter.setValue( knobHolder.knobs()[ knobName ].value() )

	else :

		for (paramClass,knobCreator,toKnobConverter,fromKnobConverter) in __parameterKnobConverters :

			if isinstance( parameter, paramClass ) :
				fromKnobConverter( knobHolder, parameter, knobName )
				break

def __createCompoundParameterKnob( knobHolder, parameter, knobName, knobLabel ) :

	if knobLabel:
		knobHolder.addKnob( nuke.Tab_Knob( knobName, knobLabel, nuke.TABBEGINGROUP) )

	for childName, child in parameter.items() :
		createKnobsFromParameter( knobHolder, child, knobName + "_" + childName )

	if knobLabel:
		knobHolder.addKnob( nuke.Tab_Knob( knobName, knobLabel, nuke.TABENDGROUP) )

	return None

def __compoundParameterToKnob( knobHolder, parameter, knobName ) :

	with IECore.IgnoredExceptions( KeyError ):
		collapsed = parameter.userData()["UI"]["collapsed"]
		knob = knobHolder.knobs()[ knobName ]
		knob.setValue( collapsed )

	for childName, child in parameter.items() :
		setKnobsFromParameter( knobHolder, child, knobName + "_" + childName )

def __compoundParameterFromKnob( knobHolder, parameter, knobName ) :

	with IECore.IgnoredExceptions( KeyError ):
		knob = knobHolder.knobs()[ knobName ]
		collapsed = bool(knob.getValue())
		if not "UI" in parameter.userData() :
			parameter.userData()["UI"] = IECore.CompoundData()
		parameter.userData()["UI"]["collapsed"] = IECore.BoolData(collapsed)

	for childName, child in parameter.items() :
		setParameterFromKnobs( knobHolder, child, knobName + "_" + childName )

def __createPathParameterKnob( knobHolder, parameter, knobName, knobLabel ) :

	return nuke.File_Knob( knobName, knobLabel )

def __fileSequenceParameterToKnob( knobHolder, parameter, knobName ) :

	knob = knobHolder.knobs()[ knobName ]
	seqPath = nukeFileSequence( parameter.getTypedValue() )
	knob.setValue( seqPath )

def __fileSequenceParameterFromKnob( knobHolder, parameter, knobName ) :

	seqPath = ieCoreFileSequence( knobHolder.knobs()[knobName].value() )
	parameter.setTypedValue( seqPath )

def __pathParameterToKnob( knobHolder, parameter, knobName ) :

	knob = knobHolder.knobs()[ knobName ]
	knob.setValue( parameter.getTypedValue() )

def __typedParameterFromKnob( knobHolder, parameter, knobName ) :

	parameter.setTypedValue( knobHolder.knobs()[knobName].value() )

def __typedParameterToKnob( knobHolder, parameter, knobName ) :

	knob = knobHolder.knobs()[ knobName ]
	knob.setValue( parameter.getTypedValue() )

def __numericParameterFromKnob( knobHolder, parameter, knobName ) :

	parameter.setNumericValue( knobHolder.knobs()[knobName].value() )

def __numericParameterToKnob( knobHolder, parameter, knobName ) :

	knob = knobHolder.knobs()[ knobName ]
	knob.setValue( parameter.getNumericValue() )

def __createStringParameterKnob( knobHolder, parameter, knobName, knobLabel ) :

	return nuke.String_Knob( knobName, knobLabel )

def __stringParameterFromKnob( knobHolder, parameter, knobName ) :

	parameter.setTypedValue( knobHolder.knobs()[knobName].getText() )

def __createIntParameterKnob( knobHolder, parameter, knobName, knobLabel ) :

	return nuke.Int_Knob( knobName, knobLabel )

def __createBoolParameterKnob( knobHolder, parameter, knobName, knobLabel ) :

	knob = nuke.Boolean_Knob( knobName, knobLabel )
	knob.setFlag( nuke.STARTLINE )
	return knob

def __createDoubleParameterKnob( knobHolder, parameter, knobName, knobLabel ) :

	knob = nuke.Double_Knob( knobName, knobLabel )
	knob.setValue( parameter.getNumericValue() )
	return knob

def __createStringVectorParameterKnob( knobHolder, parameter, knobName, knobLabel ):

	return nuke.Multiline_Eval_String_Knob( knobName, knobLabel )

def __stringVectorParameterToKnob( knobHolder, parameter, knobName ):

	knob = knobHolder.knobs()[knobName]
	knob.setValue( "\n".join( parameter.getValue() ) )

def __stringVectorParameterFromKnob( knobHolder, parameter, knobName ) :
	txt = knobHolder.knobs()[knobName].getText()
	if len(txt) :
		values = txt.split("\n")
	else:
		values = []
	parameter.setValue( IECore.StringVectorData( values ) )

def __createNumericVectorParameterKnob( knobHolder, parameter, knobName, knobLabel ):

	knob = nuke.String_Knob( knobName, knobLabel + ' (space separated)' )
	return knob

def __numericVectorParameterToKnob( knobHolder, parameter, knobName ):

	knob = knobHolder.knobs()[knobName]
	knob.setValue( " ".join( map( lambda v: str(v), parameter.getValue() ) ) )

def __numericVectorParameterFromKnob( knobHolder, parameter, knobName ) :

	dataVectorType = type( parameter.getValue() )
	dataType = IECore.DataTraits.valueTypeFromSequenceType( dataVectorType )
	values = knobHolder.knobs()[knobName].getText().strip()
	if len(values) :
		dataValues = map( lambda v: dataType(v), values.split() )
		parameter.setValue( dataVectorType( dataValues ) )
	else :
		parameter.setValue( dataVectorType() )

# \todo: Implement data types V2f, V3f, Color...
# \todo: Implement vector data types: V2f, V3f...
registerParameterKnobConverters( IECore.CompoundParameter, __createCompoundParameterKnob, __compoundParameterToKnob, __compoundParameterFromKnob )
registerParameterKnobConverters( IECore.FileSequenceParameter, __createPathParameterKnob, __fileSequenceParameterToKnob, __fileSequenceParameterFromKnob )
registerParameterKnobConverters( IECore.FileNameParameter, __createPathParameterKnob, __pathParameterToKnob, __stringParameterFromKnob )
registerParameterKnobConverters( IECore.DirNameParameter, __createPathParameterKnob, __pathParameterToKnob, __stringParameterFromKnob )
registerParameterKnobConverters( IECore.StringParameter, __createStringParameterKnob, __typedParameterToKnob, __stringParameterFromKnob )
registerParameterKnobConverters( IECore.BoolParameter, __createBoolParameterKnob, __typedParameterToKnob, __typedParameterFromKnob )
registerParameterKnobConverters( IECore.IntParameter, __createIntParameterKnob, __numericParameterToKnob, __numericParameterFromKnob )
registerParameterKnobConverters( IECore.FloatParameter, __createDoubleParameterKnob, __numericParameterToKnob, __numericParameterFromKnob )
registerParameterKnobConverters( IECore.DoubleParameter, __createDoubleParameterKnob, __numericParameterToKnob, __numericParameterFromKnob )
registerParameterKnobConverters( IECore.StringVectorParameter, __createStringVectorParameterKnob, __stringVectorParameterToKnob, __stringVectorParameterFromKnob )
registerParameterKnobConverters( IECore.IntVectorParameter, __createNumericVectorParameterKnob, __numericVectorParameterToKnob, __numericVectorParameterFromKnob )
registerParameterKnobConverters( IECore.FloatVectorParameter, __createNumericVectorParameterKnob, __numericVectorParameterToKnob, __numericVectorParameterFromKnob )
registerParameterKnobConverters( IECore.DoubleVectorParameter, __createNumericVectorParameterKnob, __numericVectorParameterToKnob, __numericVectorParameterFromKnob )


from _IECoreNuke import *

from KnobAccessors import setKnobValue, getKnobValue


import nuke
import IECore

__getters = {}
__setters = {}

##########################################################################
# public functions
##########################################################################

def registerAccessors( knobType, get, set = None ) :

	assert( not knobType in __getters )
	__getters[knobType] = get

	if set :
		assert( not knobType in __setters )
		__setters[knobType] = set

## \todo Support more knob types
def getKnobValue( knob, **kwArgs ) :

	f = __getters.get( knob.__class__, None )
	if not f :
		raise RuntimeError( "No get accessor registered for type %s", str( knob.__class__ ) )

	return f( knob, **kwArgs )

def setKnobValue( knob, value, **kwArgs ) :

	f = __setters.get( knob.__class__, None )
	if not f :
		raise RuntimeError( "No set accessor registered for type %s", str( knob.__class__ ) )

	return f( knob, value, **kwArgs )

##########################################################################
# accessor implementations
##########################################################################

# XY Knob

def __getXY( knob, resultType=IECore.V2f ) :

	return resultType( knob.getValue( 0 ), knob.getValue( 1 ) )

def __setXY( knob, value ) :

	knob.setValue( value[0], 0 )
	knob.setValue( value[1], 1 )

registerAccessors( nuke.XY_Knob, __getXY, __setXY )

# XYZ Knob

def __getXYZ( knob, resultType=IECore.V3f ) :

	return resultType( knob.getValue( 0 ), knob.getValue( 1 ), knob.getValue( 2 ) )

def __setXYZ( knob, value ) :

	knob.setValue( value[0], 0 )
	knob.setValue( value[1], 1 )
	knob.setValue( value[2], 2 )

registerAccessors( nuke.XYZ_Knob, __getXYZ, __setXYZ )
registerAccessors( nuke.Scale_Knob, __getXYZ, __setXYZ )

# Color Knob

def __getColor( knob, resultType=IECore.Color3f ) :

	return resultType( knob.getValue( 0 ), knob.getValue( 1 ), knob.getValue( 2 ) )

def __setColor( knob, value ) :

	knob.setValue( value[0], 0 )
	knob.setValue( value[1], 1 )
	knob.setValue( value[2], 2 )

registerAccessors( nuke.Color_Knob, __getColor, __setColor )

# String Knob

def __getString( knob, resultType=str ) :
	
	return resultType( knob.getText() )
	
def __setString( knob, value ) :

	knob.setValue( str( value ) )
	
registerAccessors( nuke.EvalString_Knob, __getString, __setString )

# Box3 Knob

def __getBox3( knob, resultType=IECore.Box3f ) :

	vectorType = IECore.V3f
	if resultType == IECore.Box3d :
		vectorType = IECore.V3d
		
	value = knob.getValue()
	return resultType( vectorType( *value[:3] ), vectorType( *value[3:] ) )

def __setBox3( knob, value ) :

	knob.setX( value.min[0] )
	knob.setY( value.min[1] )
	knob.setN( value.min[2] )
	
	knob.setR( value.max[0] )
	knob.setT( value.max[1] )
	knob.setF( value.max[2] )
	
registerAccessors( nuke.Box3_Knob, __getBox3, __setBox3 )


from FnAxis import FnAxis


import math
import nuke
import IECore
from KnobAccessors import getKnobValue

## This function set can be used to manipulate any nodes which have
# an axis knob. This includes the Axis, TransformGeo and Camera nodes.
## \todo Methods for dealing with TransformationMatrices, absolute transforms
# and for setting transforms.
class FnAxis :

	def __init__( self, node ) :

		if isinstance( node, str ) :

			axis = nuke.toNode( node )

		self.__node = node

	## Returns the transformation matrix for the local axis knob.
	# This ignores any parent transforms.
	def getLocalMatrix( self, resultType=IECore.M44f ) :

		vectorType = IECore.V3f
		eulerType = IECore.Eulerf
		if resultType==IECore.M44d :
			vectorType = IECore.V3d
			eulerType = IECore.Eulerd

		translate = getKnobValue( self.__node.knob( "translate" ), resultType=vectorType )
		translate = resultType.createTranslated( translate )

		pivot = getKnobValue( self.__node.knob( "pivot" ), resultType=vectorType )

		rotate = getKnobValue( self.__node.knob( "rotate" ), resultType=vectorType )
		rotate *= math.pi / 180.0

		rotOrderKnob = self.__node.knob( "rot_order" )
		rotateOrder = rotOrderKnob.enumName( int(rotOrderKnob.getValue()) )
		rotate = eulerType( rotate, getattr( eulerType.Order, rotateOrder ), eulerType.InputLayout.XYZLayout )
		rotate = rotate.toMatrix44()
		rotate = resultType.createTranslated( -pivot ) * rotate * resultType.createTranslated( pivot )

		scale = getKnobValue( self.__node.knob(  "scaling" ), resultType=vectorType )
		scale *= self.__node.knob( "uniform_scale" ).getValue()
		scale = resultType.createScaled( scale )
		scale = resultType.createTranslated( -pivot ) * scale * resultType.createTranslated( pivot )

		orderKnob = self.__node.knob( "xform_order" )
		order = orderKnob.enumName( int(orderKnob.getValue()) )
		matrices = {
			"T" : translate,
			"R" : rotate,
			"S" : scale
		}

		result = resultType()
		for m in order :
			result = result * matrices[m]

		return result

from StringUtil import nukeFileSequence, ieCoreFileSequence


import re

def __toFormatString( match ):
	txt = match.group(0)
	cc = len(txt)
	if cc == 1 :
		result = "%d"
	else :
		result = "%%0%dd" % cc
	return result

def __toNumberSign( match ):
	txt = match.group(1)
	cc = 1
	if len(txt) :
		cc = int(txt)
		if txt[0] != '0' and cc > 1 :
			raise RuntimeError, "Frame numbers padded with space is not supported!"
	result = ""
	for c in xrange( cc ) :
		result = result + "#"
	return result

## Converts IECore standard file sequence path to Nuke's syntax
def nukeFileSequence( ieCorefs ) :

	return re.sub( "#+", __toFormatString, ieCorefs )

## Converts Nuke standard file sequence path to IECore's syntax
def ieCoreFileSequence( nukefs ) :

	return re.sub( "%([0-9]*)d", __toNumberSign, nukefs )

from KnobConverters import registerParameterKnobConverters, createKnobsFromParameter, setKnobsFromParameter, setParameterFromKnobs


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

from FnParameterisedHolder import FnParameterisedHolder


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

from FnProceduralHolder import FnProceduralHolder


import nuke

import IECoreNuke

class FnProceduralHolder( IECoreNuke.FnParameterisedHolder ) :

	def __init__( self, node ) :
	
		IECoreNuke.FnParameterisedHolder.__init__( self, node )

	def setProcedural( self, className, classVersion=None ) :
	
		self.setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS" )
		
	def getProcedural( self ) :
	
		return self.getParameterised()[0]

	@staticmethod
	def create( nodeName, className, classVersion=None ) :
	
		node = nuke.createNode( "ieProcedural" )
		node.setName( nodeName )
		node.knob( "postage_stamp" ).setValue( False )
		fnPH = FnProceduralHolder( node )
		fnPH.setProcedural( className, classVersion )
		
		return fnPH

from UndoManagers import UndoState, UndoDisabled, UndoEnabled, UndoBlock


import nuke

## A context object intended for use with python's "with" block. It ensures
# that all operations in the block are performed with nuke's undo in a
# particular state (enabled or disabled) and that the previous state is correctly
# restored on exit from the block.
class UndoState :

	## state should be True to enable undo, and False to disable it.
	def __init__( self, state ) :
	
		self.__state = state

	def __enter__( self ) :

		self.__prevState = not nuke.Undo.disabled()
		if self.__state :
			nuke.Undo.enable()
		else :
			nuke.Undo.disable()

	def __exit__( self, type, value, traceBack ) :

		if self.__prevState :
			nuke.Undo.enable()
		else :
			nuke.Undo.disable()

## A context object intended for use with python's "with" block. It ensures
# that all operations in the block are performed with undo disabled, and that
# undo is reenabled if necessary upon exit from the block.
class UndoDisabled( UndoState ) :

	def __init__( self ) :
	
		UndoState.__init__( self, False )

## A context object intended for use with python's "with" block. It ensures
# that all operations in the block are performed with undo enabled, and that
# undo is disabled if necessary upon exit from the block.
class UndoEnabled( UndoState ) :

	def __init__( self ) :
	
		UndoState.__init__( self, True )

## A context object intended for use with python's "with" block. It groups a
# series of actions into a single Nuke undo.
class UndoBlock :

	def __enter__( self ) :
	
		nuke.Undo.begin()
		
	def __exit__( self, type, value, traceBack ) :

		nuke.Undo.end()
		

from TestCase import TestCase


import unittest

import nuke

## A class to help implement unit tests for nuke functionality. It
# implements setUp() to clear the nuke script and undo queue.
class TestCase( unittest.TestCase ) :

	## Derived classes may override this, but they should call the
	# base class implementation too.
	def setUp( self ) :
	
		nuke.scriptClear()
		nuke.Undo.undoTruncate( 0 )
		nuke.Undo.redoTruncate( 0 )

from FnOpHolder import FnOpHolder


import nuke

import IECoreNuke
import _IECoreNuke

class FnOpHolder( IECoreNuke.FnParameterisedHolder ) :

	def __init__( self, node ) :
	
		IECoreNuke.FnParameterisedHolder.__init__( self, node )

	def setOp( self, className, classVersion=None ) :
	
		self.setParameterised( className, classVersion, "IECORE_OP_PATHS" )
		
	def getOp( self ) :
	
		return self.getParameterised()[0]
	
	## Executes the node, returning the result of the Op. You may also
	# use nuke.execute() directly on an OpHolder node, but in this case
	# the result is not returned (which you might not need).
	def execute( self ) :
	
		nuke.execute( self.node(), nuke.frame(), nuke.frame() )
		return _IECoreNuke._opHolderExecuteResult()

	@staticmethod
	def create( nodeName, className, classVersion=None ) :
	
		node = nuke.createNode( "ieOp" )
		node.setName( nodeName )
		fnOH = FnOpHolder( node )
		fnOH.setOp( className, classVersion )
		
		return fnOH

import Menus

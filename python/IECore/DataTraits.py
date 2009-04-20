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

## \file DataTraits.py
#  Provides some utility functions for inspecting Data objects/classes and their respective element datas.
#
# \ingroup python

import string
import datetime
import _IECore as IECore
from Log import *

## Utility function that recognizes objects that are simple types. That means, 
# have one single value on it and are not IMath data types. 
# VectorData, CompoundData and IMath-type Data are not simple.
# Returns a boolean indicating if the given object is a simple Data.
def isSimpleDataType(obj):
	
	if not isinstance(obj, IECore.Data):
		return False
	if hasattr(obj.__class__, "value"):
		objClass = type(obj)
		info = __dataTypesConversionDict[objClass]
		if info is None:
			return False
		# check if the element type a basic python type.
		if info[0] in [bool, int, long, str, float]:
			return True
	return False

##	Utility function that recognizes objects that are numeric simple types. That means, 
#	have one single value on it, and it is numeric. For example, IntData. But not
#	IntVectorData, nor CharData which just accepts characters (up to now).
def isSimpleNumericDataType(obj):
	if not isinstance(obj, IECore.Data):
		return False
	if hasattr(obj.__class__, "value"):
		objClass = type(obj)
		info = __dataTypesConversionDict[objClass]
		if info is None:
			return False
		# check if the element type a basic python type.
		if info[0] in [int, long, float]:
			return True
	return False

## Utility function that recognizes objects that hold values as matrices.
# For example the IMath types: V2f, M33f, Color3f. But not vectors of those types.
def isMatrixDataType(obj):
	
	if not hasattr(obj.__class__, "value"):
		return False
	##\ todo: this attr doesn't guarantee a matrix or not. Quats and transformation matrices don't have dimension attrs
	if hasattr(obj.value.__class__, "dimensions"):
		# vectors, colors and matrices
		return True
	return False

## Utility function that returns ``True`` if a Data object obj could be created with a dict.
#
def isMappingDataType(obj):
	objClass = type(obj)
	info = __dataTypesConversionDict.get(objClass, None)
	if info is None:
		return False
	if info[0] is dict:
		return True
	return False

## Utility function that returns ``True`` if a Data object obj could be created from a list.
#
def isSequenceDataType(obj):
	objClass = type(obj)
	info = __dataTypesConversionDict.get(objClass, None)
	if info is None:
		return False
	if info[0] is list:
		return True
	return False

# map Data types => (element type, instantiate from element flag, [indexed element type])
__dataTypesConversionDict = {
	IECore.Data: None,
	IECore.BoolData: (bool, True),
	IECore.IntData: (int, True),
	IECore.UIntData: (int, False),
	IECore.CharData: (str, False),
	IECore.UCharData: (int, False),
	IECore.StringData: (str, True),
	IECore.FloatData: (float, False),
	IECore.DoubleData: (float, True),
	IECore.HalfData: (float, False),
	IECore.ShortData: (int, False),
	IECore.UShortData: (int, False),
	IECore.Int64Data: (int, False),
	IECore.UInt64Data: (int, False),	
		

	IECore.V2fData: (IECore.V2f, True),
	IECore.V2dData: (IECore.V2d, True),
	IECore.V2iData: (IECore.V2i, True),
	IECore.V3iData: (IECore.V3i, True),
	IECore.V3fData: (IECore.V3f, True),
	IECore.V3dData: (IECore.V3d, True),
	IECore.QuatfData: (IECore.Quatf, True),
	IECore.QuatdData: (IECore.Quatd, True),
	IECore.Color3fData: (IECore.Color3f, True),
	IECore.Color3dData: (IECore.Color3d, True),
	IECore.Color4fData: (IECore.Color4f, True),
	IECore.Color4dData: (IECore.Color4d, True),
	IECore.Box2iData: (IECore.Box2i, True),
	IECore.Box3iData: (IECore.Box3i, True),
	IECore.Box2fData: (IECore.Box2f, True),
	IECore.Box2dData: (IECore.Box2d, True),
	IECore.Box3fData: (IECore.Box3f, True),
	IECore.Box3dData: (IECore.Box3d, True),
	IECore.M33fData: (IECore.M33f, True),
	IECore.M33dData: (IECore.M33d, True),
	IECore.M44fData: (IECore.M44f, True),
	IECore.M44dData: (IECore.M44d, True),

	IECore.BoolVectorData: ( list, False, bool),
	IECore.CharVectorData: (list, False, str),
	IECore.UCharVectorData: (list, False, int),
	IECore.IntVectorData: (list, False, int),
	IECore.UIntVectorData: (list, False, int),
	IECore.HalfVectorData: (list, False, float),
	IECore.FloatVectorData: (list, False, float),
	IECore.DoubleVectorData: (list, False, float),
	IECore.StringVectorData: (list, False, str),
	IECore.ShortVectorData: (list, False, int),
	IECore.UShortVectorData: (list, False, int),
	IECore.Int64VectorData: (list, False, int),
	IECore.UInt64VectorData: (list, False, int),	
	IECore.V2fVectorData: (list, False, IECore.V2f),
	IECore.V2dVectorData: (list, False, IECore.V2d),
	IECore.V2iVectorData: (list, False, IECore.V2i),
	IECore.V3fVectorData: (list, False, IECore.V3f),
	IECore.V3dVectorData: (list, False, IECore.V3d),
	IECore.V3iVectorData: (list, False, IECore.V3i),
	IECore.QuatfVectorData: (list, False, IECore.Quatf),
	IECore.QuatdVectorData: (list, False, IECore.Quatd),
	IECore.Box2iVectorData: (list, False, IECore.Box2i),
	IECore.Box2fVectorData: (list, False, IECore.Box2f),
	IECore.Box2dVectorData: (list, False, IECore.Box2d),
	IECore.Box3iVectorData: (list, False, IECore.Box3i),
	IECore.Box3fVectorData: (list, False, IECore.Box3f),
	IECore.Box3dVectorData: (list, False, IECore.Box3d),
	IECore.M33fVectorData: (list, False, IECore.M33f),
	IECore.M33dVectorData: (list, False, IECore.M33d),
	IECore.M44fVectorData: (list, False, IECore.M44f),
	IECore.M44dVectorData: (list, False, IECore.M44d),
	IECore.Color3fVectorData: (list, False, IECore.Color3f),
	IECore.Color3dVectorData: (list, False, IECore.Color3d),
	IECore.Color4fVectorData: (list, False, IECore.Color4f),
	IECore.Color4dVectorData: (list, False, IECore.Color4d),

	IECore.CompoundData: (dict, True, None),

	IECore.TransformationMatrixfData: ( IECore.TransformationMatrixf, True ),
	IECore.TransformationMatrixdData: ( IECore.TransformationMatrixd, True ),
	
	IECore.SplineffData: ( IECore.Splineff, True ),
	IECore.SplineddData: ( IECore.Splinedd, True ),
	IECore.SplinefColor3fData: ( IECore.SplinefColor3f, True ),
	IECore.SplinefColor4fData: ( IECore.SplinefColor4f, True ),
	
	IECore.CubeColorLookupfData: ( IECore.CubeColorLookupf, True ),
	IECore.CubeColorLookupdData: ( IECore.CubeColorLookupd, True ),
	
	IECore.DateTimeData: ( datetime.datetime, True ),
	IECore.TimeDurationData: ( datetime.timedelta, True ),
	IECore.TimePeriodData: ( IECore.TimePeriod, True ),


}

## Function that returns a list of Data derived classes.
def getDataDerivedTypes():
	
	dataTypesList = __dataTypesConversionDict.keys()
	dataTypesList.remove(IECore.Data)
	return dataTypesList

## Returns the type (class) for the element type hold on the instances of the given Data type. In fact, returns the class that is 
# used to instantiate the Data type.
# For example, if the Data class returns true from isSequenceType(), then this function returns the type 'list'.
# See also: valueTypeFromSequenceType()
def elementTypeFromDataType(dataType):
	
	dataInfo = __dataTypesConversionDict[dataType]
	if dataInfo is None:
		raise TypeError, "This Data class can not be instantiated."
	return dataInfo[0]

## Returns the type (class) used on each indexed value on the given sequence type.
# For example: it returns 'int' for the IntVectorData class.
def valueTypeFromSequenceType(sequenceType):
	
	dataInfo = __dataTypesConversionDict[sequenceType]
	if dataInfo is None:
		raise TypeError, "This Data class can not be instantiated."
	if len(dataInfo) < 3:
		raise TypeError, "This Data class is not a sequence type!"
	return dataInfo[2]

## Returns the Data class that is instantiable given an element type.
def dataTypeFromElementType(elementType):
	
	for (dataType, value) in __dataTypesConversionDict.items():
		if value is None:
			continue
		if value[0] is elementType and value[1]:
			return dataType
	raise TypeError, "No Data type is compatible with the given element type: %s" % ( elementType )

## Returns the Data class that is instantiable given a element data object.
# It also instantiate container Data objects, like VectorData and CompoundData, given the proper list and dict.
def dataTypeFromElement(element):
	
	# threat the VectorData type exception...
	if isinstance(element, list) and len(element) > 0:
		# We have to check the list contents.
		elementValueType = type(element[0])
		for (dataType, value) in __dataTypesConversionDict.items():
			if value[0] is list and len(value) >= 2 and value[2] is elementValueType:
				return dataType

	return dataTypeFromElementType(type(element))

## Factory function for Data objects given its element data.
# Parameters:
# obj  -  any IECore simple structure objects (Color3f, V3f, ...) or python simple types (int, str, list, dict...)
# Returns:
# A Data object that holds the given element data object.
def dataFromElement(element):

	# An empty list or empty set is ambiguous - we don't know if it should be a StringVectorData, IntVectorData, or anything
	if element == [] or element == set() : 
	
		raise RuntimeError, "Cannot determine Data type for ambiguous element: %s" % ( str( element ) )
	
	dataType = dataTypeFromElement(element)
	return dataType(element)

__all__ = [ "isSimpleDataType", "isSimpleNumericDataType", "isMatrixDataType", "isMappingDataType",
	"isSequenceDataType", "getDataDerivedTypes", "elementTypeFromDataType", "valueTypeFromSequenceType",
	"dataTypeFromElementType", "dataTypeFromElement", "dataFromElement",
]

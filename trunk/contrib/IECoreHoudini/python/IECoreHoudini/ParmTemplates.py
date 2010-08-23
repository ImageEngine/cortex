##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

import hou, IECore, IECoreHoudini
import string, math

# top-level parameter creation method
# takes a cortex parameter and creates a HOM parm tuple
def createParm( p ):
	parm = None

	# int
	if p.typeId()==IECore.TypeId.IntParameter:
		parm = IECoreHoudini.ParmTemplates.intParm( p )
	if p.typeId()==IECore.TypeId.V2iParameter:
		parm = IECoreHoudini.ParmTemplates.intParm( p, 2 )
	if p.typeId()==IECore.TypeId.V3iParameter:
		parm = IECoreHoudini.ParmTemplates.intParm( p, 3 )

	# float, V2f, V3f
	if p.typeId()==IECore.TypeId.FloatParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p )
	if p.typeId()==IECore.TypeId.V2fParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 2 )
	if p.typeId()==IECore.TypeId.V3fParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 3 )

	# double
	if p.typeId()==IECore.TypeId.DoubleParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p )
	if p.typeId()==IECore.TypeId.V2dParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 2 )
	if p.typeId()==IECore.TypeId.V3dParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 3 )

	# bool
	if p.typeId()==IECore.TypeId.BoolParameter:
		parm = IECoreHoudini.ParmTemplates.boolParm( p )

	# string
	if p.typeId()==IECore.TypeId.StringParameter:
		parm = IECoreHoudini.ParmTemplates.stringParm( p )

	# path, dirname, filename, filesequence
	if p.typeId()==IECore.TypeId.PathParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p )

	if p.typeId()==IECore.TypeId.DirNameParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p )

	if p.typeId()==IECore.TypeId.FileNameParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p )

	if p.typeId()==IECore.TypeId.FileSequenceParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p )

	# color3f
	if p.typeId()==IECore.TypeId.Color3fParameter:
		parm = IECoreHoudini.ParmTemplates.colParm( p, 3 )

	# color4f
	if p.typeId()==IECore.TypeId.Color4fParameter:
		parm = IECoreHoudini.ParmTemplates.colParm( p, 4 )

	# M44f, M44d
	if p.typeId()==IECore.TypeId.M44fParameter:
		parm = IECoreHoudini.ParmTemplates.matrixParm( p )
	if p.typeId()==IECore.TypeId.M44dParameter:
		parm = IECoreHoudini.ParmTemplates.matrixParm( p )

	# Box2i, Box2f, Box2d
	if p.typeId()==IECore.TypeId.Box2iParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmInt( p, 2 )
	if p.typeId()==IECore.TypeId.Box2fParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 2 )
	if p.typeId()==IECore.TypeId.Box2dParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 2 )

	# Box3i, Box3f, Box3d
	if p.typeId()==IECore.TypeId.Box3iParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmInt( p, 3 )
	if p.typeId()==IECore.TypeId.Box3fParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 3 )
	if p.typeId()==IECore.TypeId.Box3dParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 3 )

	# is this parameter hidden?
	if "hidden" in p.userData():
		hidden = bool(p.userData()["label"].value)
		parm['houdini_tuple'].hide( hidden )

	return parm

# tries to pretty-print a parameter, in case it doesn't have a label
def labelFormat( str ):
	return string.capwords( ' '.join( str.split("_") ) )

def parmName( n, dim=1 ) :
	return "parm_%s" % n

#=====
# use the following to find out how to call template()
# n.parm("intparm").parmTemplate().asCode()
def intParm( p, dim=1 ):
	name = parmName( p.name, dim )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)

	# only simple floats have min/max values
	if dim==1:
		default = [(p.defaultValue.value)]
		min_val = 0
		max_val = 10
		min_lock = max_lock = False
		if p.hasMinValue():
			min_val = p.minValue
			min_lock = True
		if p.hasMaxValue():
			max_val = p.maxValue
			max_lock = True
	else:
		default = list(p.defaultValue.value)
		min_val = 0
		max_val = 10
		min_lock = max_lock = False

	naming = hou.parmNamingScheme.Base1
	if dim>1:
		naming = hou.parmNamingScheme.XYZW

	parm = hou.IntParmTemplate( name, label, dim,
									default_value=default,
								 	min=min_val,
								 	max=max_val,
								 	min_is_strict=min_lock,
								 	max_is_strict=max_lock,
								 	naming_scheme=naming
								 )
	return {'houdini_name':name, 'houdini_tuple':parm }

#=====
# use the following to find out how to call template()
# n.parm("floatparm").parmTemplate().asCode()
def floatParm( p, dim=1 ):
	name = parmName( p.name, dim )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)

	# only simple floats have min/max values
	if dim==1:
		default = [(p.defaultValue.value)]
		min_val = 0.0
		max_val = 10.0
		min_lock = max_lock = False
		if p.hasMinValue():
			min_val = p.minValue
			min_lock = True
		if p.hasMaxValue():
			max_val = p.maxValue
			max_lock = True
	else:
		default = list(p.defaultValue.value)
		min_val = 0.0
		max_val = 10.0
		min_lock = max_lock = False

	naming = hou.parmNamingScheme.Base1
	if dim>1:
		naming = hou.parmNamingScheme.XYZW

	parm = hou.FloatParmTemplate( name, label, dim,
									default_value=default,
								 	min=min_val,
								 	max=max_val,
								 	min_is_strict=min_lock,
								 	max_is_strict=max_lock,
								 	look=hou.parmLook.Regular,
								 	naming_scheme=naming
								 )
	return {'houdini_name':name, 'houdini_tuple':parm }

#=====
# use the following to find out how to call template()
# n.parm("boolparm").parmTemplate().asCode()
def boolParm( p ):
	name = parmName( p.name )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)
	default = p.defaultValue.value
	parm = hou.ToggleParmTemplate( name, label, default_value=default, disable_when="")
	return {'houdini_name':name, 'houdini_tuple':parm }

def stringParm( p ):
	name = parmName( p.name )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)
	default = ([p.defaultValue.value])
	parm = hou.StringParmTemplate( name, label, 1,
									default_value=default,
									naming_scheme=hou.parmNamingScheme.Base1
									)
	return {'houdini_name':name, 'houdini_tuple':parm }

def pathParm( p ):
	name = parmName( p.name )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)
	default = ([p.defaultValue.value])
	parm = hou.StringParmTemplate( name, label, 1,
									default_value=default,
									naming_scheme=hou.parmNamingScheme.Base1,
									string_type=hou.stringParmType.FileReference
									)
	return {'houdini_name':name, 'houdini_tuple':parm }

def colParm( p, dim ):
	name = parmName( p.name, dim )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)
	default = list(p.defaultValue.value)
	parm = hou.FloatParmTemplate( name, label, dim,
									default_value=default,
									min=0,
									max=1,
									min_is_strict=False,
									max_is_strict=False,
									look=hou.parmLook.ColorSquare,
									naming_scheme=hou.parmNamingScheme.RGBA
									)
	return {'houdini_name':name, 'houdini_tuple':parm }

def matrixParm( p, dim=16 ):
	name = parmName( p.name, dim )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)
	default_matrix = p.defaultValue.value
	default = []
	dim_sqrt = int(math.sqrt(dim))
	for i in range(dim_sqrt):
		for j in range(dim_sqrt):
			default.append( default_matrix[(i,j)] )
	min_val = 0.0
	max_val = 10.0
	min_lock = max_lock = False
	parm = hou.FloatParmTemplate( name, label, dim,
									default_value=default,
								 	min=min_val,
								 	max=max_val,
								 	min_is_strict=min_lock,
								 	max_is_strict=max_lock,
								 	look=hou.parmLook.Regular,
								 	naming_scheme=hou.parmNamingScheme.Base1
								 )
	return {'houdini_name':name, 'houdini_tuple':parm }

def boxParmInt( p, dim ):
	name = parmName( p.name, dim )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)

	default_box = p.defaultValue.value
	default = []
	for i in range(dim):
		default.append( default_box.min[i] )
	for i in range(dim):
		default.append( default_box.max[i] )

	min_val = 0
	max_val = 10
	min_lock = max_lock = False
	parm = hou.IntParmTemplate( name, label, dim * 2,
				default_value=default,
				min=min_val,
			 	max=max_val,
			 	min_is_strict=min_lock,
			 	max_is_strict=max_lock,
			 	naming_scheme=hou.parmNamingScheme.Base1
			 )
	return {'houdini_name':name, 'houdini_tuple':parm }

def boxParmFloat( p, dim ):
	name = parmName( p.name, dim )
	if "label" in p.userData():
		label = p.userData()["label"].value
	else:
		label = labelFormat(p.name)

	default_box = p.defaultValue.value
	default = []
	for i in range(dim):
		default.append( default_box.min[i] )
	for i in range(dim):
		default.append( default_box.max[i] )

	min_val = 0.0
	max_val = 10.0
	min_lock = max_lock = False
	parm = hou.FloatParmTemplate( name, label, dim * 2,
				default_value=default,
				min=min_val,
			 	max=max_val,
			 	min_is_strict=min_lock,
			 	max_is_strict=max_lock,
			 	look=hou.parmLook.Regular,
			 	naming_scheme=hou.parmNamingScheme.Base1
			 )
	return {'houdini_name':name, 'houdini_tuple':parm }

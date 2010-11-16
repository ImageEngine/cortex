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

'''
This contains template methods for creating Houdini parameters. They are mainly used by IECoreHoudini
to create the interface for procedurals/ops.
'''

# top-level parameter creation method
# takes a cortex parameter and creates a list of HOM parm tuples
def createParm( p, folders=None, parent=None, top_level=False ):
	parm = None
	results = []
	if not folders:
		folders = []
		
	# Compound Parameter
	if p.typeId()==IECore.TypeId.CompoundParameter:
		if top_level==True: # this is our top-level CompoundParameter
			sub_folder = ['Parameters']
			name = None
		else:
			label = parmLabel( p )
			sub_folder = folders + [label]
			name = parmName(p.name, parent=parent)
		
		# recurse through children
		for child in p.values():
			results += createParm( child, sub_folder, parent=name )

	# int
	if p.typeId()==IECore.TypeId.IntParameter:
		parm = IECoreHoudini.ParmTemplates.intParm( p, parent=parent )
	if p.typeId()==IECore.TypeId.V2iParameter:
		parm = IECoreHoudini.ParmTemplates.intParm( p, 2, parent=parent )
	if p.typeId()==IECore.TypeId.V3iParameter:
		parm = IECoreHoudini.ParmTemplates.intParm( p, 3, parent=parent )

	# float, V2f, V3f
	if p.typeId()==IECore.TypeId.FloatParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, parent=parent )
	if p.typeId()==IECore.TypeId.V2fParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 2, parent=parent )
	if p.typeId()==IECore.TypeId.V3fParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 3, parent=parent )

	# double
	if p.typeId()==IECore.TypeId.DoubleParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, parent=parent )
	if p.typeId()==IECore.TypeId.V2dParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 2, parent=parent )
	if p.typeId()==IECore.TypeId.V3dParameter:
		parm = IECoreHoudini.ParmTemplates.floatParm( p, 3, parent=parent )

	# bool
	if p.typeId()==IECore.TypeId.BoolParameter:
		parm = IECoreHoudini.ParmTemplates.boolParm( p, parent=parent )

	# string
	if p.typeId()==IECore.TypeId.StringParameter:
		parm = IECoreHoudini.ParmTemplates.stringParm( p, parent=parent )

	# path, dirname, filename, filesequence
	if p.typeId()==IECore.TypeId.PathParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p, parent=parent )

	if p.typeId()==IECore.TypeId.DirNameParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p, parent=parent )

	if p.typeId()==IECore.TypeId.FileNameParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p, parent=parent )

	if p.typeId()==IECore.TypeId.FileSequenceParameter:
		parm = IECoreHoudini.ParmTemplates.pathParm( p, parent=parent )

	# color3f
	if p.typeId()==IECore.TypeId.Color3fParameter:
		parm = IECoreHoudini.ParmTemplates.colParm( p, 3, parent=parent )

	# color4f
	if p.typeId()==IECore.TypeId.Color4fParameter:
		parm = IECoreHoudini.ParmTemplates.colParm( p, 4, parent=parent )

	# M44f, M44d
	if p.typeId()==IECore.TypeId.M44fParameter:
		parm = IECoreHoudini.ParmTemplates.matrixParm( p, parent=parent )
	if p.typeId()==IECore.TypeId.M44dParameter:
		parm = IECoreHoudini.ParmTemplates.matrixParm( p, parent=parent )

	# Box2i, Box2f, Box2d
	if p.typeId()==IECore.TypeId.Box2iParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmInt( p, 2, parent=parent )
	if p.typeId()==IECore.TypeId.Box2fParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 2, parent=parent )
	if p.typeId()==IECore.TypeId.Box2dParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 2, parent=parent )

	# Box3i, Box3f, Box3d
	if p.typeId()==IECore.TypeId.Box3iParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmInt( p, 3, parent=parent )
	if p.typeId()==IECore.TypeId.Box3fParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 3, parent=parent )
	if p.typeId()==IECore.TypeId.Box3dParameter:
		parm = IECoreHoudini.ParmTemplates.boxParmFloat( p, 3, parent=parent )

	if parm:
		# is this parameter hidden?
		do_hide = False
		if 'UI' in p.userData() and 'visible' in p.userData()['UI']:
			do_hide = not bool(p.userData()['UI']['visible'].value)
		if 'hidden' in p.userData(): # TODO: this flag is deprecated
			do_hide = bool(p.userData()['hidden'].value)
		parm['tuple'].hide( do_hide )
			
		# add our list of parent folders
		parm['folder'] = folders
		parm['cortex_name'] = p.name
		
		# add to our list of results
		results.append(parm)
		
	# certain parameter types are ok to ignore
	ignoredParameterTypes = [
		IECore.TypeId.CompoundParameter,
		IECore.TypeId.ObjectParameter,
		IECore.TypeId.PrimitiveParameter,
		IECore.TypeId.PointsPrimitiveParameter,
		IECore.TypeId.MeshPrimitiveParameter,
		IECore.TypeId.CurvesPrimitiveParameter,
		IECore.TypeId.GroupParameter,
	]
	
	if not parm and not p.typeId() in ignoredParameterTypes :
		msg = "IECoreHoudini does not currently support parameters of type " + p.typeName()
		
		# H10 and hbatch/hython don't support ui.setStatusMessage()
		if ( hou.applicationVersion()[0]>10 and hou.isUIAvailable() ) :
			hou.ui.setStatusMessage( msg, hou.severityType.Warning )
		print "Warning: ", msg
	
	# our parent folder list
	return results

# tries to pretty-print a parameter, in case it doesn't have a label
def labelFormat( str ):
	return IECore.CamelCase.toSpaced( str )

# returns a houdini parameter name, give it's cortex name
def parmName( n, parent=None ) :
	if parent:
		prefix = "%s_" % parent
	else:
		prefix = ""
	return "%sparm_%s" % ( prefix, n )

# returns a parameter label
def parmLabel( p ):
        label = labelFormat(p.name)
        if 'UI' in p.userData() and 'label' in p.userData()['UI']:
                label = p.userData()['UI']['label'].value
        if 'label' in p.userData(): # TODO: this userData key is deprecated
                label = p.userData()['label'].value
        return label

#=====
# use the following to find out how to call template()
# n.parm("intparm").parmTemplate().asCode()
def intParm( p, dim=1, parent=None ):
	name = parmName( p.name, parent=parent )
        label = parmLabel( p )

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
	return {'name':name, 'tuple':parm }

#=====
# use the following to find out how to call template()
# n.parm("floatparm").parmTemplate().asCode()
def floatParm( p, dim=1, parent=None ):
	name = parmName( p.name, parent=parent )
	label = parmLabel( p )

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
	return {'name':name, 'tuple':parm }

#=====
# use the following to find out how to call template()
# n.parm("boolparm").parmTemplate().asCode()
def boolParm( p, parent=None ):
	name = parmName( p.name )
	label = parmLabel( p )
	default = p.defaultValue.value
	parm = hou.ToggleParmTemplate( name, label, default_value=default, disable_when="")
	return {'name':name, 'tuple':parm }

def stringParm( p, parent=None ):
	name = parmName( p.name )
	label = parmLabel( p )
	default = ([p.defaultValue.value])
	parm = hou.StringParmTemplate( name, label, 1,
									default_value=default,
									naming_scheme=hou.parmNamingScheme.Base1
									)
	return {'name':name, 'tuple':parm }

def pathParm( p, parent=None ):
	name = parmName( p.name )
	label = parmLabel( p )
	default = ([p.defaultValue.value])
	parm = hou.StringParmTemplate( name, label, 1,
									default_value=default,
									naming_scheme=hou.parmNamingScheme.Base1,
									string_type=hou.stringParmType.FileReference
									)
	return {'name':name, 'tuple':parm }

def colParm( p, dim, parent=None ):
	name = parmName( p.name, parent=parent )
	label = parmLabel( p )
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
	return {'name':name, 'tuple':parm }

def matrixParm( p, dim=16, parent=None ):
	name = parmName( p.name, parent=parent )
	label = parmLabel( p )
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
	return {'name':name, 'tuple':parm }

def boxParmInt( p, dim, parent=None ):
	name = parmName( p.name, parent=parent )
	label = parmLabel( p )
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
	return {'name':name, 'tuple':parm }

def boxParmFloat( p, dim, parent=None ):
	name = parmName( p.name, parent=parent )
	label = parmLabel( p )
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
	return { 'name':name, 'tuple':parm }


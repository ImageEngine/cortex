##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import maya.OpenMaya as OpenMaya
import maya.cmds as cmds

import IECoreMaya
import IECore

def __getFloat3PlugValue(plug):

	# Retrieve the value as an MObject
	object = plug.asMObject()

	# Convert the MObject to a float3
	numDataFn = OpenMaya.MFnNumericData(object)
	xParam = OpenMaya.MScriptUtil()
	xParam.createFromDouble(0.0)
	xPtr = xParam.asFloatPtr()
	yParam = OpenMaya.MScriptUtil()
	yParam.createFromDouble(0.0)
	yPtr = yParam.asFloatPtr()
	zParam = OpenMaya.MScriptUtil()
	zParam.createFromDouble(0.0)
	zPtr = zParam.asFloatPtr()
	numDataFn.getData3Float(xPtr, yPtr, zPtr)
	return OpenMaya.MFloatVector(
		OpenMaya.MScriptUtil(xPtr).asFloat(),
		OpenMaya.MScriptUtil(yPtr).asFloat(),
		OpenMaya.MScriptUtil(zPtr).asFloat()
	)

def __hasTweaks( meshDagPath ):

	fnDN = OpenMaya.MFnDependencyNode( meshDagPath.node() )

	# Tweaks exist only if the multi "pnts" attribute contains plugs
	# which contain non-zero tweak values. 	
	tweakPlug = fnDN.findPlug("pnts")
	if not tweakPlug.isNull():
	
		if not tweakPlug.isArray():
			raise RuntimeError( "tweakPlug is not an array plug" )

		numElements = tweakPlug.numElements()
		for i in range(numElements):
			tweak = tweakPlug.elementByPhysicalIndex(i)
			if not tweak.isNull():
				tweakData = __getFloat3PlugValue(tweak)
				if 0 != tweakData.x or 0 != tweakData.y or 0 != tweakData.z:
					return True
					
	return False

def __hasHistory( meshDagPath ):

	fnDN = OpenMaya.MFnDependencyNode( meshDagPath.node() )

	return fnDN.findPlug("inMesh").isConnected()

def __processUpstreamNode(data, meshDagPath, dgModifier):
	
	if __hasHistory( meshDagPath ):
		# Just swap the connections around		
		tempPlugArray = OpenMaya.MPlugArray()		
		data.meshNodeDestPlug.connectedTo(tempPlugArray, True, False)
		assert( tempPlugArray.length() == 1 )
		
		data.upstreamNodeSrcPlug = tempPlugArray[0]

		data.upstreamNodeShape = data.upstreamNodeSrcPlug.node()
		
		data.upstreamNodeSrcAttr = data.upstreamNodeSrcPlug.attribute()

		dgModifier.disconnect(data.upstreamNodeSrcPlug, data.meshNodeDestPlug)
		dgModifier.doIt()
		
	else:	
		# Duplicate mesh, mark as "intermediate", and reconnect in the DAG
		dagNodeFn = OpenMaya.MFnDagNode( data.meshNodeShape )

		data.upstreamNodeTransform = dagNodeFn.duplicate(False, False)
		dagNodeFn.setObject(data.upstreamNodeTransform)
		
		fDagModifier = OpenMaya.MDagModifier()

		if dagNodeFn.childCount() < 1:
			raise RuntimeError( "Duplicated mesh has no shape" )
						
		data.upstreamNodeShape = dagNodeFn.child(0)

		fDagModifier.reparentNode(data.upstreamNodeShape, data.meshNodeTransform)
		fDagModifier.doIt()
		
		dagNodeFn.setObject(data.upstreamNodeShape)
		dagNodeFn.setIntermediateObject(True)

		data.upstreamNodeSrcAttr = dagNodeFn.attribute("outMesh")
		data.upstreamNodeSrcPlug = dagNodeFn.findPlug("outMesh")
		
		fDagModifier.deleteNode(data.upstreamNodeTransform)
		fDagModifier.doIt()
	
def __processTweaks(data, dgModifier, modifierNode):

	tweakIndexArray = OpenMaya.MIntArray()

	fnDN = OpenMaya.MFnDependencyNode()

	tweakDataArray = OpenMaya.MObjectArray()
	tweakSrcConnectionCountArray = OpenMaya.MIntArray()
	tweakSrcConnectionPlugArray = OpenMaya.MPlugArray()
	tweakDstConnectionCountArray = OpenMaya.MIntArray()
	tweakDstConnectionPlugArray = OpenMaya.MPlugArray()
	tempPlugArray = OpenMaya.MPlugArray()

	tweakNode = dgModifier.createNode("polyTweak")
	fnDN.setObject(tweakNode)
	tweakNodeSrcAttr = fnDN.attribute("output")
	tweakNodeDestAttr = fnDN.attribute("inputPolymesh")
	tweakNodeTweakAttr = fnDN.attribute("tweak")

	fnDN.setObject(data.meshNodeShape)
	meshTweakPlug = fnDN.findPlug("pnts")

	if not meshTweakPlug.isArray() :
		raise RuntimeError( "meshTweakPlug is not an array plug" )

	numElements = meshTweakPlug.numElements()
	for i in range(numElements):
		tweak = meshTweakPlug.elementByPhysicalIndex(i)

		if not tweak.isNull():
		
			tweakIndexArray.append( tweak.logicalIndex() )

			tweakData = tweak.asMObject()
			tweakDataArray.append(tweakData)

			if not tweak.isCompound():
				raise RuntimeError( "Element tweak plug is not a compound" )

			numChildren = tweak.numChildren()
			for j in range(numChildren):
				tweakChild = tweak.child(j)
				if tweakChild.isConnected():

					tempPlugArray.clear()
					if tweakChild.connectedTo(tempPlugArray, False, True):
						numSrcConnections = tempPlugArray.length()
						tweakSrcConnectionCountArray.append(numSrcConnections)

						for k in range(numSrcConnections):
							tweakSrcConnectionPlugArray.append(tempPlugArray[k])
							dgModifier.disconnect(tweakChild, tempPlugArray[k])
					else:
						tweakSrcConnectionCountArray.append(0)

					tempPlugArray.clear()
					if tweakChild.connectedTo(tempPlugArray, True, False):
						assert( tempPlugArray.length() == 1 )

						tweakDstConnectionCountArray.append(1)
						tweakDstConnectionPlugArray.append(tempPlugArray[0])
						dgModifier.disconnect(tempPlugArray[0], tweakChild)
					else:
						tweakDstConnectionCountArray.append(0)
				else:
					tweakSrcConnectionCountArray.append(0)
					tweakDstConnectionCountArray.append(0)

	polyTweakPlug = OpenMaya.MPlug(tweakNode, tweakNodeTweakAttr)
	numTweaks = tweakIndexArray.length()
	srcOffset = 0
	dstOffset = 0
	for i in range(numTweaks):

		tweak = polyTweakPlug.elementByLogicalIndex(tweakIndexArray[i])
		tweak.setMObject(tweakDataArray[i])

		if not tweak.isCompound():
			raise RuntimeError( "Element plug 'tweak' is not a compound" )

		numChildren = tweak.numChildren()
		for j in range(numChildren):
			tweakChild = tweak.child(j)

			if 0 < tweakSrcConnectionCountArray[i*numChildren + j]:
				k = 0
				while (k < tweakSrcConnectionCountArray[i*numChildren + j]):
					dgModifier.connect(tweakChild, tweakSrcConnectionPlugArray[srcOffset])
					srcOffset += 1
					k += 1

			if 0 < tweakDstConnectionCountArray[i*numChildren + j]:
				dgModifier.connect(tweakDstConnectionPlugArray[dstOffset], tweakChild)
				dstOffset += 1
	
	tweakDestPlug = OpenMaya.MPlug( tweakNode, tweakNodeDestAttr )
	dgModifier.connect( data.upstreamNodeSrcPlug, tweakDestPlug )

	tweakSrcPlug = OpenMaya.MPlug( tweakNode, tweakNodeSrcAttr)
	modifierDestPlug = OpenMaya.MPlug( modifierNode, data.modifierNodeDestAttr )
	dgModifier.connect( tweakSrcPlug, modifierDestPlug )				
	
def __connectNodes( modifierNode, meshDagPath ):
	class MeshOpHolderData:
		def __init__(self):
			self.meshNodeTransform = OpenMaya.MObject()
			self.meshNodeShape = OpenMaya.MObject()
			self.meshNodeDestPlug = OpenMaya.MPlug()
			self.meshNodeDestAttr = OpenMaya.MObject()

			self.upstreamNodeTransform = OpenMaya.MObject()
			self.upstreamNodeShape = OpenMaya.MObject()
			self.upstreamNodeSrcPlug = OpenMaya.MPlug()
			self.upstreamNodeSrcAttr = OpenMaya.MObject()

			self.modifierNodeSrcAttr = OpenMaya.MObject()
			self.modifierNodeDestAttr = OpenMaya.MObject()
		
	data = MeshOpHolderData()
	
	fnDN = OpenMaya.MFnDependencyNode( modifierNode )
	data.modifierNodeSrcAttr = fnDN.attribute("result")
	data.modifierNodeDestAttr = fnDN.attribute("parm_input")
	
	data.meshNodeShape = meshDagPath.node()
	dagNodeFn = OpenMaya.MFnDagNode( data.meshNodeShape )
	
	if dagNodeFn.parentCount() == 0:
		raise RuntimeError( "Mesh shape has no parent transform" )
		
	data.meshNodeTransform = dagNodeFn.parent(0)
	data.meshNodeDestPlug = dagNodeFn.findPlug("inMesh")
	data.meshNodeDestAttr = data.meshNodeDestPlug.attribute()

	dgModifier = OpenMaya.MDGModifier()
	__processUpstreamNode(data, meshDagPath, dgModifier)

	if __hasTweaks( meshDagPath ):	
		__processTweaks(data, dgModifier, modifierNode)
	else:
		modifierDestPlug = OpenMaya.MPlug(modifierNode, data.modifierNodeDestAttr)
		dgModifier.connect(data.upstreamNodeSrcPlug, modifierDestPlug)			
	
	modifierSrcPlug = OpenMaya.MPlug(modifierNode, data.modifierNodeSrcAttr)
	meshDestAttr = OpenMaya.MPlug(data.meshNodeShape, data.meshNodeDestAttr)	
	
	dgModifier.connect(modifierSrcPlug, meshDestAttr)

	dgModifier.doIt()
	
def __setParameters( op, kw ):

	for paramName, paramValue in kw.items():
		op.parameters().setValidatedParameterValue( paramName, paramValue )
	

def __createMeshOpNode( className, classVersion, **kw ):

	shortClassName = className.split( '/' ).pop()

	modifierNodeName = cmds.createNode( "ieOpHolderNode", name = shortClassName + "#" )	
	
	ph = IECoreMaya.ParameterisedHolder( str(modifierNodeName) )	
	op = ph.setParameterised( className, classVersion, "IECORE_OP_PATHS" )
					
	__setParameters( op, kw )
	
	selList = OpenMaya.MSelectionList()
	selList.add( modifierNodeName )						
	modifierNode = OpenMaya.MObject()	
	s = selList.getDependNode( 0, modifierNode )
	
	return modifierNode
	
def __applyMeshOp( meshNode, className, classVersion, kw ):

	op = IECore.ClassLoader.defaultOpLoader().load( className, classVersion )
		
	__setParameters( op, **kw )	
	
	# \todo Apply op and convert result back into original object
	
	
def create( meshDagPath, className, classVersion, **kw):

	if type(meshDagPath) is str:	
		sel = OpenMaya.MSelectionList()
		sel.add( meshDagPath )
		meshDagPath = OpenMaya.MDagPath()
		sel.getDagPath( 0,  meshDagPath)
		meshDagPath.extendToShape()
	
	constructionHistoryEnabled = IECoreMaya.mel("constructionHistory -q -tgl").value

	if not __hasHistory( meshDagPath ) and constructionHistoryEnabled == 0:
	
		# \todo we can't actually do this right now because we're unable to convert the resultant MeshPrimitive
		# back into the original meshNode MObject given to us
		raise RuntimeError( "Currently unable to apply MeshOp in-place " )
		
		meshNode = meshDagPath.node()

		__applyMeshOp(meshNode, className, classVersion, **kw )
		
		return None
	else:
		modifierNode = __createMeshOpNode( className, classVersion, **kw )

		__connectNodes( modifierNode, meshDagPath )
		
		fnDN = OpenMaya.MFnDependencyNode( modifierNode )
		
		return str( fnDN.name() )


def createUI( className, classVersion, **kw ):

	# \todo Should we use list selection instead?
	selectedMeshes = cmds.filterExpand( sm = 12 )
	
	if not selectedMeshes:
		raise RuntimeError( "No mesh selected" )
		
	modifierNodes = []
	
	for mesh in selectedMeshes:
	
		sel = OpenMaya.MSelectionList()
		sel.add( mesh )
		meshDagPath = OpenMaya.MDagPath()
		sel.getDagPath( 0,  meshDagPath)
		meshDagPath.extendToShape()
		
		modifierNode = create( meshDagPath, className, classVersion, **kw )
		
		if modifierNode :
		
			modifierNodes += [ modifierNode ]
			
			
	return modifierNodes	

import os, sys, math
import IECore

isRunningInMaya = True
try:
	from maya import cmds, OpenMaya
except:
	isRunningInMaya = False


def autoSearchMayaMainJoint():
	if not isRunningInMaya:
		sys.stderr.write('WARNING: the function autoSearchMayaMainJoint cannot be called, because maya is not available in the environment this script is running into')
		return

	skeletonGroup = 'C_grp_rig_0|C_grp_skeleton_0'
	if not cmds.objExists(skeletonGroup):
		raise Exception, 'object %s not present in scene. you can still passing by hand a valid maya\'s joint name' %skeletonGroup

	children = cmds.listRelatives(skeletonGroup, typ='joint', fullPath=True)
	if not children:
		raise Exception, 'a joint couldn\'t be found in C_grp_rig_0|C_grp_skeleton_0'
	else:
		return children[0]


def generateCrowdSkeletonFromMayaSkeleton(mainMayaJoint, shortNames=True):
	if not isRunningInMaya:
		sys.stderr.write('WARNING: the function generateCrowdSkeletonFromMayaSkeleton cannot be called, because maya is not available in the environment this script is running into')
		return

	data = {'parentIdsList' : IECore.IntVectorData(),
			'localMatList' : IECore.M44fVectorData(),
			'names' : IECore.StringVectorData()}

	def populateData(mayaJoint, jointId, parentId, data):
		data['parentIdsList'].append( parentId )
		data['names'].append(cmds.ls(mayaJoint, sn = shortNames)[0])

		data['localMatList'].append( IECore.M44f( cmds.getAttr(mayaJoint+'.m', t=0) ) )

		children = cmds.listRelatives(mayaJoint, allDescendents=False, fullPath=True, typ='joint')
		if children:
			for child in children:
				populateData(child, data['parentIdsList'].size(), jointId, data)

	if not mainMayaJoint:
		mainMayaJoint = autoSearchMayaMainJoint()
	populateData(mainMayaJoint, 0, -1, data)
	skeleton = IECore.SkeletonPrimitive(data['localMatList'], data['parentIdsList'], IECore.SkeletonPrimitive.Space.Local)
	skeleton.setJointNames(data['names'])

	skeleton.update()

	return skeleton


def saveSkeletonPrimitive(skelPrim, filePath):
	writer = IECore.Writer.create(skelPrim, filePath)
	writer.write()

def loadSkeletonPrimitive(filePath):
	reader = IECore.Reader.create(filePath)
	return reader.read()


def saveAnimationFromMaya(topJoint, fromFrame, toFrame, outputPath, fileName, toCache=True):
	if not isRunningInMaya:
		sys.stderr.write('WARNING: the function saveAnimationFromMaya cannot be called, because maya is not available in the environment this script is running into')
		return

	def populateData(mayaJoint, jointId, parentId, data, frame):
		data.append( IECore.M44f( cmds.getAttr(mayaJoint+'.m', t=frame) ) )
		children = cmds.listRelatives(mayaJoint, allDescendents=False, fullPath=True, typ='joint')
		if children:
			for child in children:
				populateData(child, data.size(), jointId, data, frame)

	for i in range(fromFrame, toFrame+1):
		cmds.currentTime(i)
		data = IECore.M44fVectorData()

		populateData(topJoint, 0, -1, data, i)

		# two possible file format, depends by the final use. maybe will be useful to expand cortex to provide one single file format that can be use
		# with caching and interpolation
		if toCache:
			ac = IECore.ObjectWriter(data, os.path.join(outputPath, fileName)+".%04d.anim" %i )
			ac.write( )
			del ac
		else:
			ac = IECore.AttributeCache( os.path.join(outputPath, fileName)+".%04d.anim" %ii, IECore.IndexedIOOpenMode.Write )
			ac.write( "our_object", "our_attribute", data )
			del ac



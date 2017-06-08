##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
import IECoreMaya
import maya.cmds as m
from maya.mel import eval as meval


def ieNodeSetup(node):
	''' setup a especific ieProceduralHolder node to be properly export by RMS '''
	preShapeAttrName = meval('rmanGetAttrName "preShapeScript"')

	for node in m.ls(node, dag=1, type='ieProceduralHolder'):
		attr = '%s.%s' % (node, preShapeAttrName)
		if not m.objExists( attr ):
			m.addAttr( node, ln=preShapeAttrName, dt="string" )
		m.setAttr( attr, '''python( "import IECoreMaya.ieRMS as ieRMS;reload(ieRMS);ieRMS.ieNodePreShape()" )''', type='string' )


def ieNodePreShape():
	'''
		mel code to use this function to export a cortex node with prman:

			python( "import IECoreMaya.ieRMS as ieRMS;reload(ieRMS);ieRMS.ieNodePreShape()" )

		put this code in the .rman__torattr___preShapeScript attribute in the ieProceduralHolder node
	'''

	# `rman ctxGetObject` retrieves the node name of the procedural so we can serialize!
	node = str( meval( "rman ctxGetObject" ) )
	print "CORTEX: Exporting node",node, "to prman."

	bb = IECore.Box3f( IECore.V3f( 999999 ), IECore.V3f( 999999 ) )
	proc = IECoreMaya.FromMayaDagNodeConverter.create( node ).convert()
	if proc:
		bb = proc.bound()

		serialize = IECore.ParameterParser().serialise( proc.parameters() )

		pythonString = "import IECoreRI;IECoreRI.executeProcedural( '%s', %s, %s )" % ( proc.path, proc.version, serialize )
		meval( 'RiProcedural "DynamicLoad" "iePython" %f %f %f %f %f %f "%s";'  % ( bb.min[0], bb.max[0], bb.min[1], bb.max[1], bb.min[2], bb.max[2], pythonString ) )


def iePreShapeMelSetup():
	''' setup renderManRISGlobals to properly export ieProceduralHolder nodes '''

	node = 'renderManRISGlobals'
	preShapeAttrName = meval('rmanGetAttrName "preShapeScript"')
	attr = '%s.%s' % (node, preShapeAttrName)

	if not m.objExists( attr ):
		m.addAttr( node, ln=preShapeAttrName, dt="string" )

	mel = '''python( "import IECoreMaya.ieRMS as ieRMS;reload(ieRMS);ieRMS.iePreShape()" )'''

	# do some cleanup if theres already some mel code in the attribute
	tmp = str( m.getAttr( attr ) ).replace( mel, '' ).strip().strip(';').strip()

	m.setAttr( attr, ';'.join([mel,tmp]).strip(';'), type='string' )


def iePreShape():
	'''
		We use this mel code in the renderManRISGlobals.rman__torattr___preShapeScript attribute to export all ieProceduralHolder nodes in the scene:

			python( "import IECoreMaya.ieRMS as ieRMS;reload(ieRMS);ieRMS.iePreShape()" )
	'''

	for node in m.ls( type='ieProceduralHolder', visible=1 ):
		# if the node already have preShapeMel code with ieNodePreShape in it, don't re-export it!
		preShapeAttrName = meval('rmanGetAttrName "preShapeScript"')
		attr = '%s.%s' % (node, preShapeAttrName)
		if m.objExists(attr) and 'ieNodePreShape' in str( m.getAttr( attr ) ):
			continue

		print "CORTEX: Exporting node",node, "to prman."

		bb = IECore.Box3f( IECore.V3f( 999999 ), IECore.V3f( 999999 ) )
		proc = IECoreMaya.FromMayaDagNodeConverter.create( str( node ) ).convert()
		bb = proc.bound()

		serialize = IECore.ParameterParser().serialise( proc.parameters() )

		pythonString = "import IECoreRI;IECoreRI.executeProcedural( '%s', %s, %s )" % ( proc.path, proc.version, serialize )
		meval( 'RiProcedural "DynamicLoad" "iePython" %f %f %f %f %f %f "%s";'  % ( bb.min[0], bb.max[0], bb.min[1], bb.max[1], bb.min[2], bb.max[2], pythonString ) )




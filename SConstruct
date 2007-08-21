##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

##########################################################################
#
# This SConstruct file is intended for use only with Image Engine's
# internal build module "IEBuild", and outside Image Engine is of use
# only as a reference.
#
##########################################################################

import os
import os.path
import sys
import glob

import VersionControl
import IEEnv

VersionControl.setVersion('IEBuild', '1.5.0')
import IEBuild

envRoot = IEEnv.Environment.rootPath()

# Names

coreName = "IECore"
corePythonName = 'IECore'
coreTestName = "test/IECoreTest"

# Versions

coreMajorVersion = '2'
coreMinorVersion = '9'
corePatchVersion = '0'
coreVersion = coreMajorVersion + "." + coreMinorVersion + "." + corePatchVersion
pythonVersion = '2.5'

# External dependencies

includes={}
includes['stlport']	= os.path.join( envRoot, 'tools/include/stlport/5.0.2/stlport' )
includes['boost']	= os.path.join( envRoot, 'tools/include/boost/1.33.1' )
includes['sqlite']	= os.path.join( envRoot, 'tools/include/sqlite/3.3.8' )
includes['openexr']	= os.path.join( envRoot, 'tools/include/OpenEXR/1.4.0' )
includes['openexrILMStyle']	= os.path.join( envRoot, 'tools/include/OpenEXR/1.4.0/OpenEXR' )
includes['tiff']	= os.path.join( envRoot, 'tools/include/tiff/3.8.2' )
includes['jpeg']	= os.path.join( envRoot, 'tools/include/jpeg/6b' )

libs={}
libs['boost'] = {}
libs['boost']['regex'] = 'boost_regex'
libs['boost']['filesystem'] = 'boost_filesystem'
libs['boost']['unit_test_framework'] = 'boost_unit_test_framework'
libs['boost']['python'] = 'boost_python'

multithreaded = ARGUMENTS.get('MULTITHREADED', 0)

if multithreaded:
	for k, v in libs['boost'].items():
		libs['boost'][k] = libs['boost'][k] + '-mt-p'

# Environment
env = Environment()
SConsignFile()

# Core library

coreLibName = coreName + "-" + coreVersion

coreSources = 		[ 
				'src/RefCounted.cpp',
				'src/RunTimeTyped.cpp',
				'src/Exception.cpp',
				'src/Object.cpp',
				'src/Data.cpp',
				'src/IndexedIO.cpp',
				'src/IndexedIOInterface.cpp',
				'src/IndexedIOFilter.cpp',
				'src/SQLiteIndexedIO.cpp', 
				'src/FileSystemIndexedIO.cpp',
				'src/FileIndexedIO.cpp',				
				'src/SimpleTypedData.cpp',
				'src/VectorTypedData.cpp',
				'src/CompoundData.cpp',
				'src/AttributeCache.cpp',
				'src/MessageHandler.cpp',
				'src/OStreamMessageHandler.cpp',
				'src/NullMessageHandler.cpp',	
				'src/FilteredMessageHandler.cpp',
				'src/LevelFilteredMessageHandler.cpp',
				'src/CompoundMessageHandler.cpp',
				'src/Reader.cpp',
				'src/ParticleReader.cpp',			
				'src/PDCParticleReader.cpp',
				'src/BlindDataHolder.cpp',
				'src/Renderable.cpp',
				'src/PrimitiveVariable.cpp',
				'src/Renderer.cpp',
				'src/Writer.cpp',
				'src/ParticleWriter.cpp',
				'src/PDCParticleWriter.cpp',
				'src/ByteOrder.cpp',
				'src/Parameter.cpp',
				'src/NumericParameter.cpp',
				'src/TypedParameter.cpp',
				'src/CompoundParameter.cpp',
				'src/CompoundObject.cpp',
				'src/ObjectReader.cpp',
				'src/ObjectWriter.cpp',
				'src/ValidatedStringParameter.cpp',
				'src/PathParameter.cpp',
				'src/DirNameParameter.cpp',
				'src/FileNameParameter.cpp',	
				'src/Primitive.cpp',
				'src/PointsPrimitive.cpp',				
				'src/Timer.cpp',
				'src/Shader.cpp',
				'src/SearchPath.cpp',
				'src/CachedReader.cpp',
				'src/NullObject.cpp',
				'src/ParameterisedInterface.cpp',
				'src/Parameterised.cpp',
				'src/Op.cpp',
				'src/ObjectParameter.cpp',
				'src/ModifyOp.cpp',
				'src/PrimitiveOp.cpp',
				'src/WrapperGarbageCollectorBase.cpp',
				'src/IndexedIOPath.cpp',				
				'src/ImagePrimitive.cpp',
				'src/ImageReader.cpp',
				'src/ImageWriter.cpp',
				'src/EXRImageReader.cpp',
				'src/EXRImageWriter.cpp',
				'src/TIFFImageReader.cpp',
		  	   	'src/TIFFImageWriter.cpp',
				'src/JPEGImageReader.cpp',
				'src/JPEGImageWriter.cpp',
				'src/CINImageReader.cpp',
		  	   	'src/CINImageWriter.cpp',
				'src/DPXImageReader.cpp',
				'src/BoxOperators.cpp',
				'src/MeshPrimitive.cpp',
				'src/MotionPrimitive.cpp',
				'src/Transform.cpp',
				'src/MatrixTransform.cpp',
				'src/Group.cpp',
				'src/AttributeState.cpp',
				'src/VisibleRenderable.cpp',
				'src/StateRenderable.cpp',
				'src/MatrixMotionTransform.cpp',
				'src/OBJReader.cpp',
				'src/IECore.cpp',			
		   ]
		
coreIncludes = 		[ 	
				includes['boost'],				
				includes['openexr'],
				includes['openexrILMStyle'],
				includes['sqlite'],
				includes['tiff'],
				includes['jpeg'],
				'include'				
			]
		
coreLibrary = IEBuild.SharedLibrary( ARGUMENTS, coreName, coreVersion, coreSources, coreIncludes)
if multithreaded:
	coreLibrary.addInclude( includes['stlport'] )
	coreLibrary.addLib('stlport')

coreLibrary.addLib( 'sqlite3' )
coreLibrary.addLib( libs['boost']['regex'] )
coreLibrary.addLib( libs['boost']['filesystem'] )
coreLibrary.addLib( 'Iex' )
coreLibrary.addLib( 'Imath' )
coreLibrary.addDefine( "IE_CORE_MAJORVERSION", coreMajorVersion )
coreLibrary.addDefine( "IE_CORE_MINORVERSION", coreMinorVersion )
coreLibrary.addDefine( "IE_CORE_PATCHVERSION", corePatchVersion )

# image library dependencies
coreLibrary.addLib('IlmImf')
coreLibrary.addLib('Half')
coreLibrary.addLib('IlmThread')
coreLibrary.addLib('tiff')
coreLibrary.addLib('jpeg')

coreLibrary.addLinkFlag( '-pthread' )

# Core library API documentation		
coreLibrary.doxygen('doc/config/Doxyfile', 'doc/config/header.html', 'doc/config/footer.html')
	
coreLibrary.finalize()	

core = coreLibrary.getTargets()

# Core library python binding

corePythonSources = 	[	
				'src/bindings/IECore.cpp',
				'src/bindings/RefCountedBinding.cpp',
				'src/bindings/RunTimeTypedBinding.cpp',
				'src/bindings/ExceptionBinding.cpp',
				'src/bindings/ImathBinding.cpp',
				'src/bindings/ImathVecBinding.cpp',
				'src/bindings/ImathBoxBinding.cpp',
				'src/bindings/ImathQuatBinding.cpp',
				'src/bindings/ImathMatrixBinding.cpp',
				'src/bindings/ImathColorBinding.cpp',
				'src/bindings/KDTreeBinding.cpp',
				'src/bindings/IndexedIOInterfaceBinding.cpp',
				'src/bindings/DataBinding.cpp',
				'src/bindings/SimpleTypedDataBinding.cpp',
				'src/bindings/VectorTypedDataBinding.cpp',
				'src/bindings/ImathMatrixVectorBinding.cpp',
				'src/bindings/ImathVecVectorBinding.cpp',
				'src/bindings/ImathColorVectorBinding.cpp',
				'src/bindings/ImathBoxVectorBinding.cpp',
				'src/bindings/ImathQuatVectorBinding.cpp',
				'src/bindings/CompoundDataBinding.cpp',
				'src/bindings/ObjectBinding.cpp',
				'src/bindings/TypeIdBinding.cpp',
				'src/bindings/AttributeCacheBinding.cpp',
				'src/bindings/MessageHandlerBinding.cpp',
				'src/bindings/ReaderBinding.cpp',
				'src/bindings/ParticleReaderBinding.cpp',
				'src/bindings/PDCParticleReaderBinding.cpp',
				'src/bindings/BlindDataHolderBinding.cpp',
				'src/bindings/RenderableBinding.cpp',				
				'src/bindings/RendererBinding.cpp',
				'src/bindings/PrimitiveVariableBinding.cpp',
				'src/bindings/WriterBinding.cpp',
				'src/bindings/ParticleWriterBinding.cpp',
				'src/bindings/ParameterBinding.cpp',		
				'src/bindings/NumericParameterBinding.cpp',		
				'src/bindings/TypedParameterBinding.cpp',		
				'src/bindings/CompoundParameterBinding.cpp',		
				'src/bindings/ValidatedStringParameterBinding.cpp',		
				'src/bindings/PathParameterBinding.cpp',		
				'src/bindings/FileNameParameterBinding.cpp',		
				'src/bindings/DirNameParameterBinding.cpp',		
				'src/bindings/CompoundObjectBinding.cpp',		
				'src/bindings/PDCParticleWriterBinding.cpp',
				'src/bindings/ObjectReaderBinding.cpp',				
				'src/bindings/ObjectWriterBinding.cpp',				
				'src/bindings/PrimitiveBinding.cpp',				
				'src/bindings/PointsPrimitiveBinding.cpp',				
				'src/bindings/ImagePrimitiveBinding.cpp',
				'src/bindings/ImageReaderBinding.cpp',
				'src/bindings/ImageWriterBinding.cpp',
				'src/bindings/EXRImageReaderBinding.cpp',
				'src/bindings/EXRImageWriterBinding.cpp',
				'src/bindings/HalfBinding.cpp',
				'src/bindings/TimerBinding.cpp',
				'src/bindings/TIFFImageReaderBinding.cpp',
				'src/bindings/TIFFImageWriterBinding.cpp',
				'src/bindings/CINImageReaderBinding.cpp',
				'src/bindings/CINImageWriterBinding.cpp',
				'src/bindings/DPXImageReaderBinding.cpp',
				'src/bindings/PerlinNoiseBinding.cpp',
				'src/bindings/JPEGImageReaderBinding.cpp',
				'src/bindings/JPEGImageWriterBinding.cpp',
				'src/bindings/TurbulenceBinding.cpp',
				'src/bindings/MeshPrimitiveBinding.cpp',
				'src/bindings/ShaderBinding.cpp',
				'src/bindings/SearchPathBinding.cpp',
				'src/bindings/CachedReaderBinding.cpp',
				'src/bindings/ParameterisedBinding.cpp',
				'src/bindings/OpBinding.cpp',
				'src/bindings/ObjectParameterBinding.cpp',
				'src/bindings/ModifyOpBinding.cpp',
				'src/bindings/PrimitiveOpBinding.cpp',
				'src/bindings/MotionPrimitiveBinding.cpp',
				'src/bindings/TransformBinding.cpp',
				'src/bindings/MatrixTransformBinding.cpp',
				'src/bindings/GroupBinding.cpp',
				'src/bindings/AttributeStateBinding.cpp',
				'src/bindings/VisibleRenderableBinding.cpp',
				'src/bindings/StateRenderableBinding.cpp',
				'src/bindings/MatrixMotionTransformBinding.cpp',
				'src/bindings/OBJReaderBinding.cpp',
				'src/bindings/NullObjectBinding.cpp',
			]
			
corePythonPy = 	[ 			
				'python/IECore/ClassLoader.py',
				'python/IECore/RemoteOpLoader.py',
				'python/IECore/DataTraits.py',
				'python/IECore/FormattedParameterHelp.py',
				'python/IECore/Formatter.py',
				'python/IECore/__init__.py',
				'python/IECore/Log.py',
				'python/IECore/ParameterParser.py',
				'python/IECore/StringUtil.py',
				'python/IECore/WrappedTextFormatter.py',
				'python/IECore/CompoundFrameList.py',
				'python/IECore/EmptyFrameList.py',
				'python/IECore/FileSequenceFunctions.py',
				'python/IECore/FileSequence.py',
				'python/IECore/FrameRange.py',
				'python/IECore/FrameList.py',
				'python/IECore/ObjectOverwriting.py',
				'python/IECore/OpOverwriting.py',
				'python/IECore/ParameterOverwriting.py',
				'python/IECore/ParameterisedOverwriting.py',
				'python/IECore/RunTimeTypedUtil.py',				
				'python/IECore/FileSequenceParameter.py',				
				'python/IECore/ConfigLoader.py',
				'python/IECore/RemovePrimitiveVariables.py',				
				'python/IECore/RenamePrimitiveVariables.py',				
				'python/IECore/SequenceLsOp.py',				
				'python/IECore/SequenceRmOp.py',				
				'python/IECore/SequenceCpOp.py',				
				'python/IECore/SequenceRenumberOp.py',				
				'python/IECore/SequenceMvOp.py',				
				'python/IECore/ReadProcedural.py',				
				'python/IECore/ClassLsOp.py',		
				'python/IECore/OptionalCompoundParameter.py',		
				'python/IECore/FileDependenciesOp.py',		
				'python/IECore/FileExaminer.py',		
				'python/IECore/NukeFileExaminer.py',		
				'python/IECore/RIBFileExaminer.py',		
				'python/IECore/CheckFileDependenciesOp.py',		
			]


corePythonIncludes = 	coreIncludes
								
corePythonModule = IEBuild.PythonModule( ARGUMENTS, 'IECore', coreVersion, corePythonSources, corePythonIncludes, corePythonPy, "python/IECore")
corePythonModule.setPythonVersion('2.5')

#Override default python compiler
corePythonModule.getLibrary().setCompiler()

corePythonModule.getLibrary().addLibPath('.')
corePythonModule.getLibrary().addLib(coreLibName)
corePythonModule.getLibrary().addLib(libs['boost']['python'])
corePythonModule.getLibrary().addLibs( coreLibrary.getLibs() )

corePythonModule.finalize()

corePython = corePythonModule.getTargets()
env.Depends( corePython, core )

do = env.Command( "scripts/do", "scripts/do.in", [ "sed 's:!CORE_VERSION!:"+coreMajorVersion+":' < $SOURCE > $TARGET", Chmod( "$TARGET", 0755 ) ] )
doInstall = env.InstallAs( os.path.join( corePythonModule.getInstallPrefix(), "apps", "do", coreMajorVersion, "do" ), do )

env.Alias( "install", [ doInstall ] )

# op wrappers

coreOps = [
	[ "SequenceLsOp", "seqLs", "common/fileSystem" ],
	[ "SequenceCpOp", "seqCp", "common/fileSystem" ],
	[ "SequenceMvOp", "seqMv", "common/fileSystem" ],
	[ "SequenceRmOp", "seqRm", "common/fileSystem" ],
	[ "SequenceRenumberOp", "seqRenumber", "common/fileSystem" ],
	[ "RemovePrimitiveVariables", "removeVariables", "common/primitive" ],
	[ "RenamePrimitiveVariables", "renameVariables", "common/primitive" ],
	[ "ClassLsOp", "classLs", "common/classes" ],
	[ "FileDependenciesOp", "depLs", "common/fileSystem" ],
	[ "CheckFileDependenciesOp", "depCheck", "common/fileSystem" ],
]


for op in coreOps :

	IEBuild.OpWrapper( ARGUMENTS, op[0], "IECore", coreVersion, op[2], op[1], coreMajorVersion ).finalize()

# procedural wrappers

coreProcedurals = [
	[ "ReadProcedural", "read", "" ],
]

for procedural in coreProcedurals :

	IEBuild.ProceduralWrapper( ARGUMENTS, procedural[0], "IECore", coreVersion, procedural[2], procedural[1], coreMajorVersion ).finalize()
	
# Core library unit test

coreTestSources = 	[ 
				'test/KDTreeTest.cpp',
				'test/IECoreTest.cpp',
				'test/TypedDataTest.cpp',
				'test/InterpolatorTest.cpp',
				'test/IndexedIOTest.cpp',
				'test/BoostUnitTestTest.cpp',				
			]

coreTestProgram = IEBuild.Program( ARGUMENTS, coreTestName, coreTestSources, coreIncludes )
coreTestProgram.addLibPath('.')
coreTestProgram.addLibs( coreLibrary.getLibs() )
coreTestProgram.addLib( coreLibName )
coreTestProgram.addLib( libs['boost']['unit_test_framework'] )
coreTestProgramEnv = coreTestProgram.finalize()

coreTest = coreTestProgram.getTargets()

coreTestLibPath = coreTestProgramEnv["LIBPATH"]


if IEEnv.platform().startswith("osx104"):
	coreTestRun = env.Command('test/IECoreTest.out', '', 'env DYLD_LIBRARY_PATH=' + ":".join(coreTestLibPath) + ' ' +coreTestName )
else:	
	coreTestRun = env.Command('test/IECoreTest.out', '', 'env LD_LIBRARY_PATH=' + ":".join(coreTestLibPath) + ' ' +coreTestName )
env.Depends( coreTestRun, coreTest )

pythonTestEnv = Environment( ENV=os.environ )
pythonTestEnv["ENV"]["PYTHONPATH"] = "python:" + pythonTestEnv["ENV"]["PYTHONPATH"]
pythonTestEnv["ENV"]["IEENV_LIBRARY_PREFIX_PATH"] = "./:"+pythonTestEnv["ENV"]["IEENV_LIBRARY_PREFIX_PATH"]
pythonTestEnv["ENV"]["COMPILER"] = corePythonModule.getLibrary().getCompiler()
pythonTestEnv["ENV"]["COMPILER_VERSION"] = corePythonModule.getLibrary().getCompilerVersion()

corePythonTestRun = pythonTestEnv.Command( "test/IECorePythonTest.out", "",  "python"+pythonVersion+" "+ARGUMENTS.get("TESTSCRIPT", "test/All.py") + " -v")
pythonTestEnv.Depends( corePythonTestRun, corePython )

env.Alias('test', [
			corePythonTestRun, 
			coreTestRun
		  ] )
		  
env.Alias('all', [core, corePython, do] )
  
Default('all')


cortexLocation=/development/software/cortex/linux.centos5.x86_64/7.0.0
export PYTHONPATH=$cortexLocation/python/2.6:$PYTHONPATH
# MUST NOT SET PYTHONHOME FOR MAYA!
#export PYTHONHOME=/development/software/python/linux.centos6.x86_64/2.6.4
export LD_LIBRARY_PATH=$cortexLocation/lib:/development/software/boost/linux.centos5.x86_64/1.47/lib:/development/software/tbb/linux.centos5.x86_64/3.0.221/lib:$PYTHONHOME/lib:$LD_LIBRARY_PATH
export IECORE_PROCEDURAL_PATHS=/disk1/playpen/dev/cortexExamples/procedurals
export IECORE_OP_PATHS=/disk1/playpen/dev/cortexExamples/ops
export CORTEX_POINTDISTRIBUTION_TILESET=/disk1/playpen/dev/cortex/tileset_2048.dat
export PATH=$cortexLocation/scripts:$PATH
export MAYA_PLUG_IN_PATH=$cortexLocation/maya/2012/plugins:$MAYA_PLUG_IN_PATH
export MAYA_SCRIPT_PATH=$cortexLocation/maya/2012/mel:$MAYA_SCRIPT_PATH


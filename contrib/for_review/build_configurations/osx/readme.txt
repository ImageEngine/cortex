This is an options file to build cortex with maya 2011 on osx 10.6.
This is tested with maya2011 and 3Delight-9.0.105

Addionally there is a patch for the SConcstruct to fix
the references of the installed libraries by
inspection with otool -L and changing references with install_name_tool

To build in cortex root:

1. $ patch -p0 < contrib/for_review/build_configurations/osx/SConstruct.patch

2. $ cp contrib/for_review/build_configurations/osx/options-maya2011.txt .

3. Edit options-maya2011.txt and adapt the installed locations of the required software:

  * Install boost 1.42 to /opt/boost_1_42_0 (or some other location, but then you need to change BOOST_INCLUDE_PATH and BOOST_LIB_PATH)

  * Install TBB 2.2 to /opt/tbb_2.2 (or some other location, but then you need to change the TBB_INCLUDE_PATH and TBB_LIB_PATH )

  * Change RMAN_ROOT in the options file to the location where 3Delight is installed.

2. Build cortex
   
   $ scons OPTIONS=options-maya2011.txt

3. Install cortex, if you do not change INSTALL_PREFIX it will be installed to /opt/cortex/maya

   $ sudo scons OPTIONS=options-maya2011.txt install

4. Add cortex python module to Maya. The easiest way is to create a .pth file.
  
  * Create The file /Applications/Autodesk/maya2011/Maya.app/Contents/Frameworks/Python.framework/Versions/2.6/lib/python2.6/site-packages/cortex.pth
  * Enter the path to you cortex installation, if you haven't changed the INSTALL_PREFIX option this will be 
    
    /opt/cortex/maya/lib/python2.6/site-packages


5. Locate your Maya.env file  (should be ~/Library/Preferences/Autodesk/maya/2011-x64/Maya.env ) and set 

  MAYA_PLUG_IN_PATH=/opt/cortex/maya/maya/plugins
  MAYA_SCRIPT_PATH=/opt/cortex/maya/maya/mel
  MAYA_ICON_PATH=/opt/cortex/maya/maya/icons   
  IECORE_OP_PATHS=/opt/cortex/maya/ops                  
  IECORE_PROCEDURAL_PATHS=/path/to/your/procedurals

  Adapt again MAYA_PLUG_IN_PATH, MAYA_SCRIPT_PATH and MAYA_ICON_PATH to your INSTALL_PREFIX
  Set IECORE_PROCEDURAL_PATHS to the location where you want to keep your Cortex Procedurals


6. There is a problem with exposed boost symbols in Maya 2011 which results in a crash of Maya under some conditions.
   The workaround is to launch Maya from the terminal and to preload libboost_regex.dylib

   $ export DYLD_INSERT_LIBRARIES=/opt/cortex/maya/lib/libboost_regex.dylib

   $ export LC_ALL="C"
   Note: 
   If Maya does not start properly and you get an error like 
   "/Applications/Autodesk/maya2011/Maya.app/Contents/scripts/startup/layerEditor.mel line 268: Value must be greater than 0"
   you must also set your locale properly by running this command (i.e. on non-english systems)

   $ /Applications/Autodesk/maya2011/Maya.app/Contents/MacOS/Maya


7. Load the Cortex Plugin with the Plug-in Manager (should be listed there)

8. A new Cortex Menu should appear that lists your installed Ops and Procedurals


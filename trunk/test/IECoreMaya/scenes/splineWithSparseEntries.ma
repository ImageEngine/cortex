//Maya ASCII 2009 scene
//Name: splineParameterHandlerTest.ma
//Last modified: Thu, Dec 03, 2009 12:32:33 PM
//Codeset: ANSI_X3.4-1968
requires maya "2009";
requires "ieCore" "1.0";
currentUnit -l centimeter -a degree -t film;
fileInfo "application" "maya";
fileInfo "product" "Maya Render 2009";
fileInfo "version" "2009 x64";
fileInfo "cutIdentifier" "200809110001-734661";
fileInfo "osv" "Linux 2.6.18-53.el5 #1 SMP Mon Nov 12 02:14:55 EST 2007 x86_64";
createNode transform -s -n "persp";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 28 21 28 ;
	setAttr ".r" -type "double3" -27.938352729602379 44.999999999999972 -5.172681101354183e-14 ;
createNode camera -s -n "perspShape" -p "persp";
	setAttr -k off ".v" no;
	setAttr ".fl" 34.999999999999993;
	setAttr ".coi" 44.82186966202994;
	setAttr ".imn" -type "string" "persp";
	setAttr ".den" -type "string" "persp_depth";
	setAttr ".man" -type "string" "persp_mask";
	setAttr ".hc" -type "string" "viewSet -p %camera";
createNode transform -s -n "top";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 100.1 0 ;
	setAttr ".r" -type "double3" -89.999999999999986 0 0 ;
createNode camera -s -n "topShape" -p "top";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "top";
	setAttr ".den" -type "string" "top_depth";
	setAttr ".man" -type "string" "top_mask";
	setAttr ".hc" -type "string" "viewSet -t %camera";
	setAttr ".o" yes;
createNode transform -s -n "front";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 0 100.1 ;
createNode camera -s -n "frontShape" -p "front";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "front";
	setAttr ".den" -type "string" "front_depth";
	setAttr ".man" -type "string" "front_mask";
	setAttr ".hc" -type "string" "viewSet -f %camera";
	setAttr ".o" yes;
createNode transform -s -n "side";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 100.1 0 0 ;
	setAttr ".r" -type "double3" 0 89.999999999999986 0 ;
createNode camera -s -n "sideShape" -p "side";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "side";
	setAttr ".den" -type "string" "side_depth";
	setAttr ".man" -type "string" "side_mask";
	setAttr ".hc" -type "string" "viewSet -s %camera";
	setAttr ".o" yes;
createNode lightLinker -n "lightLinker1";
	setAttr -s 2 ".lnk";
	setAttr -s 2 ".slnk";
createNode displayLayerManager -n "layerManager";
createNode displayLayer -n "defaultLayer";
createNode renderLayerManager -n "renderLayerManager";
createNode renderLayer -n "defaultRenderLayer";
	setAttr ".g" yes;
createNode ieOpHolderNode -n "test";
	addAttr -ci true -m -sn "parm_spline" -ln "parm_spline" -bt "UNKN" -at "compound" 
		-nc 3;
	addAttr -ci true -sn "parm_splinep" -ln "parm_spline_Position" -bt "UNKN" -at "float" 
		-p "parm_spline";
	addAttr -ci true -sn "parm_splinefv" -ln "parm_spline_FloatValue" -bt "UNKN" -at "float" 
		-p "parm_spline";
	addAttr -ci true -sn "parm_splinei" -ln "parm_spline_Interp" -bt "UNKN" -dv 1 -min 
		0 -max 3 -en "None:Linear:Smooth:Spline" -at "enum" -p "parm_spline";
	addAttr -ci true -k true -sn "result" -ln "result" -bt "UNKN" -min -2147483648 -max 
		2147483647 -at "long";
	setAttr ".cch" no;
	setAttr ".ihi" 2;
	setAttr ".nds" 0;
	setAttr ".clas" -type "string" "splineInput";
	setAttr ".ver" 1;
	setAttr ".spev" -type "string" "IECORE_OP_PATHS";
	
	setAttr -s 7 ".parm_spline";
	setAttr ".parm_spline[0:3]" 0 0.4166666567325592 3 1 0.3684210479259491 
		3 0.089108914136886597 0.56578946113586426 3 0.15807560086250305 0.32017543911933899 
		3;
	setAttr ".parm_spline[5:7]" 0.84536081552505493 0.23684211075305939 
		3 0.22852233052253723 0.2850877046585083 3 0.48969072103500366 0.24561403691768646 
		3;
		
createNode script -n "uiConfigurationScriptNode";
	setAttr ".b" -type "string" "// Maya Mel UI Configuration File.\n// No UI generated in batch mode.\n";
	setAttr ".st" 3;
createNode script -n "sceneConfigurationScriptNode";
	setAttr ".b" -type "string" "playbackOptions -min 1 -max 24 -ast 1 -aet 48 ";
	setAttr ".st" 6;
select -ne :time1;
	setAttr ".o" 1;
select -ne :renderPartition;
	setAttr -s 2 ".st";
select -ne :renderGlobalsList1;
select -ne :defaultShaderList1;
	setAttr -s 2 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :lightList1;
select -ne :initialShadingGroup;
	setAttr ".ro" yes;
select -ne :initialParticleSE;
	setAttr ".ro" yes;
select -ne :hardwareRenderGlobals;
	setAttr ".ctrs" 256;
	setAttr ".btrs" 512;
select -ne :defaultHardwareRenderGlobals;
	setAttr ".fn" -type "string" "im";
	setAttr ".res" -type "string" "ntsc_4d 646 485 1.333";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[0].llnk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.lnk[0].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[1].llnk";
connectAttr ":initialParticleSE.msg" "lightLinker1.lnk[1].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.slnk[0].sllk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.slnk[0].solk";
connectAttr ":defaultLightSet.msg" "lightLinker1.slnk[1].sllk";
connectAttr ":initialParticleSE.msg" "lightLinker1.slnk[1].solk";
connectAttr "layerManager.dli[0]" "defaultLayer.id";
connectAttr "renderLayerManager.rlmi[0]" "defaultRenderLayer.rlid";
connectAttr "lightLinker1.msg" ":lightList1.ln" -na;
// End of splineParameterHandlerTest.ma

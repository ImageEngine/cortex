import sys
import os
import shutil
import subprocess

if sys.version_info[0] < 3 :
	from urllib import urlretrieve
else :
	from urllib.request import urlretrieve

delightVersion="1.1.12"
delightDirectory="free/beta/2018-11-01-oIDoJTpO"

baseUrl = "https://3delight-downloads.s3-us-east-2.amazonaws.com"

if sys.platform.startswith("linux") :  # pre Python 3.3 the major version is added at the end
	package="3DelightNSI-{}-Linux-x86_64".format( delightVersion )

	url = "{baseUrl}/{delightDirectory}/{package}.tar.xz".format(
		baseUrl = baseUrl,
		delightDirectory = delightDirectory,
		package = package
	)

	print( "Downloading 3Delight \"{}\"".format( url ) )
	archiveFileName, headers = urlretrieve( url )

	exitStatus = os.system( "tar -xf {} -C .".format( archiveFileName ) )
	if exitStatus != 0 :
		exit( exitStatus )

	shutil.copytree( "./{}/3delight/Linux-x86_64".format( package ), "./3delight" )

elif sys.platform == "darwin" :
	package="3DelightNSI-{}-Darwin-Universal".format( delightVersion )

	url = "{baseUrl}/{delightDirectory}/{package}.dmg".format(
		baseUrl = baseUrl,
		delightDirectory = delightDirectory,
		package = package
	)

	print( "Downloading 3Delight \"{}\"".format( url ) )
	archiveFileName, headers = urlretrieve( url )

	subprocess.check_call(
		[
			"sudo",
			"hdiutil",
			"mount",
			archiveFileName,
		]
	)
	subprocess.check_call(
		[
			"sudo",
			"installer",
			"-pkg",
			"/Volumes/3Delight NSI "
			"{delightVersion}/3DelightNSI-{delightVersion}-Darwin-x86_64.pkg".format(
				delightVersion = delightVersion
			),
			"-target",
			"/",
		]
	)
	subprocess.check_call(
		[
			"sudo",
			"mv",
			"/Applications/3Delight",
			"./3delight",
		]
	)


elif sys.platform == "win32":
	print( "3Delight on Windows is not currently supported." )


Compile cortex on windows
When compiling cortex's IECore,IECoreGL,IECoreMaya lib, you will recieve some errors like 'def file not file'.
After compiling cpp files and right before link, you can open visual studio command line product.
1.go to $(ProjectDir)x64/Release
2.run python picknames.py. This will generate the def file
3.build (not Rebuild)
Other lib needn't do that!
Have fun!

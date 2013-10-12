import os
import subprocess

# Generate DEF/DLL files for Windows
def DllDef_File(source):
    objs=source
    library_name="IECore"
    print "Generating DEF file for %s"%library_name
    # use dumpbin to figure out what to export
    dumpbin="dumpbin"

    allobjs = ""
    for obj in objs:
        allobjs = allobjs + " "+obj
    cmd="%s /symbols %s"%(dumpbin,allobjs)
    print cmd
    #sp=subprocess.Popen([dumpbin,'/symbols',allobjs],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    #fp,err = sp.communicate()
    #print fp
    fp=os.popen(cmd)
    defs={}
    while 1:
        line=fp.readline()
        if line=="": break
        line=line.replace("\r","").replace("\n","")
        if line.find("External")!=-1:
            left,right=line.split(" | ")
            if left.find("External")==-1: continue # one more check!
            if left.find("UNDEF")!=-1: continue
            mangled_symbol=right.split(" ")[0]
            if mangled_symbol.find("@IECore")!=-1: # only export PhysBAM namespace symbols
                defs[mangled_symbol]=True
            #if mangled_symbol.find("?convert@IECore@@YAXAEBV?$Box@V?$Vec3@M@Imath@@@Imath@@QEAM@Z")!=-1: # only export PhysBAM namespace symbols
            #    defs[mangled_symbol]=True
    print "%s has %d symbols exported"%(library_name,len(defs.keys()))
    deffp=open('IECore.def',"w")
    deffp.write("LIBRARY %s\n"%(library_name))
    deffp.write("EXPORTS\n")
    for item in defs.keys():
        deffp.write(item+"\n")
    deffp.close()
    return None

dict = []
for d, fd, fl in os.walk('./'):
    for f in fl:
        sufix = os.path.splitext(f)[1][1:]
        if sufix=='obj':
            dict.append(f)
DllDef_File(dict)

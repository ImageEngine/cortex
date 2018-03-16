# CMake

This is an alternative build system for Cortex which is placed here unsupported.
It is recommended to use the SCons build system on Linux/macOS, but this could be used to build a Windows version.
Currently IECoreMaya, IECoreHoudini and IECoreNuke are not supported.


## Instructions developing the CMake Build scripts

You can use the gaffer [dependencies project](https://github.com/GafferHQ/dependencies/releases) for all the dependant libraries & headers.



If you use bash:

**Create directory for gaffer development**

~~~
mkdir dev-cortex
cd dev-cortex
~~~

**Download Gaffer Dependencies**
~~~
wget https://github.com/GafferHQ/dependencies/releases/download/0.45.0.0/gafferDependencies-0.45.0.0-linux.tar.gz
~~~

**Extract**
~~~
tar xvf gafferDependencies-0.45.0.0-linux.tar.gz
~~~

**Set root env var for build**
```
export GAFFER_DEPENDENCIES_ROOT=$PWD/gafferDependencies-0.45.0.0-linux
export ILMBASE_ROOT=$GAFFER_DEPENDENCIES_ROOT
export OPENEXR_ROOT=$GAFFER_DEPENDENCIES_ROOT
export FREETYPE_DIR=$GAFFER_DEPENDENCIES_ROOT
```

**Clone git repo and create a directory called .build**

~~~
git clone https://github.com/ImageEngine/cortex.git
cd cortex
mkdir .build
cd .build
~~~


**Configure**
```
cmake ../contrib/cmake
-DTBB_INSTALL_DIR=$GAFFER_DEPENDENCIES_ROOT
-DBOOST_ROOT=$GAFFER_DEPENDENCIES_ROOT
-DILMBASE_ROOT=$GAFFER_DEPENDENCIES_ROOT
-DOPENEXR_ROOT=$GAFFER_DEPENDENCIES_ROOT
-DBLOSC_LOCATION=$GAFFER_DEPENDENCIES_ROOT
-DCMAKE_PREFIX_PATH=$GAFFER_DEPENDENCIES_ROOT
-DILMBASE_NAMESPACE_VERSIONING=OFF
-DOPENEXR_NAMESPACE_VERSIONING=OFF
-DWITH_IECORE_IMAGE=ON
-DWITH_IECORE_SCENE=ON
-DWITH_IECORE_ALEMBIC=ON
-DWITH_IECORE_USD=OFF
-DCMAKE_INSTALL_PREFIX=$GAFFER_DEPENDENCIES_ROOT
```

**Build**
```
make install -j4
```

### Build Flags

- WITH_IECORE_PYTHON=1 : Build IECorePython
- WITH_IECORE_GL=1 : Build IECoreGL
- WITH_IECORE_IMAGE=1 : Build IECoreImage
- WITH_IECORE_SCENE=1 : Build IECoreScene
- WITH_IECORE_ARNOLD=1 : Build IECoreArnold
- WITH_IECORE_ALEMBIC=1 : Build IECoreAlembic
- WITH_IECORE_APPLESEED=1 : Build IECoreAppleseed
- WITH_IECORE_USD=1 : Build IECoreUSD

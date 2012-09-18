#==========
#
# Copyright (c) 2010, Dan Bethell.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# * Neither the name of Dan Bethell nor the names of any
# other contributors to this software may be used to endorse or
# promote products derived from this software without specific prior
# written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#==========
#
# Variables defined by this module:
# Cortex_FOUND
# Cortex_INCLUDE_DIR
# Cortex_LIBRARIES
#
# Usage:
# FIND_PACKAGE( Cortex )
# FIND_PACKAGE( Cortex REQUIRED )
#
# Todo: add support for finding additional components
# (more than just IECore) using a mechanism similar to Boost's
# cmake module.
#
#==========

# try to find header
FIND_PATH( Cortex_INCLUDE_DIR IECore/IECore.h
  ${Cortex_INSTALL_PATH}/include
  $ENV{CORTEX_ROOT}/include
  )

# try to find libs
FIND_LIBRARY( Cortex_LIBRARY IECore
  ${Cortex_INSTALL_PATH}/lib
  $ENV{CORTEX_ROOT}/lib
  )

# did we find everything?
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "Cortex" DEFAULT_MSG
  Cortex_INCLUDE_DIR
  Cortex_LIBRARY
  )

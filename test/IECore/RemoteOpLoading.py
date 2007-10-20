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

import os
import unittest
from IECore import *

#export IECORE_MAJOR_VERSION="2"
# "fetch" a certificate - note the expire time if you are trying to develop with this...
certificate = '<certificate expire-time="2007-05-29T13:57:47.918640" id="1" key="83f84f973e7426ede7b9687382c0632d" login-time="2007-05-28T13:57:47.918640" uid="501" user="blair"/>'

class testRemoteOpLoader(unittest.TestCase):


    def testListOps(self):
        
        r = RemoteOpLoader.defaultRemoteOpLoader('http://10.20.30.10:8010', certificate)
        
        # list the jobs
        lsjobs = r.load('common/fileSystem/lsJobs')()
        result = lsjobs.operate()
        
    def testRootPrivilegesRequiredOp(self):

        r = RemoteOpLoader.defaultRemoteOpLoader('http://10.20.30.10:8010', certificate)

        # make a job with code 'EXA'.  this requires root privileges.
        mkjob = r.load('common/fileSystem/mkJob')()
        ps = mkjob.parameters()
        ps['job'].setTypedValue('EXA')
        try:
            mkjob.operate()

        except Exception, e:
            # expect an exception to be thrown
            pass
                
                
if __name__ == "__main__":
        unittest.main()   

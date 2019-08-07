##########################################################################
#
#  Copyright (c) 2019, Hypothetical Inc. All rights reserved.
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

import re
import os

re_matches = {
    "majorVersion": re.compile(r"ieCoreMajorVersion=([0-9]*)"),
    "minorVersion": re.compile(r"ieCoreMinorVersion=([0-9]*)"),
    "patchVersion": re.compile(r"ieCorePatchVersion=([0-9]*)"),
    "versionSuffix": re.compile(r"ieCoreVersionSuffix=\"?([A-Za-z0-9]*)\"?")
}

versionInfo = {}

with open("SConstruct") as infile:
    for line in infile:
        for key in re_matches.keys():
            match = re_matches[key].search(line)
            if match is not None:
                versionInfo[key] = str(match.groups()[0])

with open("version.txt", "w") as outfile:
    outfile.write("{}.{}.{}.{}".format(
        versionInfo.get("majorVersion", "0"),
        versionInfo.get("minorVersion", "0"),
        versionInfo.get("patchVersion", "0"),
        versionInfo.get("versionSuffix", "0")
    ))

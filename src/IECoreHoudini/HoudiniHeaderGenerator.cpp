//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "IECoreHoudini/CoreHoudini.h"

#include "IECore/CompoundData.h"
#include "IECore/HeaderGenerator.h"
#include "IECore/SimpleTypedData.h"

#include "MOT/MOT_Director.h"
#include "OP/OP_Director.h"
#include "UT/UT_Version.h"

using namespace IECore;

namespace IECoreHoudini
{

static void houdiniHeaderGenerator( CompoundObjectPtr header )
{
	CH_Manager *channelManager = OPgetDirector()->getChannelManager();
	MOT_Director *motDirector = dynamic_cast<MOT_Director *>( OPgetDirector() );

	CompoundDataPtr compound = new CompoundData();
	compound->writable()["houdiniVersion"] = new StringData( UTgetFullVersion() );
	compound->writable()["sceneFile"] = new StringData( motDirector->getFileName().toStdString() );
	compound->writable()["currentTime"] = new FloatData( channelManager->getSampleRaw( CHgetEvalTime() ) );
	compound->writable()["minTime"] = new FloatData( channelManager->getGlobalStartFrame() );
	compound->writable()["maxTime"] = new FloatData( channelManager->getGlobalEndFrame() );
	compound->writable()["frameRate"] = new FloatData( channelManager->getSamplesPerSec() );

	header->members()["houdini"] = compound;
}

static bool resIeCoreHoudini = HeaderGenerator::registerDataHeaderGenerator( &houdiniHeaderGenerator );

}

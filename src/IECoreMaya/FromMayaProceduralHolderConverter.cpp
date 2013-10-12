//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore\CompoundObject.h"
#include "IECoreMaya/FromMayaProceduralHolderConverter.h"
#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "maya/MFnDagNode.h"
#include "maya/MDagPath.h"

using namespace IECoreMaya;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( FromMayaProceduralHolderConverter );

FromMayaDagNodeConverter::Description<FromMayaProceduralHolderConverter> FromMayaProceduralHolderConverter::m_description( MTypeId( ProceduralHolderId ), ParameterisedProceduralTypeId, true );

FromMayaProceduralHolderConverter::FromMayaProceduralHolderConverter( const MDagPath &dagPath )
	:	FromMayaDagNodeConverter( "Converts maya procedural holder into an IECore::ParameterisedProcedural object.", dagPath )
{
}

IECore::ObjectPtr FromMayaProceduralHolderConverter::doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnDagNode nodeFn( dagPath );
	
	ProceduralHolder* node = dynamic_cast< ProceduralHolder* >( nodeFn.userNode() );
	
	if( !node )
	{
		throw Exception( std::string( "FromMayaProceduralHolderConverter::doConversion: Couldn't find a ProceduralHolder node at " ) + dagPath.fullPathName().asChar() );
	}
	
	return node->getProcedural();
}

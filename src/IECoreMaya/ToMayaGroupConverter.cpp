//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "maya/MFnTransform.h"

#include "IECore/Group.h"

#include "IECoreMaya/ToMayaGroupConverter.h"
#include "IECoreMaya/Convert.h"

using namespace IECoreMaya;

ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaGroupConverter> ToMayaGroupConverter::g_registrar( IECore::Group::staticTypeId(), MFn::kTransform );

ToMayaGroupConverter::ToMayaGroupConverter( IECore::ConstObjectPtr object )
	: ToMayaObjectConverter( "Converts IECore::Group objects to maya hierarchies.", object )
{
}

bool ToMayaGroupConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{	
	IECore::ConstGroupPtr group = IECore::runTimeCast<const IECore::Group>( from );
	if( !group )
	{
		return false;
	}
	
	MFnTransform fnTransform;
	MObject oTransform = fnTransform.create( to );
	
	IECore::ConstTransformPtr coreTransform = group->getTransform();
	if( coreTransform )
	{
		Imath::M44f matrix = coreTransform->transform();
		fnTransform.set( MTransformationMatrix( IECore::convert<MMatrix>( matrix ) ) );
	}
	
	for( IECore::Group::ChildContainer::const_iterator it=group->children().begin(); it!=group->children().end(); it++ )
	{
		ToMayaObjectConverterPtr converter = ToMayaObjectConverter::create( *it );
		if( !converter )
		{
			continue;
		}
		MObject parent = oTransform;
		converter->convert( parent );	
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAlembic/FromAlembicConverter.h"

using namespace Alembic::Abc;
using namespace IECoreAlembic;

IE_CORE_DEFINERUNTIMETYPED( FromAlembicConverter );

FromAlembicConverter::FromAlembicConverter( const std::string &description, Alembic::Abc::IObject iObject )
	:	ToCoreConverter( description ), m_iObject( iObject )
{
	parameters()->addParameter(
		new IECore::IntParameter(
			"sampleIndex",
			"The sample to be converted.",
			0
		)
	);
}

IECore::IntParameter *FromAlembicConverter::sampleIndexParameter()
{
	return parameters()->parameter<IECore::IntParameter>( "sampleIndex" );
}

const IECore::IntParameter *FromAlembicConverter::sampleIndexParameter() const
{
	return parameters()->parameter<IECore::IntParameter>( "sampleIndex" );
}

IECore::ObjectPtr FromAlembicConverter::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	ISampleSelector sampleSelector( (index_t)operands->member<IECore::IntData>( "sampleIndex" )->readable() );

	return doAlembicConversion( m_iObject, sampleSelector, operands.get() );
}

FromAlembicConverterPtr FromAlembicConverter::create( Alembic::Abc::IObject object, IECore::TypeId resultType )
{
	const MetaData &md = object.getMetaData();
	const RegistrationVector &r = registrations();
	for( RegistrationVector::const_reverse_iterator it = r.rbegin(), eIt = r.rend(); it!=eIt; it++ )
	{
		if( ( resultType == it->resultType || RunTimeTyped::inheritsFrom( it->resultType, resultType ) ) && it->matcher( md, kStrictMatching ) )
		{
			return it->creator( object );
		}
	}
	return 0;
}

FromAlembicConverter::RegistrationVector &FromAlembicConverter::registrations()
{
	static RegistrationVector r;
	return r;
}


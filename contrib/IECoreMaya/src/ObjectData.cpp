//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "typeIds/TypeIds.h"

#include "IECoreMaya/ObjectData.h"

using namespace IECore;
using namespace IECoreMaya;

const MString ObjectData::typeName("ieObjectData");
const MTypeId ObjectData::id( ObjectDataId );

ObjectData::ObjectData() : m_object(0)
{
}

ObjectData::~ObjectData()
{
}

void *ObjectData::creator()
{
	return new ObjectData;
}

MStatus ObjectData::readASCII( const MArgList&, unsigned& lastElement )
{
	/// \todo
	m_object = 0;
	return MS::kSuccess;
}
MStatus ObjectData::readBinary( istream& in, unsigned length )
{
	/// \todo
	m_object = 0;	
	return MS::kSuccess;
}

MStatus ObjectData::writeASCII( ostream& out )
{
	/// \todo
	return MS::kSuccess;
}

MStatus ObjectData::writeBinary( ostream& out )
{
	/// \todo
	return MS::kSuccess;
}

void ObjectData::copy( const MPxData &other )
{
	const ObjectData *otherData = dynamic_cast<const ObjectData *>(&other);
	assert (otherData);

	setObject( otherData->getObject() );
}

MTypeId ObjectData::typeId() const
{
	return id;
}

MString ObjectData::name() const
{
	return typeName;
}

ObjectPtr ObjectData::getObject()
{
	return m_object;
}

ConstObjectPtr ObjectData::getObject() const
{
	return m_object;
}

void ObjectData::setObject( ConstObjectPtr otherObject )
{
	if (otherObject)
	{
		m_object = otherObject->copy();
	}
	else
	{
		m_object = 0;
	}
}

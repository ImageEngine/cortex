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

#ifndef IECOREALEMBIC_ALEMBICINPUT_H
#define IECOREALEMBIC_ALEMBICINPUT_H

#include "boost/shared_ptr.hpp"

#include "IECore/RefCounted.h"
#include "IECore/CompoundData.h"
#include "IECore/VectorTypedData.h"

namespace IECoreAlembic
{

IE_CORE_FORWARDDECLARE( AlembicInput )

/// This class provides very high level access to the contents
/// of an Alembic cache. It deliberately hides all Alembic
/// data types and provides an interface tailored to the
/// reading of 3D scene data only. Finer control and lower level
/// access can be obtained by using a combination of the Alembic
/// APIs and the FromAlembicConverters.
class AlembicInput : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( AlembicInput );

		AlembicInput( const std::string &fileName );
		virtual ~AlembicInput();
		
		const std::string &name() const;
		const std::string &fullName() const;
		
		IECore::CompoundDataPtr metaData() const;
		
		Imath::Box3d bound() const;
		
		IECore::ObjectPtr convert( IECore::TypeId resultType = IECore::ObjectTypeId ) const;
		
		size_t numChildren() const;
		AlembicInputPtr child( size_t index ) const;

		IECore::StringVectorDataPtr childNames() const;
		AlembicInputPtr child( const std::string &name ) const;

	private :
	
		AlembicInput();
	
		struct DataMembers;
		
		boost::shared_ptr<DataMembers> m_data;

};

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_ALEMBICINPUT_H

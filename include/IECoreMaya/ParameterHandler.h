//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_PARAMETERHANDLER_H
#define IE_COREMAYA_PARAMETERHANDLER_H

#include <map>

#include "IECore/RefCounted.h"
#include "IECore/TypeIds.h"
#include "IECore/Object.h"
#include "IECore/Parameter.h"

#include "maya/MObject.h"
#include "maya/MString.h"
#include "maya/MPlug.h"

namespace IECoreMaya
{

class ParameterHandler;
IE_CORE_DECLAREPTR( ParameterHandler );

/// \todo Documentation!
class ParameterHandler : public IECore::RefCounted
{	
	friend class Parameter;
	friend class ObjectParameterHandler;	
	
	public :

		/// \todo This should be called create() to match every other factory
		/// interface we have elsewhere.
		/// Return a handler which can deal with the given parameter
		static ConstParameterHandlerPtr get( IECore::ConstParameterPtr parameter );
		
		/// Return a handler which can deal with the given object
		static ConstParameterHandlerPtr get( IECore::ConstObjectPtr object );
		
		/// Return a handler which can deal with an object or parameter of the given type id
		static ConstParameterHandlerPtr get( IECore::TypeId id );		
		
		virtual ~ParameterHandler();
		
		template<class T>
		struct Description
		{
			/// Declare a static instance to register a parameter handler. Pass the type of parameter handled and,
			/// optionally, the type of the data contained within the parameter. This allows, for example, ObjectParameter
			/// to find handlers based on the typeIds of objects it can contain.
			Description( IECore::TypeId parameterType, IECore::TypeId dataType = IECore::InvalidTypeId);
		};
		
	protected:
	
		virtual MObject create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const = 0;
		virtual MStatus update( IECore::ConstParameterPtr parameter, MObject &attribute ) const = 0;
		/// \todo We really need an undoable version of this.
		virtual MStatus setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const = 0;
		virtual MStatus setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const = 0;		
		
	private:
	
		static void registerHandler( IECore::TypeId parameterType, IECore::TypeId dataType, ConstParameterHandlerPtr handler );
        
		typedef std::map<IECore::TypeId, ConstParameterHandlerPtr> HandlerMap;
		
		static HandlerMap &handlers();
};

} // namespace IECoreMaya

#include "IECoreMaya/ParameterHandler.inl"

#endif // IE_COREMAYA_PARAMETERHANDLER_H

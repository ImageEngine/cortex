//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/Export.h"

#include "maya/MObject.h"
#include "maya/MString.h"
#include "maya/MPlug.h"

namespace IECoreMaya
{

class ParameterHandler;
IE_CORE_DECLAREPTR( ParameterHandler );

/// The ParameterHandler class provides a mapping between IECore::Parameters and maya attributes. It
/// is used by the IECoreMaya::ParameterisedHolder classes.
class IECOREMAYA_API ParameterHandler : public IECore::RefCounted
{

	public :

		/// Creates and returns an MPlug capable of representing the specified parameter.
		/// The plug will have the specified name and be added to the specified node.
		/// In the case of a failure MPlug::isNull() will be true for the return value.
		/// \todo: return an MStatus like the other methods, and pass a plug reference as an argument (Cortex 8)
		static MPlug create( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node );
		/// Updates a previously created plug to reflect changes on the specified parameter.
		/// Returns MStatus::kFailure if the plug is not suitable for the parameter.
		/// \bug Maya doesn't seem to correctly store default values for dynamic string attributes
		/// when saving the scene - so this method doesn't set the default value appropriately for
		/// StringParameter and its derived classes (tested in maya 7.0.1).
		static MStatus update( IECore::ConstParameterPtr parameter, MPlug &plug );
		/// Sets the value of plug to reflect the value of parameter.
		static MStatus setValue( IECore::ConstParameterPtr parameter, MPlug &plug );
		/// Sets the value of parameter to reflect the value of plug.
		static MStatus setValue( const MPlug &plug, IECore::ParameterPtr parameter );
		/// Called to restore a parameter's properties when a file is loaded or the version of a held class
		/// has been updated.
		static MStatus restore( const MPlug &plug, IECore::ParameterPtr parameter );

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

		friend class ObjectParameterHandler;

		/// Return a handler which can deal with the given parameter
		static ConstParameterHandlerPtr create( IECore::ConstParameterPtr parameter );
		/// Return a handler which can deal with the given object
		static ConstParameterHandlerPtr create( IECore::ConstObjectPtr object );
		/// Return a handler which can deal with an object or parameter of the given type id
		static ConstParameterHandlerPtr create( IECore::TypeId id );

		/// Performs common actions which all handlers should apply to newly created plugs, including
		/// creating any default connections requested in the parameter userData. This function should
		/// be called at the end of all doCreate() implementations.
		MPlug finishCreating( IECore::ConstParameterPtr parameter, MPlug &plug ) const;
		/// An overload for the above function which accepts an attribute for which a plug needs to be made.
		MPlug finishCreating( IECore::ConstParameterPtr parameter, MObject &attribute, MObject &node ) const;

		/// Performs common actions which all handlers should apply to updated plugs, including
		/// the setting of any Attribute properties as requested in the parameter userData.
		/// Currently, only 'storable' is supported.
		/// This function should be called at the end of all doUpdate() implementations, and doCreate()
		/// if doUpdate() isn't called as part of doCreate()
		MStatus finishUpdating( IECore::ConstParameterPtr parameter, MPlug &plug ) const;
		/// An overload for the above function which accepts an attribute for which a plug needs to be made.
		MStatus finishUpdating( IECore::ConstParameterPtr parameter, MObject &attribute, MObject &node ) const;

		virtual MPlug doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const = 0;
		virtual MStatus doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const = 0;
		virtual MStatus doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const = 0;
		virtual MStatus doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const = 0;
		virtual MStatus doRestore( const MPlug &plug, IECore::ParameterPtr parameter ) const;

	private:

		static void registerHandler( IECore::TypeId parameterType, IECore::TypeId dataType, ConstParameterHandlerPtr handler );

		typedef std::map<IECore::TypeId, ConstParameterHandlerPtr> HandlerMap;

		static HandlerMap &handlers();
};

} // namespace IECoreMaya

#include "IECoreMaya/ParameterHandler.inl"

#endif // IE_COREMAYA_PARAMETERHANDLER_H

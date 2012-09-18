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

#ifndef IE_COREMAYA_OBJECTDATA_H
#define IE_COREMAYA_OBJECTDATA_H

#include "IECore/Object.h"


#include "maya/MArgList.h"
#include "maya/MPxData.h"
#include "maya/MString.h"
#include "maya/MTypeId.h"

namespace IECoreMaya
{

/// A Maya data type to hold IECore::Objects, allowing them to participate in the Dependency Graph.
/// \todo Perhaps offer this as a template instead, to allow specific classes of objects to be held
/// in their own data type entirely. Maya would then prevent connection between incompatible
/// types.
class ObjectData : public MPxData
{
	public:

		enum CopyMode
		{
			Shallow,
			Deep
		};

		ObjectData();
		virtual ~ObjectData();

		static void* creator();

		virtual MStatus readASCII( const MArgList &argList, unsigned int &endOfTheLastParsedElement );
		virtual MStatus readBinary( istream &in, unsigned length );
		virtual MStatus writeASCII( ostream &out );
		virtual MStatus writeBinary( ostream &out );

		/// The behaviour of this function is defined by the current copy mode of source.
		virtual void copy( const MPxData &source );
		virtual MTypeId typeId() const;
		virtual MString name() const;

		static const MString typeName;
		static const MTypeId id;

		/// Controls how the copy() method behaves when this object is the source
		/// for the copy. When in Shallow mode
		/// an ObjectData copy will point to the same Object that the original
		/// pointed to. When in Deep mode, the copy will point to a copy() of
		/// the original Object. The copied ObjectData inherits the copy mode
		/// from the original. The default copy mode for all new instances of
		/// ObjectData is Deep.
		void setCopyMode( CopyMode mode );
		/// Returns the current copy mode.
		CopyMode getCopyMode() const;

		/// Returns the object held by this instance - note that this is not
		/// a copy so you should be careful not to cause unwanted side effects
		/// through modification.
		IECore::ObjectPtr getObject();
		IECore::ConstObjectPtr getObject() const;
		/// Sets the object held by this instance - note that a copy is not
		/// taken, so any subsequent modification of object directly affects this
		/// ObjectData.
		void setObject( IECore::ObjectPtr object );

	protected:

		CopyMode m_copyMode;
		IECore::ObjectPtr m_object;

};

}

#endif // IE_COREMAYA_OBJECTDATA_H

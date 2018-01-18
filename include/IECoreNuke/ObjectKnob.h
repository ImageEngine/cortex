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

#ifndef IECORENUKE_OBJECTKNOB_H
#define IECORENUKE_OBJECTKNOB_H

#include "DDImage/Knobs.h"

#include "IECore/Object.h"

#include "IECoreNuke/Export.h"

namespace IECoreNuke
{

/// A nuke knob capable of holding arbitrary IECore::Objects.
class IECORENUKE_API ObjectKnob : public DD::Image::Knob
{

	public :

		/// A copy of the value is taken. Returns true if the new value is different
		/// to the old value and false otherwise.
		bool setValue( IECore::ConstObjectPtr value );
		IECore::ConstObjectPtr getValue() const;

		/// Call this from an Op::knobs() implementation to create an ObjectKnob. The value placed in storage by the knob
		/// must be treated as read only.
		static ObjectKnob *objectKnob( DD::Image::Knob_Callback f, IECore::ObjectPtr *storage, const char *name, const char *label );

	protected :

		ObjectKnob( DD::Image::Knob_Closure *f, IECore::ObjectPtr *storage, const char *name, const char *label = 0 );
		virtual ~ObjectKnob();

		virtual const char *Class() const;

		virtual void to_script( std::ostream &os, const DD::Image::OutputContext *context, bool quote ) const;
		virtual bool from_script( const char *value );
		virtual bool not_default() const;
		virtual void store( DD::Image::StoreType storeType, void *storage, DD::Image::Hash &hash, const DD::Image::OutputContext &context );

	private :

		bool valuesEqual( const IECore::Object *value1, const IECore::Object *value2 ) const;

		IECore::ObjectPtr m_defaultValue;
		IECore::ObjectPtr m_value;

};

namespace Detail
{

// Used to implement the python binding
struct PythonObjectKnob : public IECore::RefCounted
{

	IE_CORE_DECLAREMEMBERPTR( PythonObjectKnob );

	ObjectKnob *objectKnob;

};

IE_CORE_DECLAREPTR( PythonObjectKnob );

} // namespace Detail

} // namespace IECoreNuke

#endif // IECORENUKE_OBJECTKNOB_H

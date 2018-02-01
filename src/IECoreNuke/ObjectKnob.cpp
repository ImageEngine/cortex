//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECoreNuke/ObjectKnob.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECore/HexConversion.h"
#include "IECore/MemoryIndexedIO.h"

using namespace IECoreNuke;
using namespace DD::Image;
using namespace boost::python;

ObjectKnob::ObjectKnob( DD::Image::Knob_Closure *f, IECore::ObjectPtr *storage, const char *name, const char *label )
	:	DD::Image::Knob( f, name, label ), m_defaultValue( 0 ), m_value( 0 )
{

	set_flag( NO_ANIMATION );

	if( storage && *storage )
	{
		m_defaultValue = (*storage)->copy();
		m_value = m_defaultValue;
	}

	// set up the object that will provide the python binding
	IECorePython::ScopedGILLock gilLock;
	Detail::PythonObjectKnobPtr pythonKnob = new Detail::PythonObjectKnob;
	pythonKnob->objectKnob = this;
	object pythonKnobObject( pythonKnob );
	Py_INCREF( pythonKnobObject.ptr() );
	setPyObject( pythonKnobObject.ptr() );
}

ObjectKnob::~ObjectKnob()
{
	// tidy up the object for the python binding
	IECorePython::ScopedGILLock gilLock;
	object pythonKnobObject( handle<>( borrowed( (PyObject *)pyObject() ) ) );
	Detail::PythonObjectKnobPtr pythonKnob = extract<Detail::PythonObjectKnobPtr>( pythonKnobObject );
	pythonKnob->objectKnob = 0;
	Py_DECREF( pythonKnobObject.ptr() );
}

bool ObjectKnob::setValue( IECore::ConstObjectPtr value )
{
	if( !valuesEqual( m_value.get(), value.get() ) )
	{
		new_undo();
		m_value = value ? value->copy() : 0;
		changed();
		return true;
	}

	return false;
}

IECore::ConstObjectPtr ObjectKnob::getValue() const
{
	return m_value;
}

ObjectKnob *ObjectKnob::objectKnob( DD::Image::Knob_Callback f, IECore::ObjectPtr *storage, const char *name, const char *label )
{
	return CustomKnob2( ObjectKnob, f, storage, name, label );
}

const char *ObjectKnob::Class() const
{
	return "ObjectKnob";
}

void ObjectKnob::to_script( std::ostream &os, const DD::Image::OutputContext *context, bool quote ) const
{
	if( quote )
	{
		os << "{";
	}

		if( m_value )
		{
			IECore::MemoryIndexedIOPtr io = new IECore::MemoryIndexedIO( IECore::ConstCharVectorDataPtr(), IECore::IndexedIO::rootPath, IECore::IndexedIO::Exclusive | IECore::IndexedIO::Write );
			m_value->save( io, "object" );
			IECore::ConstCharVectorDataPtr buffer = io->buffer();
			os << IECore::decToHex( buffer->readable().begin(), buffer->readable().end() );
		}

	if( quote )
	{
		os << "}";
	}
}

bool ObjectKnob::from_script( const char *value )
{
	IECore::ObjectPtr object = m_defaultValue;
	if( value && strlen( value ) )
	{

		size_t n = strlen( value );
		IECore::CharVectorDataPtr buffer = new IECore::CharVectorData;
		buffer->writable().resize( n / 2 );
		IECore::hexToDec<char>( value, value + n, buffer->writable().begin() );

		try
		{
			IECore::MemoryIndexedIOPtr io = new IECore::MemoryIndexedIO( buffer, IECore::IndexedIO::rootPath, IECore::IndexedIO::Exclusive | IECore::IndexedIO::Read );
			object = IECore::Object::load( io, "object" );
		}
		catch( std::exception &e )
		{
			error( e.what() );
		}

	}

	return setValue( object );
}

bool ObjectKnob::not_default() const
{
	return !valuesEqual( m_value.get(), m_defaultValue.get() );
}

void ObjectKnob::store( DD::Image::StoreType storeType, void *storage, DD::Image::Hash &hash, const DD::Image::OutputContext &context )
{
	assert( storeType == DD::Image::Custom );
	if( storage )
	{
		*((IECore::ObjectPtr *)storage) = m_value;
	}
}

bool ObjectKnob::valuesEqual( const IECore::Object *value1, const IECore::Object *value2 ) const
{
	bool equal = true;
	if( value2 )
	{
		equal = value1 && value1->isEqualTo( value2 );
	}
	else
	{
		equal = !value1;
	}

	return equal;
}

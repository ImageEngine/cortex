//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREPYTHON_RUNTIMETYPEDBINDING_H
#define IE_COREPYTHON_RUNTIMETYPEDBINDING_H

#include "IECore/bindings/RefCountedBinding.h"

namespace IECore
{

void bindRunTimeTyped();

/// A class to simplify the binding of RunTimeTyped derived classes. This should be used
/// in place of the usual boost::python::class_. It automatically makes sure the class is bound
/// with the correct name and base class, as well as dealing with all the issues that RefCountedClass
/// fixes.
template<typename T, typename Ptr=boost::intrusive_ptr<T> >
class RunTimeTypedClass : public RefCountedClass<T, typename T::BaseClass, Ptr>
{

	public :

		typedef RefCountedClass<T, typename T::BaseClass, Ptr> BaseClass;

		RunTimeTypedClass( const char *docString = 0 );

};

#define IE_COREPYTHON_RUNTIMETYPEDWRAPPERFNS( CLASSNAME )\
	virtual TypeId typeId() const\
	{\
		if( boost::python::override f = this->get_override( "typeId" ) )\
		{\
			boost::python::object res = f(); \
			return boost::python::extract<TypeId>( res );\
		}\
		return CLASSNAME::typeId();\
	}\
	virtual const char *typeName() const\
	{\
		if( boost::python::override f = this->get_override( "typeName" ) )\
		{\
			boost::python::object res = f(); \
			return boost::python::extract<const char *>( res );\
		}\
		return CLASSNAME::typeName();\
	}\
	virtual bool isInstanceOf( TypeId typeId ) const\
	{\
		if( boost::python::override f = this->get_override( "isInstanceOf" ) )\
		{\
			boost::python::object res = f( typeId ); \
			return boost::python::extract<bool>( res );\
		}\
		return CLASSNAME::isInstanceOf( typeId );\
	}\
	virtual bool isInstanceOf( const char *typeName ) const\
	{\
		if( boost::python::override f = this->get_override( "isInstanceOf" ) )\
		{\
			boost::python::object res = f( typeName ); \
			return boost::python::extract<bool>( res );\
		}\
		return CLASSNAME::isInstanceOf( typeName );\
	}

} // namespace IECore

#include "IECore/bindings/RunTimeTypedBinding.inl"

#endif // IE_COREPYTHON_RUNTIMETYPEDBINDING_H

//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_SCENEINTERFACE_INL
#define IE_CORE_SCENEINTERFACE_INL

namespace IECore
{

template< typename T > typename T::Ptr SceneInterface::parent()
{
	return dynamicPointerCast< T >( parentImpl() );
}

template< typename T > typename T::ConstPtr SceneInterface::parent() const
{
	return dynamicPointerCast< T >( parentImpl() );
}

template< typename T > typename T::Ptr SceneInterface::child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour )
{
	return dynamicPointerCast< T >( childImpl( name, missingBehavior ) );
}

template< typename T > typename T::ConstPtr SceneInterface::child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return dynamicPointerCast< T >( childImpl( name, missingBehavior ) );
}

template< typename T > typename T::Ptr SceneInterface::createChild( const Name &name )
{
	return dynamicPointerCast< T >( createChildImpl( name, missingBehavior ) );
}

template< typename T > typename T::ConstPtr SceneInterface::scene( const Path &path, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return dynamicPointerCast< T >( sceneImpl( name, missingBehavior ) );
}

template<> SceneInterfacePtr SceneInterface::parent()
{
	return parentImpl();
}

template<> ConstSceneInterfacePtr SceneInterface::parent() const
{
	return parentImpl();
}

template<> SceneInterfacePtr SceneInterface::child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour )
{
	return childImpl( name, missingBehavior );
}

template<> ConstSceneInterfacePtr SceneInterface::child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return childImpl( name, missingBehavior );
}

template<> SceneInterfacePtr SceneInterface::createChild( const Name &name )
{
	return createChildImpl( name, missingBehavior );
}

template<> ConstSceneInterfacePtr SceneInterface::scene( const Path &path, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return sceneImpl( name, missingBehavior );
}

} // namespace IECore

#endif // IE_CORE_SCENEINTERFACE_INL

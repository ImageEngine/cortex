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

#include "IECore/LensModel.h"

#include "IECore/Object.h"
#include "IECore/RunTimeTyped.h"

#include <cmath>
#include <iostream>
#include <string>

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( LensModel );

LensModel::LensModel()
	: Parameterised( this->staticTypeName() )
{
}

LensModel::~LensModel()
{
}

Imath::Box2i LensModel::bounds( int mode, const Imath::Box2i &input, int width, int height )
{
	Imath::Box2i out( input );
	bool init( false );

	for( int i = input.min.x; i <= input.max.x; ++i )
	{
		for( int pass = 0; pass < 2; ++pass )
		{
			double x = ( double(i) + 0.5 ) / width;
			double y = ( double( pass == 0 ? input.min.y : input.max.y ) + 0.5 ) / height;

			Imath::V2d pIn( x, y );
			Imath::V2d pOut( 0, 0 );

			if( mode == Distort )
			{
				pOut = distort( pIn );
			}
			else
			{
				pOut = undistort( pIn );
			}

			if( !std::isinf( pOut.x ) && !std::isinf( pOut.y ) && !std::isnan( pOut.x ) && !std::isnan( pOut.y ) )
			{
				pOut.x = pOut.x*width-0.5;
				pOut.y = pOut.y*height-0.5;
				if( !init )
				{
					out.min.x = int( floor( pOut.x ) );
					out.min.y = int( floor( pOut.y ) );
					out.max.x = int( floor( pOut.x ) );
					out.max.y = int( floor( pOut.y ) );
					init = true;
				}
				else
				{
					out.min.x = std::min( int( floor( pOut.x ) ), out.min.x );
					out.min.y = std::min( int( floor( pOut.y ) ), out.min.y );
					out.max.x = std::max( int( floor( pOut.x ) ), out.max.x );
					out.max.y = std::max( int( floor( pOut.y ) ), out.max.y );
				}
			}
		}
	}

	if ( !init ) return Imath::Box2i( Imath::V2i(0,0), Imath::V2i(0,0) );

	for( int j = input.min.y; j <= input.max.y; j++ )
	{
		for( int pass = 0; pass < 2; pass++ )
		{
			double x = ( double( pass == 0 ? input.min.x : input.max.x ) + 0.5 ) / width;
			double y = ( double(j) + 0.5 ) / height;

			Imath::V2d pIn( x, y );
			Imath::V2d pOut( 0, 0 );

			if( mode == Distort )
			{
				pOut = distort( pIn );
			}
			else
			{
				pOut = undistort( pIn );
			}

			if( !std::isinf( pOut.x ) && !std::isinf( pOut.y ) && !std::isnan( pOut.x ) && !std::isnan( pOut.y ) )
			{
				pOut.x = pOut.x*width-0.5;
				pOut.y = pOut.y*height-0.5;

				out.min.x = std::min( int( floor( pOut.x ) ), out.min.x );
				out.min.y = std::min( int( floor( pOut.y ) ), out.min.y );
				out.max.x = std::max( int( floor( pOut.x ) ), out.max.x );
				out.max.y = std::max( int( floor( pOut.y ) ), out.max.y );
			}
		}
	}

	return out;
}

LensModelPtr LensModel::create( const std::string &name )
{
	// Check to see whether the requested lens model is registered and if not, throw an exception.
	if ( creators().find( name ) == creators().end() )
		throw IECore::Exception( (boost::format("LensModel: Could not find registered lens model \"%s\".") % name).str() );

	LensModelPtr lensModel = (creators()[name])( new CompoundObject() );

	// Return a new instance of the LensModel
	return lensModel;
}

LensModelPtr LensModel::create( IECore::TypeId id )
{
	return create( IECore::RunTimeTyped::typeNameFromTypeId( static_cast<IECore::TypeId>(id) ) );
}

LensModelPtr LensModel::create( IECore::ConstCompoundObjectPtr lensParams )
{
	// The parameter lensParams must have the data member "lensModel"
	IECore::ConstStringDataPtr modelName(lensParams->member<IECore::StringData>("lensModel", true));
	std::string name( modelName->readable() );

	// Check to see whether the requested lens model is registered and if not, throw an exception.
	if ( creators().find( name ) == creators().end() )
		throw IECore::Exception( (boost::format("LensModel: Could not find registered lens model \"%s\".") % name).str() );

	LensModelPtr lensModel = (creators()[name])( lensParams );

	// Return a new instance of the LensModel
	return lensModel;
}

std::map< std::string, LensModel::CreatorFn >& LensModel::creators()
{
	static std::map< std::string, LensModel::CreatorFn > c;
	return c;
}

std::vector<std::string> LensModel::lensModels()
{
	std::vector<std::string> models;
	for( CreatorMap::iterator it = creators().begin(); it != creators().end(); ++it )
	{
		models.push_back( it->first );
	}
	return models;
}


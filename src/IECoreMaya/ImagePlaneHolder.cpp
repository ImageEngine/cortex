//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
#include <iostream>
#include <cassert>

#include "boost/format.hpp"

#include "maya/MImage.h"
#include "maya/MGlobal.h"

#include "IECoreMaya/ImagePlaneHolder.h"
#include "IECoreMaya/Parameter.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/ToMayaImageConverter.h"

#include "IECore/ImagePrimitive.h"
#include "IECore/MessageHandler.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/TypedParameter.h"

#include "IECore/Parameterised.h"

using namespace IECore;
using namespace IECoreMaya;

MTypeId ImagePlaneHolder::id( ImagePlaneHolderId );

ImagePlaneHolder::ImagePlaneHolder()
{
}

ImagePlaneHolder::~ImagePlaneHolder()
{
}

void ImagePlaneHolder::postConstructor()
{
	setExistWithoutInConnections(true);
	setExistWithoutOutConnections(true);	
}

bool ImagePlaneHolder::isAbstractClass()
{
	return false;
}

void *ImagePlaneHolder::creator()
{
	return new ImagePlaneHolder();
}

MStatus ImagePlaneHolder::initialize()
{
	return inheritAttributesFrom( ParameterisedHolderImagePlane::typeName );
}	 

MStatus ImagePlaneHolder::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	/// This isn't the best way of doing it, but at this point we can't even be sure that the Op has been loaded,
	/// so calling plugParameter() may not work. We can't call getOp() or getParameterised() here, as it seems
	/// we can't do things such as adding/removing attributes within this function
	if( std::string( plug.partialName().substring( 0, 4 ).asChar() ) == "parm_" )
	{
		setImageDirty();
	}
		
	return ParameterisedHolderImagePlane::setDependentsDirty( plug, plugArray );
}

MStatus ImagePlaneHolder::loadImageMap ( const MString &fileName, int frame, MImage &image )
{
	IECore::OpPtr op = getOp();
	
	if (op)
	{
		IECore::ObjectPtr result;

		try
		{
			ParameterisedHolderImagePlane::setParameterisedValues();
			result = op->operate();
			if (!result)
			{
				return MS::kFailure;
			}
			
			ImagePrimitivePtr imageResult = runTimeCast< ImagePrimitive >( result );
		
			if (!imageResult )
			{
				MGlobal::displayError( "ImagePlaneHolder: Op did not return an ImagePrimtiive" ); // \todo
				return MS::kFailure;
			}

			ToMayaImageConverterPtr toMaya = ToMayaImageConverter::create( imageResult );	

			MStatus s = toMaya->convert( image );
			if ( !s )
			{
				return MS::kFailure;
			}
			
			return MS::kSuccess;
		} 		
		catch( std::exception &e )
		{
			MGlobal::displayError( e.what() );
			return MS::kFailure;
		}
		catch( boost::python::error_already_set & )
		{
			PyErr_Print();
			return MS::kFailure;			
		}		
		catch (...)
		{
			MGlobal::displayError( "ImagePlaneHolder: Caught unknown error" );
			return MS::kFailure;		
		}	
	}

	return MS::kFailure;
}
		
MStatus ImagePlaneHolder::setOp( const std::string &className, int classVersion )
{
        return setParameterised( className, classVersion, "IECORE_OP_PATHS");
}

IECore::OpPtr ImagePlaneHolder::getOp( std::string *className, int *classVersion )
{
        return IECore::runTimeCast<IECore::Op>( getParameterised( className, classVersion ) );       
}

//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ColorTransformOp.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost;
using namespace boost::python;


namespace IECore {

class ColorTransformOpWrap : public ColorTransformOp, public Wrapper<ColorTransformOp>
{
	public :
		
		ColorTransformOpWrap( PyObject *self, const std::string name, const std::string description ) : ColorTransformOp( name, description ), Wrapper<ColorTransformOp>( self, this ) {};
		
		virtual void begin( ConstCompoundObjectPtr operands )
		{
			override o = this->get_override( "begin" );
			if( o )
			{
				o( const_pointer_cast<CompoundObject>( operands ) );
			}
		}
		
		virtual void transform( Imath::Color3f &color ) const
		{
			override o = this->get_override( "transform" );
			if( o )
			{
				Imath::Color3f c = o( color );
				color = c;
			}
			else
			{
				throw Exception( "transform() python method not defined" );
			}
		};

		virtual void end()
		{
			override o = this->get_override( "end" );
			if( o )
			{
				o();
			}
		}
			
};
IE_CORE_DECLAREPTR( ColorTransformOpWrap );

void bindColorTransformOp()
{
	using boost::python::arg;
	
	typedef class_< ColorTransformOp, ColorTransformOpWrapPtr, boost::noncopyable, bases<PrimitiveOp> > ColorTransformOpPyClass;
	ColorTransformOpPyClass( "ColorTransformOp", no_init )
		.def( init< const std::string, const std::string>( ( arg( "name" ), arg( "description" ) ) ) )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(ColorTransformOp)
	;
	
	WrapperToPython<ColorTransformOpPtr>();

	INTRUSIVE_PTR_PATCH( ColorTransformOp, ColorTransformOpPyClass );
	implicitly_convertible<ColorTransformOpPtr, PrimitiveOpPtr>();	

}

} // namespace IECore

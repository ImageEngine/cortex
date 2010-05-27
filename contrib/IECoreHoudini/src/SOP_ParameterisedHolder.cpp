//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include <IECore/SimpleTypedData.h>
#include <IECore/MessageHandler.h>

#include <boost/python.hpp>
#include <boost/format.hpp>
using namespace boost::python;
using namespace boost;

#include "IECorePython/ScopedGILLock.h"

#include "CoreHoudini.h"
#include "SOP_ParameterisedHolder.h"
using namespace IECore;
using namespace IECoreHoudini;

SOP_ParameterisedHolder::SOP_ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op ) :
	SOP_Node( net, name, op ),
	m_parameterised(0),
	m_requiresUpdate(true)
{
	CoreHoudini::initPython();
}

SOP_ParameterisedHolder::~SOP_ParameterisedHolder()
{
}

void SOP_ParameterisedHolder::setParameterised( IECore::RunTimeTypedPtr p )
{
	m_parameterised=p;
}

IECore::RunTimeTypedPtr SOP_ParameterisedHolder::getParameterised()
{
	return m_parameterised;
}

bool SOP_ParameterisedHolder::hasParameterised()
{
	return (bool)(m_parameterised.get()!=0);
}

/// update a specific parameters on parameterised using SOP param
void SOP_ParameterisedHolder::updateParameter( IECore::ParameterPtr parm, float now )
{
	try
	{
		std::string parm_name = "parm_" + std::string( parm->name() );

		// check we can find the parameter on our Houdini node
		if ( !getParmList()->getParmPtr( parm_name.c_str() ) )
			return;

		// does this parameter cause a gui refresh?
		bool do_update = true;
		if ( parm->userData()->members().count("gui_update")>0 )
		{
			BoolDataPtr update_data = IECore::runTimeCast<BoolData>( parm->userData()->members()["gui_update"] );
			if ( update_data )
				do_update = update_data->readable();
		}

		// handle the different parameter types
		switch( parm->typeId() )
		{
			// int parameter
			case IECore::IntParameterTypeId:
			{
				int val = evalInt( parm_name.c_str(), 0, now );
				checkForUpdate<int, IntData>( do_update, val, parm );
				parm->setValue( new IECore::IntData(val) );
				break;
			}

			// V2f parameter
			case IECore::V2iParameterTypeId:
			{
				int vals[2];
				for ( unsigned int i=0; i<2; i++ )
					vals[i] = evalInt( parm_name.c_str(), i, now );
				Imath::V2i val(vals[0], vals[1]);
				checkForUpdate<Imath::V2i, V2iData>( do_update, val, parm );
				parm->setValue( new IECore::V2iData(val) );
				break;
			}

			// V3i parameter
			case IECore::V3iParameterTypeId:
			{
				int vals[3];
				for ( unsigned int i=0; i<3; i++ )
					vals[i] = evalInt( parm_name.c_str(), i, now );
				Imath::V3i val(vals[0], vals[1], vals[2]);
				checkForUpdate<Imath::V3i, V3iData>( do_update, val, parm );
				parm->setValue( new IECore::V3iData(val) );
				break;
			}

			// float parameter
			case IECore::FloatParameterTypeId:
			{
				float val = evalFloat( parm_name.c_str(), 0, now );
				checkForUpdate<float, FloatData>( do_update, val, parm );
				parm->setValue( new IECore::FloatData(val) );
				break;
			}

			// V2f parameter
			case IECore::V2fParameterTypeId:
			{
				float vals[2];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::V2f val(vals[0], vals[1]);
				checkForUpdate<Imath::V2f, V2fData>( do_update, val, parm );
				parm->setValue( new IECore::V2fData(val) );
				break;
			}

			// V3f parameter
			case IECore::V3fParameterTypeId:
			{
				float vals[3];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::V3f val(vals[0], vals[1], vals[2]);
				checkForUpdate<Imath::V3f, V3fData>( do_update, val, parm );
				parm->setValue( new IECore::V3fData(val) );
				break;
			}

			// double parameter
			case IECore::DoubleParameterTypeId:
			{
				float val = evalFloat( parm_name.c_str(), 0, now );
				checkForUpdate<float, DoubleData>( do_update, val, parm );
				parm->setValue( new IECore::DoubleData(val) );
				break;
			}

			// V2d parameter
			case IECore::V2dParameterTypeId:
			{
				float vals[2];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::V2d val(vals[0], vals[1]);
				checkForUpdate<Imath::V2d, V2dData>( do_update, val, parm );
				parm->setValue( new IECore::V2dData(val) );
				break;
			}

			// V3d parameter
			case IECore::V3dParameterTypeId:
			{
				float vals[3];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::V3d val(vals[0], vals[1], vals[2]);
				checkForUpdate<Imath::V3d, V3dData>( do_update, val, parm );
				parm->setValue( new IECore::V3dData(val) );
				break;
			}

			// bool parameter
			case IECore::BoolParameterTypeId:
			{
				bool val = (bool)evalInt( parm_name.c_str(), 0, now );
				checkForUpdate<bool, BoolData>( do_update, val, parm );
				parm->setValue( new IECore::BoolData(val) );
				break;
			}

			// string parameter
			case IECore::StringParameterTypeId:
			case IECore::PathParameterTypeId:
			case IECore::DirNameParameterTypeId:
			case IECore::FileNameParameterTypeId:
			case IECore::FileSequenceParameterTypeId:
			{
				UT_String h_str;
				evalString( h_str, parm_name.c_str(), 0, now );
				std::string val( h_str.buffer() );
				checkForUpdate<std::string, StringData>( do_update, val, parm );
				parm->setValue( new IECore::StringData(val) );
				break;
			}

			// colour 3f parameter
			case IECore::Color3fParameterTypeId:
			{
				float vals[3];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::Color3f val( vals[0], vals[1], vals[2] );
				checkForUpdate<Imath::Color3f, Color3fData>( do_update, val, parm );
				parm->setValue( new IECore::Color3fData( val ) );
				break;
			}

			// colour 4f parameter
			case IECore::Color4fParameterTypeId:
			{
				float vals[4];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::Color4f val( vals[0], vals[1], vals[2], vals[3] );
				checkForUpdate<Imath::Color4f, Color4fData>( do_update, val, parm );
				parm->setValue( new IECore::Color4fData( val ) );
				break;
			}

			// M44f
			case IECore::M44fParameterTypeId:
			{
				float vals[16];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::M44f val(vals[0], vals[1], vals[2], vals[3],
								vals[4], vals[5], vals[6], vals[7],
								vals[8], vals[9], vals[10], vals[11],
								vals[12], vals[13], vals[14], vals[15]);
				checkForUpdate<Imath::M44f, M44fData>( do_update, val, parm );
				parm->setValue( new IECore::M44fData(val) );
				break;
			}

			// M44d
			case IECore::M44dParameterTypeId:
			{
				float vals[16];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::M44d val(vals[0], vals[1], vals[2], vals[3],
								vals[4], vals[5], vals[6], vals[7],
								vals[8], vals[9], vals[10], vals[11],
								vals[12], vals[13], vals[14], vals[15]);
				checkForUpdate<Imath::M44d, M44dData>( do_update, val, parm );
				parm->setValue( new IECore::M44dData(val) );
				break;
			}


			default:
				std::cerr << "Could not get parameter values from '" << parm_name << "' of type " << parm->typeName() << std::endl;
				break;
		}
	}
	catch( const std::exception &e )
	{
		IECore::msg( IECore::Msg::Error, "SOP_ParameterisedHolder::updateParameter", e.what() );
	}
	catch( ... )
	{
		IECore::msg( IECore::Msg::Error, "SOP_ParameterisedHolder::updateParameter", "Caught unknown exception" );
	}
}

/// Utility class which loads a ParameterisedProcedural from Disk
IECore::RunTimeTypedPtr SOP_ParameterisedHolder::loadParameterised(
		const std::string &type,
		int version,
		const std::string &search_path )
{
	IECore::RunTimeTypedPtr new_procedural;
	IECorePython::ScopedGILLock gilLock;

	string python_cmd = boost::str( format(
			"IECore.ClassLoader.defaultLoader( \"%s\" ).load( \"%s\", %d )()\n"
		) % search_path % type % version );

	try
	{
		boost::python::handle<> resultHandle( PyRun_String(
			python_cmd.c_str(),
			Py_eval_input, CoreHoudini::globalContext().ptr(),
			CoreHoudini::globalContext().ptr() )
		);
		boost::python::object result( resultHandle );
		new_procedural = boost::python::extract<IECore::RunTimeTypedPtr>(result)();
		if ( IECore::runTimeCast<IECore::ParameterisedProcedural>(new_procedural)==0 )
		{
			new_procedural = 0;
		}
	}
	catch( ... )
	{
		PyErr_Print();
	}
	return new_procedural;
}

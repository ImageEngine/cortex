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

#include "IECorePython/ScopedGILLock.h"

#include "CoreHoudini.h"
#include "SOP_ParameterisedHolder.h"

using namespace boost::python;
using namespace boost;

using namespace IECore;
using namespace IECoreHoudini;

SOP_ParameterisedHolder::SOP_ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op ) :
	SOP_Node( net, name, op ),
	m_className(""),
	m_classVersion(-1),
	m_parameterised(0),
	m_requiresUpdate(true),
    m_matchString("")
{
	CoreHoudini::initPython();
	enableParameterisedUpdate();
}

SOP_ParameterisedHolder::~SOP_ParameterisedHolder()
{
}

void SOP_ParameterisedHolder::setParameterisedDirectly( IECore::RunTimeTypedPtr p )
{
	m_parameterised = p;
}

IECore::RunTimeTypedPtr SOP_ParameterisedHolder::getParameterised()
{
	return m_parameterised;
}

bool SOP_ParameterisedHolder::hasParameterised()
{
	return (bool)(m_parameterised.get()!=0);
}

/// update a Cortex parameter on using our SOP parameter value
void SOP_ParameterisedHolder::updateParameter( IECore::ParameterPtr parm, float now, std::string prefix, bool top_level )
{
	try
	{
		// find out our parameter name
		std::string parm_name = prefix + std::string("parm_") + std::string( parm->name() );

		// compoundParameters - recursively calling updateParameter on children
		if ( parm->typeId()==IECore::CompoundParameterTypeId )
		{
			if ( top_level==true )
			{
				parm_name = ""; // our top-level compound parameter should not apply a prefix
			}
			else
			{
				parm_name += "_";
			}

			IECore::CompoundParameterPtr compound = IECore::runTimeCast<CompoundParameter>(parm);
			if ( parm )
			{
				const IECore::CompoundParameter::ParameterMap &child_parms = compound->parameters();
				for( IECore::CompoundParameter::ParameterMap::const_iterator it=child_parms.begin();
						it!=child_parms.end(); ++it )
				{
					updateParameter( it->second, now, parm_name );
				}
			}
			return;
		}

		// check we can find the parameter on our Houdini node
		if ( !getParmList()->getParmPtr( parm_name.c_str() ) )
			return;

		// does this parameter cause a gui refresh?
		bool do_update = true;
		if( CompoundObjectPtr uiData = parm->userData()->member<CompoundObject>( "UI" ) )
		{
			// World space parameter values
			if( BoolDataPtr update_data = uiData->member<BoolData>( "update" ) )
			{
				do_update = update_data->readable();
			}
		}

		/// \todo: This gui_update userData flag is deprecated!
		if ( BoolDataPtr update_data = parm->userData()->member<BoolData>("gui_update") )
		{
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

			// Box2i
			case IECore::Box2iParameterTypeId:
			{
				int vals[4];
				for ( unsigned int i=0; i<4; i++ )
					vals[i] = evalInt( parm_name.c_str(), i, now );
				Imath::Box2i val( Imath::V2i( vals[0], vals[1] ),
									Imath::V2i( vals[2], vals[3] ) );
				checkForUpdate<Imath::Box2i, Box2iData>( do_update, val, parm );
				parm->setValue( new IECore::Box2iData(val) );
				break;
			}

			// Box2f
			case IECore::Box2fParameterTypeId:
			{
				float vals[4];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::Box2f val( Imath::V2f( vals[0], vals[1] ),
									Imath::V2f( vals[2], vals[3] ) );
				checkForUpdate<Imath::Box2f, Box2fData>( do_update, val, parm );
				parm->setValue( new IECore::Box2fData(val) );
				break;
			}

			// Box2d
			case IECore::Box2dParameterTypeId:
			{
				float vals[4];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::Box2d val( Imath::V2d( vals[0], vals[1] ),
									Imath::V2d( vals[2], vals[3] ) );
				checkForUpdate<Imath::Box2d, Box2dData>( do_update, val, parm );
				parm->setValue( new IECore::Box2dData(val) );
				break;
			}

			// Box3i
			case IECore::Box3iParameterTypeId:
			{
				int vals[6];
				for ( unsigned int i=0; i<6; i++ )
					vals[i] = evalInt( parm_name.c_str(), i, now );
				Imath::Box3i val( Imath::V3i( vals[0], vals[1], vals[2] ),
									Imath::V3i( vals[3], vals[4], vals[5] ) );
				checkForUpdate<Imath::Box3i, Box3iData>( do_update, val, parm );
				parm->setValue( new IECore::Box3iData(val) );
				break;
			}

			// Box3f
			case IECore::Box3fParameterTypeId:
			{
				float vals[6];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::Box3f val( Imath::V3f( vals[0], vals[1], vals[2] ),
									Imath::V3f( vals[3], vals[4], vals[5] ) );
				checkForUpdate<Imath::Box3f, Box3fData>( do_update, val, parm );
				parm->setValue( new IECore::Box3fData(val) );
				break;
			}

			// Box3d
			case IECore::Box3dParameterTypeId:
			{
				float vals[6];
				evalFloats( parm_name.c_str(), vals, now );
				Imath::Box3d val( Imath::V3d( vals[0], vals[1], vals[2] ),
									Imath::V3d( vals[3], vals[4], vals[5] ) );
				checkForUpdate<Imath::Box3d, Box3dData>( do_update, val, parm );
				parm->setValue( new IECore::Box3dData(val) );
				break;
			}

			// Compound
			case IECore::CompoundParameterTypeId:
			{
				std::cerr << "TODO: need to add code to evaluate compoundParameters and it's children." << __FILE__ << ", " << __LINE__ << std::endl;
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

/// Utility class which loads a Procedural from Disk
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

		if ( IECore::runTimeCast<IECore::ParameterisedProcedural>(new_procedural)==0 &&
				IECore::runTimeCast<IECore::Op>(new_procedural)==0 )
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

const std::vector<std::string> &SOP_ParameterisedHolder::classNames()
{
	return m_cachedNames;
}


std::vector<std::string> SOP_ParameterisedHolder::classNames( const LoaderType &type, const std::string &matchString )
{
	IECorePython::ScopedGILLock lock;
	std::vector<std::string> class_names;
	try
	{
		std::string python_cmd;
		switch( type )
		{
			case OP_LOADER:
			{
				python_cmd = boost::str( format("IECore.ClassLoader.defaultOpLoader().classNames(\"\%s\")") % matchString );
				break;
			}
			case PROCEDURAL_LOADER:
			{
				python_cmd = boost::str( format("IECore.ClassLoader.defaultProceduralLoader().classNames(\"\%s\")") % matchString );
				break;
			}
			default:
			{
				return class_names;
				break;
			}
		}

		object result = CoreHoudini::evalPython( python_cmd );
		boost::python::list names = extract<boost::python::list>(result)();
		class_names.clear();
		for ( unsigned int i=0; i<names.attr("__len__")(); ++i )
		{
			class_names.push_back( extract<std::string>(names[i]) );
		}
	}
	catch( ... )
	{
		PyErr_Print();
	}
	return class_names;
}

std::vector<int> SOP_ParameterisedHolder::classVersions( const LoaderType &loader_type, const std::string &type )
{
	IECorePython::ScopedGILLock lock;
	std::vector<int> class_versions;
	try
	{
		std::string python_cmd;
		switch( loader_type )
		{
			case OP_LOADER:
			{
				python_cmd = boost::str( format("IECore.ClassLoader.defaultOpLoader().versions(\"\%s\")") % type );
				break;
			}
			case PROCEDURAL_LOADER:
			{
				python_cmd = boost::str( format("IECore.ClassLoader.defaultProceduralLoader().versions(\"\%s\")") % type );
				break;
			}
			default:
			{
				return class_versions;
				break;
			}
		}

		object result = CoreHoudini::evalPython( python_cmd );
		boost::python::list versions = extract<boost::python::list>(result)();
		class_versions.clear();
		for ( unsigned int i=0; i<versions.attr("__len__")(); ++i )
		{
			class_versions.push_back( extract<int>(versions[i]) );
		}
	}
	catch( ... )
	{
		PyErr_Print();
	}
	return class_versions;
}

int SOP_ParameterisedHolder::defaultClassVersion( const LoaderType &loader_type, const std::string &type )
{
	// just return the highest version we find on disk
	std::vector<int> versions = classVersions( loader_type, type );
	if ( versions.size()==0 )
		return -1;
	else
		return versions[versions.size()-1];
}

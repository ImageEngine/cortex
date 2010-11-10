//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H
#define IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H

// Houdini
#include "SOP/SOP_Node.h"

// Cortex
#include "IECore/Parameter.h"
#include "IECore/CompoundParameter.h"

namespace IECoreHoudini
{
	/// Class representing a SOP node acting as a holder for the
	/// abstract Parameterised class. SOP_ProceduralHolder inherits
	/// directly from this.
	class SOP_ParameterisedHolder : public SOP_Node
	{
		public :

			enum LoaderType
			{
				OP_LOADER = 0,
				PROCEDURAL_LOADER
			};

			/// Ctor
			SOP_ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op );
			/// Dtor
			virtual ~SOP_ParameterisedHolder();

			/// Sets a parameterised on this holder
			void setParameterisedDirectly( IECore::RunTimeTypedPtr p );
			virtual void setParameterised( IECore::RunTimeTypedPtr p, const std::string &type, int version ) = 0;

			/// Gets the parameterised held by this holder
			IECore::RunTimeTypedPtr getParameterised();

			/// Returns whether or not this holder have a valid parameterised?
			bool hasParameterised();

			/// Updates all parameters on the parameterised from the Houdini
			/// SOP's parameter values.
			template <class T>
			bool updateParameters( T parameterised, float now )
			{
				updateParameter( parameterised->parameters(), now, "", true );
				bool doesRequireUpdate = m_requiresUpdate;
				m_requiresUpdate = false; // return doesRequireUpdate so clear this flag for next time
				return doesRequireUpdate;
			}

			/// Update a specific Cortex parameter using values from the
			/// corresponding Houdini SOP Parameter.
			/// \parm prefix A string prefix for the houdini parameter name
			/// \parm top_level This should be true if you know the parm is the top-level CompoundParameter
			void updateParameter( IECore::ParameterPtr parm, float now, std::string prefix="", bool top_level=false );

			/// Checks for changes in parameter values and
			/// flags a gui update if required.
			template <class T, class U>
			void checkForUpdate( bool do_update, T val, IECore::ParameterPtr parm )
			{
				if ( do_update )
				{
					IECore::IntrusivePtr<U> data = IECore::runTimeCast<U>( parm->getValue() );
					if ( val!=data->readable() )
					{
						m_requiresUpdate = true;
					}
				}
			}

			/// Method for loading a ParameterisedProcedural from disk
			IECore::RunTimeTypedPtr loadParameterised( const std::string &type, int version, const std::string &search_path );

			/// These control whether or not the gui type/version controls
			/// update the parameterised object - most of the time they do.
			void enableParameterisedUpdate(){ m_parameterisedUpdate = true; }
			void disableParameterisedUpdate(){ m_parameterisedUpdate = false; }
			bool doParameterisedUpdate(){ return m_parameterisedUpdate; }

			/// get the classes we could load
			void virtual refreshClassNames()=0;
			const std::vector<std::string> &classNames();

			static std::vector<std::string> classNames( const LoaderType &loader_type, const std::string &matchString );
			static std::vector<int> classVersions( const LoaderType &loader_type, const std::string &type );
			static int defaultClassVersion( const LoaderType &loader_type, const std::string &type );

		protected :
			
			bool m_requiresUpdate;

			// class type/version
			std::string m_className;
			int m_classVersion;
			IECore::RunTimeTypedPtr m_parameterised;
			std::vector<std::string> m_cachedNames;

		private :
		
			bool m_parameterisedUpdate; // this controls whether the parameterised is loaded if the type/version is changed in the gui
			std::string m_matchString; // our class loader match string
	};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H

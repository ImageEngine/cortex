//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef IECOREHOUDINI_PARAMETERISEDHOLDER_H
#define IECOREHOUDINI_PARAMETERISEDHOLDER_H

#include "IECore/Parameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

#include "IECoreHoudini/MessageHandler.h"
#include "IECoreHoudini/ParameterisedHolderInterface.h"

namespace IECoreHoudini
{

/// Class representing an OP node acting as a holder for the abstract Parameterised class.
template<typename BaseType>
class ParameterisedHolder : public BaseType, public ParameterisedHolderInterface
{
	public :
		
		ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~ParameterisedHolder();
		
		static PRM_Template parameters[];
		static CH_LocalVariable variables[];
		
		static PRM_Name pParameterisedClassCategory;
		static PRM_Name pParameterisedClassName;
		static PRM_Name pParameterisedVersion;
		static PRM_Name pParameterisedSearchPathEnvVar;
		static PRM_Name pMatchString;
		static PRM_Name pReloadButton;
		static PRM_Name pEvaluateParameters;
		static PRM_Name pSwitcher;
		
		static PRM_Default matchStringDefault;
		static PRM_Default switcherDefaults[];
		
		static PRM_ChoiceList classCategoryMenu;
		static PRM_ChoiceList classNameMenu;
		static PRM_ChoiceList classVersionMenu;
		
		//! @name className/version UI functions
		/// Dynamic menus, callbacks, and helper functions for the className/version parameters.
		/////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// \todo: the concept of class category is a hack to make the UI usable. remove this when
		/// Houdini supports cascading menus for parameters.
		static void buildClassCategoryMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * );
		static void buildClassNameMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * );
		static void buildVersionMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * );
		static int reloadClassCallback( void *data, int index, float time, const PRM_Template *tplate );
		static int reloadButtonCallback( void *data, int index, float time, const PRM_Template *tplate );
		static void classNames( const std::string searchPathEnvVar, const std::string &matchString, std::vector<std::string> &names );
		static void classVersions( const std::string className, const std::string searchPathEnvVar, std::vector<int> &versions );
		static int defaultClassVersion( const std::string className, const std::string searchPathEnvVar );
		//@}
		
		virtual const char *inputLabel( unsigned pos ) const;
		virtual unsigned minInputs() const;
		virtual unsigned maxInputs() const;
		
		//! @name ParameterisedHolderInterface implementation
		/////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Set the node to hold a particular Parameterised object. When using this version
		/// of setParameterised the node will not be able to preserve the object across scene
		/// save/load - this becomes your responsibility if it's necessary.
		virtual void setParameterised( IECore::RunTimeTypedPtr p );
		virtual void setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar );
		virtual bool hasParameterised();
		virtual IECore::RunTimeTypedPtr getParameterised();
		/// \todo: actually implement this once parameter handling is in c++
		virtual bool setNodeValues();
		virtual void setParameterisedValues( double time );
		//@}
	
	protected :
		
		/// Load the node from disk. It checks for className/version/searchPath values on the node
		/// and attempts to reload the parameterised class from disk
		virtual bool load( UT_IStream &is, const char *ext, const char *path );
		
		virtual IECore::MessageHandler *getMessageHandler();
		virtual void setMessageHandler( IECore::MessageHandler *handler );
		
		/// Update a specific Cortex parameter using values from the corresponding Houdini node parameter.
		/// @param prefix A string prefix for the houdini parameter name
		/// @param top_level This should be true if you know the parm is the top-level CompoundParameter
		void updateParameter( IECore::ParameterPtr parm, float now, std::string prefix="", bool top_level=false );
		
		/// Pushes the data from the incomming connections into the associated Cortex parameters.
		/// Derived classes should implement this method in a way appropriate to the context.
		virtual void setInputParameterValues( float now ) = 0;
		
		/// updates the input connections to match the current loaded parameters
		virtual void refreshInputConnections() = 0;
				
		/// A vector of IECore::Parameters which are passed through node inputs rather than PRM_Templates
		/// Should be filled by refreshInputConnections() if input parameters make sense for this context.
		IECore::CompoundParameter::ParameterVector m_inputParameters;
		
		/// Determines if the node is dirty
		bool m_dirty;
	
	private :
		
		/// creates and sets a particular type/version of class on this node
		void load( const std::string &className, int classVersion, const std::string &searchPathEnvVar, bool updateGUI=true );
		
		/// Method for loading a Parameterised object from disk
		IECore::RunTimeTypedPtr loadParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar );
		
		/// Checks for changes in parameter values and marks the node as dirty
		template <class T, class U>
		void checkForUpdate( bool do_update, T val, IECore::ParameterPtr parm )
		{
			if ( do_update )
			{
				typename U::Ptr data = IECore::runTimeCast<U>( parm->getValue() );
				if ( val != data->readable() )
				{
					m_dirty = true;
				}
			}
		}
		
		IECore::RunTimeTypedPtr m_parameterised;
		
		// stores the className of the currently loaded parameterised object (if any)
		std::string m_loadedClassName;
		
		IECore::MessageHandlerPtr m_messageHandler;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_PARAMETERISEDHOLDER_H

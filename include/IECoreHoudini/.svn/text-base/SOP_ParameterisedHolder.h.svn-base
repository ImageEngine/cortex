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

#ifndef IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H
#define IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H

#include "SOP/SOP_Node.h"

#include "IECore/Parameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

namespace IECoreHoudini
{

/// Class representing a SOP node acting as a holder for the abstract Parameterised class.
class SOP_ParameterisedHolder : public SOP_Node
{
	public :

		SOP_ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~SOP_ParameterisedHolder();

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

		/// Returns whether or not this SOP is holding a valid parameterised object
		bool hasParameterised();

		/// Set the SOP to hold a particular Parameterised object. When using this version
		/// of setParameterised the SOP will not be able to preserve the object across scene
		/// save/load - this becomes your responsibility if it's necessary.
		void setParameterised( IECore::RunTimeTypedPtr p );
		void setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar );
		
		/// Sets the values of the parameters of the held Parameterised object to reflect the values
		// of the attributes of the node.
		/// \todo: add setNodeValues as well
		void setParameterisedValues( double time );
		
		/// Gets the parameterised object held by this SOP
		IECore::RunTimeTypedPtr getParameterised();

	protected :

		/// Load the SOP from disk. It checks for className/version/searchPath values on the node
		/// and attempts to reload the parameterised class from disk
		virtual bool load( UT_IStream &is, const char *ext, const char *path );

		/// Update a specific Cortex parameter using values from the corresponding Houdini SOP Parameter.
		/// @param prefix A string prefix for the houdini parameter name
		/// @param top_level This should be true if you know the parm is the top-level CompoundParameter
		void updateParameter( IECore::ParameterPtr parm, float now, std::string prefix="", bool top_level=false );

		/// Pushes the geometry data from the incomming connections into the associated Cortex parameters.
		/// This method will cook the incomming nodes. If the input node derives from SOP_ParameterisedHolder,
		/// it's Cortex output will be passed through. If it is a native Houdini node, it will be converted
		/// using the appropriate FromHoudiniGeometryConverter.
		void setInputParameterValues( float now );
		
		/// Returns an IECoreHoudini::MessageHandler setup to use the standard SOP_Node messaging methods
		IECore::MessageHandler *messageHandler();
		
		/// A vector of IECore::Parameters which are passed through SOP inputs rather than PRM_Templates
		IECore::CompoundParameter::ParameterVector m_inputParameters;

		/// Determines if the node is dirty
		bool m_dirty;

	private :

		/// creates and sets a particular type/version of class on this sop
		void load( const std::string &className, int classVersion, const std::string &searchPathEnvVar, bool updateGUI=true );

		/// Method for loading a Parameterised object from disk
		IECore::RunTimeTypedPtr loadParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar );

		/// updates the input connections to match the current loaded parameters
		void refreshInputConnections();

		/// Checks for changes in parameter values and marks the node as dirty
		template <class T, class U>
		void checkForUpdate( bool do_update, T val, IECore::ParameterPtr parm )
		{
			if ( do_update )
			{
				IECore::IntrusivePtr<U> data = IECore::runTimeCast<U>( parm->getValue() );
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

#endif // IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H

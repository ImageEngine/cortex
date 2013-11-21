//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Weta Digital Limited. All rights reserved.
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

#ifndef IECORENUKE_LENSDISTORT_H
#define IECORENUKE_LENSDISTORT_H

#include "DDImage/Knobs.h"
#include "DDImage/Row.h"
#include "DDImage/Thread.h"
#include "DDImage/Iop.h"
#include "DDImage/Filter.h"
#include "IECore/LensModel.h"

#define IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS 30

namespace IECoreNuke
{

/// LensDistort
/// Uses the IECore::LensModel libraries to distort or undistort a plate or sequence.
/// The LensDistort node provides a nuke interface to the IECore::LensDistort libraries.
/// It queries any registered lens models, displaying them within the pull-down menu of the "lens model"
/// knob. When a lens model is selected the node will dynamically create the required knobs on the
/// UI panel. An additional knob has been added to allow the input of a file sequence of lens models
/// which have been serialized into .cob files.
///
/// Weta Digitals LensDistortion node was referenced when designing this node.
/// Their source code is available freely at: https://github.com/wetadigital/lensDistortion_3de
class LensDistort : public DD::Image::Iop
{
	/// PluginAttribute
	/// A small struct for maintaining a list of the attributes on the current lens model.
	struct PluginAttribute
	{
		public:
			
			PluginAttribute( std::string name ) :
				m_name( name ),
				m_low( 0. ),
				m_high( 1. )
			{};
			
			PluginAttribute() :
				m_name( "Unused" ),
				m_low( 0. ),
				m_high( 1. )
			{};
			
			std::string m_name;
			double m_low;
			double m_high;
	};
	
	typedef std::vector< PluginAttribute > PluginAttributeList;
	
	public:

		enum
		{
			Distort,
			Undistort
		};
		
		LensDistort( Node* node );

		//! @name Nuke Virtual Methods
		//////////////////////////////////////////////////////////////
		//@{
		virtual void knobs( DD::Image::Knob_Callback f );
		virtual int knob_changed( DD::Image::Knob* k );
		virtual void append( DD::Image::Hash &hash );
		virtual void _request( int x, int y, int r, int t, DD::Image::ChannelMask channels, int count );
		virtual const char *Class() const;
		virtual const char *node_help() const;
		virtual void _validate( bool for_real );
		virtual void engine( int y, int x, int r, DD::Image::ChannelMask channels, DD::Image::Row & outrow );
		//@}

		static const Iop::Description m_description;
		static DD::Image::Op *build( Node *node );
		
	private:
		
		//////////////////////////////////////////////////////////////
		// Private Methods
		
		/// Returns an array of const char* which is populated
		/// with the available lens models. The names keep
		/// the order that the lens models are held within the
		/// IECore::LensModel::lensModels() list.
		static const char ** modelNames();
		/// Returns the index of a particular lens model within the
		/// IECore::LensModel::lensModels() list. 
		static int indexFromModelName( const char *name );
		/// Returns the index of the current lens model within the
		/// IECore::LensModel::lensModels() list.
		int currentLensModelIndex() const;
		/// Sets the current distortion to the lens model returned
		/// by passing 'parameters' into the IECore::LensModel::create()
		/// factory function.
		void setLensModel( IECore::ConstCompoundObjectPtr parameters );
		/// Sets the current distortion to the lens model with a name 'modelName'.
		void setLensModel( std::string modelName );
		/// Updates the internal list of lens parameters (and their associated knobs)
		/// to those defined within the current lens models.
		/// @param updateKnobsFromParameters If this value is true then all knobs will
		///		   be updated to the new lens model's parameters and any existing knob values
		///        will be discarded. If this value is false then the values of common
		///        parameters between the current and new lens model will be retained.
		void updateLensModel( bool updateKnobsFromParameters = false );
		/// Returns true if there is text in the file sequence knob.
		/// The contents of the knob are returned in the attribute 'path'.
		bool fileSequencePath( std::string& path );
		/// Checks that the file sequence is valid and then loads the required file from it.
		/// File sequences of the format path.#.ext and path.%0Xd.ext will have their
		/// wild card characters replaced and set to the current frame.
		/// @param returnPath The path of the file that has been loaded.
		/// @return Whether or not the file path was successful.
		bool setLensFromFile( std::string &returnPath );
		
		
		//! @name Lens Parameter Convenience Members 
		//////////////////////////////////////////////////////////////
		/// To make the knobs on the UI look like they have the name of the lens parameter
		/// that they are representing on the LensModel, we set their label to display
		/// the parameter name when a new lens model is selected or updateUI() is called.
		/// In reality, each knob isactually named with the convention "lensParamX",
		/// where 'X' is the index of the lens parameter on the lens model.
		/// These methods are provided for convenience to allow the label of a knob
		/// (which is the same as the lens model's parameter name) to be converted
		/// to the knobs actual name and vice-versa.
		//@{
		/// Returns the name of a knob that represents a lens parameter at index 'i' on the lens model.
		std::string parameterKnobName( unsigned int i ) const;
		/// Returns the name of the associated parameter for a knob with a given name.	
		std::string parameterNameFromKnobName( std::string knobName ) const;
		/// Updates the knobs so that their labels correspond to their parameter's name, sets their visibility
		/// and makes then read-only if the parameters for the lens model are being read from a file.
		void updateUI();
		//@}
	
		//////////////////////////////////////////////////////////////
		// Private Members
		
		/// A flag to indicate where there is any text in the "lensFileSequence" knob.
		bool m_useFileSequence;
		
		/// A flag that is set when a valid file sequence has been entered into the "lensFileSequence" knob.
		bool m_hasValidFileSequence;
		
		/// A list of the attributes that the plugin uses.
		PluginAttributeList m_pluginAttributes;
		
		//! @name Multi-Threading members
		/// As we can't assume that any derived classes of the IECore::LensModel
		/// class are thread safe, we make multiple instances of some members
		/// so that each thread has it's own.
		/// For example, we create one instance of each lens model per thread and
		/// store each of the instances in the m_lensModels member.
		//////////////////////////////////////////////////////////////
		//@{
		/// The maximum number of threads that we are going to use in parallel.
		const int m_nThreads;
		/// Plugin loaders. We need one of these per thread in
		/// case the LensModel is not thread safe.
		std::vector< IECore::LensModelPtr > m_lensModels;
		/// Locks for each LensModel object.
		DD::Image::Lock* m_locks;
		//@}
		
		//! @name Knob Storage Members
		/// These members hold the values of the various nuke knobs.
		//////////////////////////////////////////////////////////////
		//@{
		/// Path that holds the file sequence string.
		const char *m_assetPath;
		/// The method of filtering. Defined by the 'filter' knob.
		DD::Image::Filter m_filter;
		/// Which lens model we are currently using. This is an index into
		/// the IECore::LensModel::lensModels() list.
		int m_lensModel;
		/// Distort or undistort.
		int m_mode;
		/// Holds the values for the lens model's parameters.
		double m_knobData[IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS];
		//@}
};

};

#endif

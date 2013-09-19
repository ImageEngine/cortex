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

namespace IECoreNuke
{

/// LensDistort
/// Uses the IECore::LensModel libraries to distort or undistort a plate or sequence.
/// The LensDistort node provides a nuke interface to the IECore::LensDistort libraries.
/// It queries any registered lens models, displaying them within the pull-down menu of the "lens model"
/// knob. When a lens model is selected the node will dynamically create the required knobs on the
/// UI panel. An additional knob has been added to allow the input of a sequence of lens models
/// which have been serialized into .cob files.
///
/// Weta Digitals LensDistortion node was referenced when designing this node.
/// Their source code is available freely at: https://github.com/wetadigital/lensDistortion_3de
class LensDistort : public DD::Image::Iop
{
	/// PluginAttribute
	/// A small struct for maintaining a list of the knobs on the UI and their values.
	struct PluginAttribute
	{
		public:
			
			PluginAttribute( std::string name, double defaultValue = 0. ) :
				m_name( name ),
				m_knob( NULL ),
				m_value( defaultValue ),
				m_low( 0. ),
				m_high( 1. )
			{};
			
			PluginAttribute() :
				m_name( "Unused" ),
				m_knob( NULL ),
				m_value( 0. ),
				m_low( 0. ),
				m_high( 1. )
			{};
			
			std::string m_name;
			DD::Image::Knob *m_knob;
			std::string m_script;
			double m_value;
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
		
		inline int getLensModel() const { return (int)knob("model")->get_value(); };
		inline PluginAttributeList &attributeList() { return m_pluginAttributes; };
		
		virtual void knobs( DD::Image::Knob_Callback f );
		virtual int knob_changed( DD::Image::Knob* k );
		virtual void append( DD::Image::Hash &hash );
		virtual void _request( int x, int y, int r, int t, DD::Image::ChannelMask channels, int count );
		virtual const char *Class() const;
		virtual const char *node_help() const;
		virtual void _validate( bool for_real );
		virtual void engine( int y, int x, int r, DD::Image::ChannelMask channels, DD::Image::Row & outrow );
		
		static void addDynamicKnobs( void*, DD::Image::Knob_Callback f );
		static const Iop::Description m_description;
		static DD::Image::Op *build( Node *node );
		
	private:
		
		/// Returns an array of const char* which is populated with the available lens models.
		static const char ** modelNames();
		static int indexFromModelName( const char *name );
	
		/// Sets the lens model to use by calling the appropriate creators..
		void setLensModel( IECore::ConstCompoundObjectPtr parameters );
		void setLensModel( std::string modelName );

		/// Updates the internal list of lens parameters (and their associated knobs) to those defined within the current lens models.
		/// This method should only be called at the end of setLensModel.
		/// @param updateKnobsFromParameters If this value is true then all knobs will be updated to the new lens model's parameters and any existing knob values.
		/// will be discarded. If this value is false then the values of common parameters between the current and new lens model will be copied across.
		void updateLensModel( bool updateKnobsFromParameters = false );
		
		/// Returns true if there is text in the file sequence knob. The contents of the knob are returned in the attribute 'path'.
		bool getFileSequencePath( std::string& path );

		/// Checks that the file sequence is valid and then loads the required file from it. File sequences of the format path.#.ext and path.%0Xd.ext will have their
		/// wild card characters replaced and set to the current frame.
		/// @param returnPath The path of the file that has been loaded.
		/// @return Whether or not the file path was successful.
		bool setLensFromFile( std::string &returnPath );
		
		/// The maximum number of threads that we are going to use in parallel.
		const int m_nThreads;
		
		/// Plugin loaders. We need one of these per threads to make the lens lib thread safe.
		std::vector< IECore::LensModelPtr > m_model;
		
		/// locks for each LensModel object
		DD::Image::Lock* m_locks;
		
		/// A list of the attributes that the plugin uses.
		PluginAttributeList m_pluginAttributes;
		
		/// Which lens model we are currently using.
		int m_lensModel;
		
		/// Used to track the number of knobs created by the previous pass, so that the same number can be deleted next time.
		int m_numNewKnobs;
		
		/// The method of filtering. Defined by the 'filter' knob.
		DD::Image::Filter m_filter;
	
		/// Pointer to the 'mode' knob.
		DD::Image::Knob *m_kModel;

		/// Distort or undistort.
		int m_mode;
		
		/// All knobs below this one are dynamic.
		DD::Image::Knob* m_lastStaticKnob;

		/// Path that holds the file sequence information.
		const char *m_assetPath;
		
		/// Set within the knob_changed method to indicate whether a valid file sequence has been entered.		
		bool m_hasValidFileSequence;
		bool m_useFileSequence;
};

};

#endif

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

#ifndef IECORE_LENSMODEL_H
#define IECORE_LENSMODEL_H

#include <map>

#include "IECore/Export.h"
#include "IECore/Object.h"
#include "IECore/Parameter.h"
#include "IECore/Parameterised.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/TypeIds.h"
#include "boost/format.hpp"

namespace IECore
{

IE_CORE_FORWARDDECLARE( CompoundObject )
IE_CORE_FORWARDDECLARE( LensModel );

/// LensModel:
///
/// The LensModel class is the base for all IECore lens models which implements a simple framework for writing arbitrary lens distortion models.
/// A simple interface is exposed in the form of the validate(), distort(), undistort() methods which should be reimplemented by any subclass to
/// evaluate the parameters and perform the distortion of a point in UV space.
///
/// The static LensModel::create() methods are implemented as factory functions for creating LensModels. Derived classes should define a static instance
/// of the LensModel::LensModelRegistration<> which will register it will the factory functions. A vector of all registered lens models can be returned by
/// calling LensModel::lensModels(). As the factory functions were implemented to ensure correct initialization of the lens models, all derived classes
/// should make their default constructor protected and add LensModel as a friend class.
///
/// LensModels can be serialized by just writing out the CompoundObject that contains it's parameters. LensModels can be easily loaded by just
/// passing the same compound object containing it's parameter's values into LensModel::create().
///
/// To use an object derived from LensModel:
/// * First populate the parameters with values.
/// * Call validate() to validate the parameters and set up any internal state as necessary.
/// * Call distort(), undistort() or bounds() as desired to query distorted UV values.
///
class IECORE_API LensModel : public Parameterised
{
	public:

		enum
		{
			Distort = 0,
			Undistort = 1
		} DistortionMode;

		typedef LensModelPtr (*CreatorFn)( ConstCompoundObjectPtr data );

		IE_CORE_DECLARERUNTIMETYPED( LensModel, Parameterised );

		/// Calculates the distorted/undistorted data window of an image.
		/// The distorted bounding box is calculated by finding the smallest box that contains a set of
		/// distort points which were taken from the border of the undistorted bounding box.
		//! @param mode Distort/Undistort
		//! @param input The data window of the input
		//! @param width The width in pixels of the display window (also known as the format) of the input.
		//! @param height The height in pixels of the display window (also known as the format) of the input.
		Imath::Box2i bounds( int mode, const Imath::Box2i &input, int width, int height );

		//! @name Distort/Undistort Methods
		/// A set of methods to distort or undistort points and bounding boxes.
		//////////////////////////////////////////////////////////////
		//@{
		/// Compute should be called to set up the internal values. This method must be called
		/// before subsequent calls to distort(), undistort() and bounds() or their results are undefined.
		virtual void validate() = 0;

		/// Distorts a point in UV space of the range (0-1) where the lower left corner is 0,0.
		/// Should be implemented by derived classes to return the distorted UV coordinate.
		//! @param uv The undistorted point that will be distorted. Should be a 2D vector in pixel space.
		virtual Imath::V2d distort( Imath::V2d p ) = 0;

		/// Undistorts a point in UV space of the range (0-1) where the lower left corner is 0,0.
		/// Should be implemented by derived classes to return the undistorted UV coordinate.
		//! @param uv The distorted point that will be undistorted. Should be a 2D vector in pixel space.
		virtual Imath::V2d undistort( Imath::V2d p ) = 0;
		//@}

		//! @name Lens Model Registry
		/// A set of methods to query the available lens models and create them.
		//////////////////////////////////////////////////////////////
		//@{
		/// Instantiates a new LensModel object and initialises it with the parameters held within lensParams.
		/// @param lensParams Must contain a string object called "lensModel" that contains the name of
		/// the registered lens model to instantiate.
		static LensModelPtr create( ConstCompoundObjectPtr lensParams );

		/// Instantiates a new LensModel object from just the lens model's typeName.
		/// @param name The typeName() of the registered lens model.
		static LensModelPtr create( const std::string &name );

		/// Instantiates a new LensModel object from it's typeId.
		/// @param name The typeId() of the registered lens model.
		static LensModelPtr create( TypeId id );

		/// Returns a vector of available lens models.
		static std::vector<std::string> lensModels();

	protected :

		/// The Default Constructor is protected as the LensModel::create() method
		/// should be used to instantiate a new LensModel instead. This is the
		/// only way that we can ensure that the LensModel is initialized correctly.
		LensModel();
		~LensModel() override;

		/// Instantiating an instance of LensModelRegistration<YourLensModelClass>
		/// registers your model to a mapping of LensModel names
		/// to their creator functions. This allows the LensModel::create()
		/// methods to return instantiations of the LensModel.
		/// The best way to ensure that your lens model is registered is to create a
		/// static instantiation of LensModelRegistration within YourLensModelClass.
		template<class T>
		struct LensModelRegistration
		{
			public:
				/// Registers the lens model
				LensModelRegistration();

			private:
				/// Returns a new instance of the LensModel class.
				static LensModelPtr creator( ConstCompoundObjectPtr data = nullptr );
		};

	private:

		typedef std::map< std::string, CreatorFn > CreatorMap;

		/// Registration mechanism for Lens Model classes.
		static CreatorMap& creators();

};

IE_CORE_DECLAREPTR(LensModel);

}

#include "LensModel.inl"

#endif // IECORE_LENSMODEL_H


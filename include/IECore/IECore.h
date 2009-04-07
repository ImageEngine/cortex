//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_CORE_H
#define IE_CORE_CORE_H

#include <string>

/// This namespace contains all components of the core library.
namespace IECore
{

/// Returns the major version for the IECore library
int majorVersion();
/// Returns the minor version for the IECore library
int minorVersion();
/// Returns the patch version for the IECore library
int patchVersion();
/// Returns a string of the form "major.minor.patch"
const std::string &versionString();

/// Returns true if IECore was build with TIFF support
bool withTIFF();
/// Returns true if IECore was built with JPEG support
bool withJPEG();
/// Returns true if IECore was built with FreeType suppport
bool withFreeType();

}

//! \mainpage
///
/// The IECore library provides a C++ and Python framework for visual
/// effects development. It was originally developed at Image Engine and
/// is now developed as part of the open source Cortex project. The sections
/// below provide a fairly high level overview of the library, and a few useful
/// jumping off points for learning more.
///
/// \ref mainPageMathSection <br>
/// \ref mainPageMemoryManagementSection <br>
/// \ref mainPageRTTISection <br>
/// \ref mainPageObjectSection <br>
/// \ref mainPageDataSection <br>
/// \ref mainPageParameterisedSection <br>
/// \ref mainPageIOSection <br>
/// \ref mainPageRenderingSection <br>
/// \ref mainPageAlgorithmsSection <br>
/// \ref mainPagePythonSection <br>
/// \ref mainPageApplicationSupportSection <br>
///
/// \section mainPageMathSection Math
///
/// Basic math types are provided by the Imath library which comes as part
/// of the <a href="http://www.openexr.org">OpenEXR package</a>. Any additional
/// types are designed along similar lines - for example the templated
/// TransformationMatrix class.
///
/// The \link IECore::VectorTraits VectorTraits \endlink and \link IECore::MatrixTraits MatrixTraits \endlink classes provide a template based mechanism
/// for querying the dimensions of vector and matrix classes, and for setting and
/// getting their elements. This allows coding compatible with both the Imath types
/// and other 3rd party math types. The VectorOps.h and MatrixOps.h files then
/// provide common operations on top of this lower level access, allowing classes
/// such as \link IECore::KDTree KDTree \endlink and \link IECore::PerlinNoise PerlinNoise \endlink to operate both on native IECore types and
/// application specific types such as the Maya MPoint and MVector classes.
/// 
/// The VectorOps.h and MatrixOps.h code provides a pretty ugly c-style function
/// based interface - functional enough to provide library components like the KDTree
/// but not useable enough for everyday coding. It may be better in the future to
/// create object oriented template wrappers for 3rd party maths types -
/// these wrappers could still use the Traits classes for low level access.
///
/// \section mainPageMemoryManagementSection Memory Management
///
/// IECore uses reference counting for any reasonably complex class. The \link IECore::RefCounted RefCounted \endlink
/// base class provides an internal reference count for the object, and boost::intrusive_ptr
/// is used most places where raw pointers would otherwise be used. This means that
/// generally you don't need to worry about memory management - objects die appropriately
/// when the last boost::intrusive_ptr pointing to them goes out of scope.
///
/// For convenience, all classes define typedefs of the form ClassPtr, ConstClassPtr,
/// Class::Ptr and Class::ConstPtr - these should be used in preference to the
/// clunkier boost::intrusive_ptr<Class> syntax.
///
/// \code
///	{
///		CompoundDataPtr c = new CompoundData;
/// 		{
/// 			DataPtr d = new IntVectorData;
///			c->writable()["d"] = d;
///			// d goes out of scope, but the CompoundData still
///			// holds a reference to the IntVectorData
///		}
///		// the original IntVectorData object dies when it's
///		// replaced by the FloatVectorData.
/// 		c->writable()["d"] = new FloatVectorData;
///		// all the objects die appropriately when c goes out of scope
///	}
/// \endcode
///
/// \section mainPageRTTISection RTTI
///
/// IECore uses its own mechanism for type identification. This comes in the form
/// of the \link IECore::RunTimeTyped RunTimeTyped \endlink base class, and the runTimeCast function.
///
/// \code
/// void incrementIntData( ConstDataPtr d )
/// {
///		ConstIntDataPtr f = runTimeCast<const IntData>( d );
///		if( f )
///		{
///			f->writable()++;
///		}
///		else
///		{
///			cerr << "I didn't want an instance of class " << d->typeName() << endl;
///		}
/// }
/// \endcode
///
/// See the documentation for the RunTimeTyped class for the rationale behind not using the built in C++ typeinfo
/// system.
///
/// \section mainPageObjectSection The Object class
///
/// The \link IECore::Object Object \endlink class defines methods for serialisation and copying, and
/// provides a factory mechanism.
///
/// \code
///	ObjectPtr i0 = new IntData;
///	ObjectPtr i1 = Object::create( IntData::staticTypeId() );
///	ObjectPtr i2 = i1->copy();
///
///	i1->save( "i.cob" );
///	IntDataPtr i3 = runTimeCast<IntData>( Object::load( "i.cob" ) );
/// \endcode
///
/// \section mainPageDataSection The Data classes
///
/// The \link IECore::Data Data \endlink classes derive from \link IECore::Object Object \endlink and provide simple wrappers around
/// basic datatypes and std::vectors of those datatypes. They implement the
/// copy() method in a lazy manner, so you don't pay for a copy until you
/// modify it. For this reason, you use the readable() and writable() methods
/// to get access to the underlying data.
///
/// \code
///	IntVectorDataPtr i = new IntVectorData;
///	i->writable().resize( 10000 );
///	IntVectorDataPtr ii = i->copy();
///	// at this point, no additional memory has been used for the copy
///	cerr << ii->readable()[0];
///	// we only did a read, so still no memory overhead
///	ii->writable()[0] = 0;
///	// we called writable, so have to pay for the copy now to avoid
///	// modifying the original data
///	\endcode
///
/// \section mainPageParameterisedSection The Parameter and Parameterised classes
///
/// The \link IECore::Parameter Parameter \endlink classes provide a means of describing values to be passed to some
/// process, and the Parameterised class stores a bunch of parameters. Parameters provide
/// enough information for different host applications to represent them appropriately.
/// The IECore library provides a means of formatting parameter help for display in the
/// terminal and for parsing parameter values from a command line. The IECoreMaya library
/// allows parameters to be represented as attributes on Maya nodes, and creates custom
/// attribute editor layouts for them.
///
/// Concrete implementations of the \link IECore::Parameterised Parameterised \endlink class include the \link IECore::Op Op \endlink subclasses, which
/// have an operate() method to compute some result from the input Parameters, the
/// \link IECore::Renderer::Procedural Procedural \endlink classes which define an interface for the rendertime generation of geometry,
/// and the \link IECore::Reader Reader \endlink and \link IECore::Writer Writer \endlink classes which are described briefly below.
///
/// \section mainPageIOSection IO
///
/// As we saw above, Objects have a built in serialisation mechanism. This makes use of
/// the \link IECore::IndexedIOInterface IndexedIOInterface \endlink, which defines a mechanism for the creation of random access
/// files containing a hierarchy of named data items. Currently the main implementation
/// of this interface is \link IECore::FileIndexedIO FileIndexedIO \endlink, with 
/// \link IECore::FileSystemIndexedIO FileSystemIndexedIO \endlink implementing a reference
/// solution of use for debugging and the like.
///
/// The \link IECore::AttributeCache AttributeCache \endlink and \link IECore::InterpolatedCache 
/// InterpolatedCache \endlink sit on top of IndexedIOInterface and provide
/// a simple framework useable for vertex caching, transform caching and more - in fact any
/// Object subclass can be stored as an element of a cache - this is one of the benefits
/// of combining the object serialisation and random access caching code.
///
/// For input and output of third party file formats there are the \link IECore::Reader 
/// Reader \endlink and \link IECore::Writer Writer \endlink classes.
/// These contain factory mechanisms for obtaining Readers and Writers given a filename with
/// an extension, and use Parameters to control any conversions during the io process.
/// Currently implementations exist for several common image formats and for the Maya PDC file format.
///
/// \code
/// ObjectPtr o = Reader::create( "image.jpg" )->read();
/// Writer::create( o, "image.exr" )->write();
/// \endcode
///
/// \section mainPageRenderingSection Rendering
///
/// IECore defines the \link IECore::Renderer Renderer \endlink class for the description of scenes to a renderer. The IECoreRI::Renderer
/// subclass implements this class for RIB generation and use in DynamicLoad procedurals, and the IECoreGL::Renderer subclass implements
/// the same interface in terms of OpenGL, providing fast preview rendering which can be used to visualise procedural results within
/// an IECoreMaya::ProceduralHolder node.
///
/// The Renderable class and subclasses such as \link IECore::MeshPrimitive MeshPrimitive \endlink
/// provide objects which can be manipulated with Ops and which can describe themselves to Renderers.
///
/// \section mainPageAlgorithmsSection Algorithms
///
/// \link IECore::KDTree KDTree \endlink and \link IECore::BoundedKDTree BoundedKDTree \endlink
/// structures allow for fast spatial queries on large data sets.
///
/// \link IECore::PerlinNoise PerlinNoise \endlink implements the classic noise function for arbitrary dimensions.
///
/// Fast closest point and ray intersection queries can be performed on some of the classes derived
/// from \link IECore::Primitive Primitive \endlink, using an instance of a 
/// \link IECore::PrimitiveEvaluator PrimitiveEvaluator \endlink.
///
/// A fast, robust implementation of \link IECore::MarchingCubes MarchingCubes \endlink is available for creating meshes from an
/// \link IECore::ImplicitSurfaceFunction ImplicitSurfaceFunction \endlink.
///
/// A templated class is avaiable to perform \link IECore::InverseDistanceWeightedInterpolation InverseDistanceWeightedInterpolation \endlink from
/// a set of scattered data points.
///
/// \section mainPagePythonSection Python
///
/// All of the IECore C++ classes are bound into Python using the <a href="http://www.boost.org/libs/python/doc/">boost::python</a> framework.
/// Many can actually be subclassed in python, making it very quick to implement a new Op
/// or Procedural.
///
/// \section mainPageApplicationSupportSection Application support
///
/// The generic functionality contained within IECore is interfaced with specific 3rd party applications and frameworks using additional
/// libraries. These libraries include \ref IECoreMaya, IECoreNuke, IECoreGL and IECoreTruelight.

#endif // IE_CORE_CORE_H

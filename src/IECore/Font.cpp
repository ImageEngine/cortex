//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include <algorithm>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include "IECore/Font.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/Triangulator.h"
#include "IECore/BezierAlgo.h"
#include "IECore/PolygonAlgo.h"
#include "IECore/BoxOps.h"
#include "IECore/TransformOp.h"
#include "IECore/MeshMergeOp.h"
#include "IECore/Group.h"
#include "IECore/MatrixTransform.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( Font );

////////////////////////////////////////////////////////////////////////////////
// Font::Mesher class implementation
////////////////////////////////////////////////////////////////////////////////

class Font::Mesher
{

	public :

		Mesher( FT_Pos curveTolerance );

		MeshPrimitivePtr mesh( FT_Outline *outline );

	private :

		FT_Pos m_curveTolerance;

		typedef vector<V3f> PointVector;
		typedef std::pair<PointVector::const_iterator, PointVector::const_iterator> Loop;
		typedef std::vector<Loop> LoopVector;
		typedef std::vector<Box2f> BoundVector;

		PointVector &currentPointVector();
		void addPoint( const V3f &p );

		// used in the subdivision of bezier curves
		class BezierCallback;

		// functions to be used as part of an FT_Outline_Funcs_ struct
		static int moveTo( const FT_Vector *to, void *that );
		static int lineTo( const FT_Vector *to, void *that );
		static int conicTo( const FT_Vector *control, const FT_Vector *to, void *that );
		static int cubicTo( const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *that );

		vector<PointVector> m_pointVectors;

};


// a functor used in subdividing bezier curves in the mesher
class Font::Mesher::BezierCallback
{
	public :

		BezierCallback( Font::Mesher *m )
			:	m_ignoredFirst( false ), m_mesher( m )
		{
		}

		void operator()( const V3f &p )
		{
			if( !m_ignoredFirst )
			{
				m_ignoredFirst = true;
			}
			else
			{
				m_mesher->addPoint( p );
			}
		}

	private :

		bool m_ignoredFirst;
		Font::Mesher *m_mesher;
};

Font::Mesher::Mesher( FT_Pos curveTolerance )
	:	m_curveTolerance( curveTolerance )
{
}

MeshPrimitivePtr Font::Mesher::mesh( FT_Outline *outline )
{
	// break the contours down into our m_pointVectors
	// structure
	FT_Outline_Funcs_ funcs;
	funcs.move_to = moveTo;
	funcs.line_to = lineTo;
	funcs.conic_to = conicTo;
	funcs.cubic_to = cubicTo;
	funcs.shift = 0;
	funcs.delta = 0;
	FT_Outline_Decompose( outline, &funcs, this );

	// reverse the contours if necessary
	if( !(outline->flags & FT_OUTLINE_REVERSE_FILL) )
	{
		for( unsigned i=0; i<m_pointVectors.size(); i++ )
		{
			std::reverse( m_pointVectors[i].begin(), m_pointVectors[i].end() );
		}
	}

	// sort into outlines and holes
	LoopVector outlines;
	BoundVector outlineBounds;
	LoopVector holes;
	BoundVector holeBounds;
	vector<bool> holeUsed;

	for( unsigned i=0; i<m_pointVectors.size(); i++ )
	{
		if( m_pointVectors[i].size() )
		{
			// freetype explicitly joins the last segment to the beginning
			// of the first, which gives us two coincident points. we don't
			// want that as it confuses the triangulator.
			m_pointVectors[i].resize( m_pointVectors[i].size() - 1 );
		}
		if( !m_pointVectors[i].size() )
		{
			continue;
		}
		if( polygonWinding( m_pointVectors[i].begin(), m_pointVectors[i].end() )==ClockwiseWinding )
		{
			holes.push_back( Loop( m_pointVectors[i].begin(), m_pointVectors[i].end() ) );
			Box3f b = polygonBound( m_pointVectors[i].begin(), m_pointVectors[i].end() );
			holeBounds.push_back( Box2f( V2f( b.min.x, b.min.y ), V2f( b.max.x, b.max.y ) ) );
			holeUsed.push_back( false );
		}
		else
		{
			outlines.push_back( Loop( m_pointVectors[i].begin(), m_pointVectors[i].end() ) );
			Box3f b = polygonBound( m_pointVectors[i].begin(), m_pointVectors[i].end() );
			outlineBounds.push_back( Box2f( V2f( b.min.x, b.min.y ), V2f( b.max.x, b.max.y ) ) );
		}
	}

	// triangulate each outline, along with any holes which should be associated with it
	MeshPrimitiveBuilder::Ptr b = new MeshPrimitiveBuilder;
	V3fTriangulator::Ptr t = new V3fTriangulator( b );

	for( unsigned i=0; i<outlines.size(); i++ )
	{
		LoopVector loops;
		loops.push_back( outlines[i] );
		for( unsigned j=0; j<holes.size(); j++ )
		{
			if( !holeUsed[j] )
			{
				if( boxContains( outlineBounds[i], holeBounds[j] ) )
				{
					// the containment test is a bit weak - we might need to check
					// that the edges of the outline and hole don't intersect either.
					loops.push_back( holes[j] );
					holeUsed[j] = true;
				}
			}
		}

		t->triangulate( loops.begin(), loops.end() );
	}

	return b->mesh();
}

Font::Mesher::PointVector &Font::Mesher::currentPointVector()
{
	return *(m_pointVectors.rbegin());
}

void Font::Mesher::addPoint( const V3f &p )
{
	PointVector &points = currentPointVector();
	if( !points.size() || !(*(points.rbegin())).equalWithAbsError( p, 1e-6 ) )
	{
		points.push_back( p );
	}
}

int Font::Mesher::moveTo( const FT_Vector *to, void *that )
{
	Mesher *m = (Mesher *)( that );
	m->m_pointVectors.push_back( PointVector() );
	m->addPoint( V3f( to->x, to->y, 0 ) );
	return 0;
}

int Font::Mesher::lineTo( const FT_Vector *to, void *that )
{
	Mesher *m = (Mesher *)( that );
	m->addPoint( V3f( to->x, to->y, 0 ) );
	return 0;
}

int Font::Mesher::conicTo( const FT_Vector *control, const FT_Vector *to, void *that )
{
	Mesher *m = (Mesher *)( that );
	BezierCallback c( m );
	bezierSubdivide(
		*(m->currentPointVector().rbegin()),
		V3f( control->x, control->y, 0 ),
		V3f( to->x, to->y, 0 ),
		m->m_curveTolerance,
		c
	);
	return 0;
}

int Font::Mesher::cubicTo( const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *that )
{
	Mesher *m = (Mesher *)( that );
	BezierCallback c( m );
	bezierSubdivide(
		*(m->currentPointVector().rbegin()),
		V3f( control1->x, control1->y, 0 ),
		V3f( control2->x, control2->y, 0 ),
		V3f( to->x, to->y, 0 ),
		m->m_curveTolerance,
		c
	);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Font::Implementation
////////////////////////////////////////////////////////////////////////////////

class Font::Implementation : public IECore::RefCounted
{

	public :
	
		Implementation( const std::string &fontFile )
			:	m_fileName( fontFile ), m_kerning( 1.0f ), m_curveTolerance( 0.01 ), m_pixelsPerEm( 0 )
		{
			FT_Error e = FT_New_Face( library(), fontFile.c_str(), 0, &m_face );
			if( e )
			{
				throw Exception( "Error creating new FreeType face." );
			}
			setResolution( 100 );
			m_meshes.resize( 128 );
			m_images.resize( 128 );
		}

		virtual ~Implementation()
		{
			FT_Done_Face( m_face );
		}
		
		const std::string &fileName() const
		{
			return m_fileName;
		}
		
		void setKerning( float kerning )
		{
			m_kerning = kerning;
		}

		float getKerning() const
		{
			return m_kerning;
		}

		void setCurveTolerance( float tolerance )
		{
			m_curveTolerance = tolerance;
			m_meshes.clear();
		}

		float getCurveTolerance() const
		{
			return m_curveTolerance;
		}

		void setResolution( float pixelsPerEm )
		{
			if( pixelsPerEm!=m_pixelsPerEm )
			{
				m_pixelsPerEm = pixelsPerEm;
				for( size_t i = 0, e = m_images.size(); i < e; i++ )
				{
					m_images[i] = 0;
				}
				FT_Set_Pixel_Sizes( m_face, (FT_UInt)pixelsPerEm, (FT_UInt)pixelsPerEm );
			}
		}

		float getResolution() const
		{
			return m_pixelsPerEm;
		}

		const MeshPrimitive *mesh( char c ) const
		{
			return cachedMesh( c )->primitive.get();
		}
		
		MeshPrimitivePtr mesh( const std::string &text ) const
		{
			MeshPrimitivePtr result = new MeshPrimitive;
			V3fVectorDataPtr pData = new V3fVectorData;
			pData->setInterpretation( GeometricData::Point );
			result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, pData );

			if( !text.size() )
			{
				return result;
			}

			MeshMergeOpPtr merger = new MeshMergeOp;
			merger->inputParameter()->setValue( result );
			merger->copyParameter()->setTypedValue( false );

			TransformOpPtr transformOp = new TransformOp;
			transformOp->copyParameter()->setTypedValue( false );
			M44fDataPtr matrixData = new M44fData;
			transformOp->matrixParameter()->setValue( matrixData );

			for( unsigned i=0; i<text.size(); i++ )
			{
				const Mesh *character = cachedMesh( text[i] );

				MeshPrimitivePtr primitive = character->primitive->copy();

				transformOp->inputParameter()->setValue( primitive );
				transformOp->operate();

				merger->meshParameter()->setValue( primitive );
				merger->operate();

				if( i<text.size()-1 )
				{
					V2f a = advance( text[i], text[i+1] );
					matrixData->writable().translate( V3f( a.x, a.y, 0 ) );
				}
			}

			return result;
		}

		GroupPtr meshGroup( const std::string &text ) const
		{
			GroupPtr result = new Group;

			if( !text.size() )
			{
				return result;
			}

			M44f transform;
			for( unsigned i=0; i<text.size(); i++ )
			{
				const Mesh *character = cachedMesh( text[i] );
				if( character->primitive->variableSize( PrimitiveVariable::Uniform ) )
				{
					GroupPtr g = new Group;
					g->addChild( character->primitive->copy() );
					g->setTransform( new MatrixTransform( transform ) );
					result->addChild( g );
				}

				if( i<text.size()-1 )
				{
					V2f a = advance( text[i], text[i+1] );
					transform.translate( V3f( a.x, a.y, 0 ) );
				}
			}

			return result;
		}

		Imath::V2f advance( char first, char second ) const
		{
			V2f a = cachedMesh( first )->advance;
			if( m_kerning!=0.0f )
			{
				FT_UInt left = FT_Get_Char_Index( m_face, first );
				FT_UInt right = FT_Get_Char_Index( m_face, second );
				FT_Vector kerning;
				FT_Error e = FT_Get_Kerning( m_face, left, right, FT_KERNING_UNSCALED, &kerning );
				if( !e )
				{
					a += m_kerning * V2f( kerning.x, kerning.y ) / m_face->units_per_EM;
				}
			}
			return a;
		}

		
		Imath::Box2f bound() const
		{
			float scale = 1.0f / (float)(m_face->units_per_EM);
			return Box2f(
				V2f( (float)m_face->bbox.xMin * scale, (float)m_face->bbox.yMin * scale ),
				V2f( (float)m_face->bbox.xMax * scale, (float)m_face->bbox.yMax * scale )
			);
		}

		Imath::Box2f bound( char c ) const
		{
			Imath::Box3f b = cachedMesh( c )->bound;
			return Imath::Box2f( Imath::V2f( b.min.x, b.min.y ), Imath::V2f( b.max.x, b.max.y ) );
		}

		Imath::Box2f bound( const std::string &text ) const
		{
			Imath::Box2f result;
			if( !text.size() )
			{
				return result;
			}

			V2f translate( 0 );
			for( unsigned i=0; i<text.size(); i++ )
			{
				Box2f b = bound( text[i] );
				b.min += translate;
				b.max += translate;
				result.extendBy( b );
				if( i<text.size()-1 )
				{
					translate += advance( text[i], text[i+1] );
				}
			}

			return result;
		}

		const ImagePrimitive *image( char c ) const
		{
			return cachedImage( c );	
		}
		
		ImagePrimitivePtr image() const
		{
			Box2i charDisplayWindow = boundingWindow();
			int charWidth = charDisplayWindow.size().x + 1;
			int charHeight = charDisplayWindow.size().y + 1;

			int width = charWidth * 16;
			int height = charHeight * 8;
			Box2i window( V2i( 0 ), V2i( width-1, height-1 ) );

			ImagePrimitivePtr result = new ImagePrimitive( window, window );
			FloatVectorDataPtr luminanceData = result->createChannel<float>( "Y" );
			float *luminance = &*(luminanceData->writable().begin());

			for( unsigned c=0; c<128; c++ )
			{
				ConstImagePrimitivePtr charImage = image( (char)c );
				ConstFloatVectorDataPtr charLuminanceData = charImage->getChannel<float>( "Y" );
				assert( charLuminanceData );
				const float *charLuminance = &*(charLuminanceData->readable().begin());
				const Box2i &charDataWindow = charImage->getDataWindow();
				assert( charDisplayWindow == charImage->getDisplayWindow() );

				V2i dataOffset = charDataWindow.min - charDisplayWindow.min;

				int cx = c % 16;
				int cy = c / 16;
				float *outBase= luminance + (cy * charHeight + dataOffset.y ) * width + cx * charWidth + dataOffset.x;
				for( int y=charDataWindow.min.y; y<=charDataWindow.max.y; y++ )
				{
					float *rowOut = outBase + (y-charDataWindow.min.y) * width;
					for( int x=charDataWindow.min.x; x<=charDataWindow.max.x; x++ )
					{
						*rowOut++ = *charLuminance++;
					}
				}
			}
			return result;
		}

	private :

		std::string m_fileName;

		FT_Face m_face;
		float m_kerning;
		float m_curveTolerance;
		float m_pixelsPerEm;

		struct Mesh
		{
			ConstMeshPrimitivePtr primitive;
			Box3f bound;
			V2f advance;
		};

		typedef boost::shared_ptr<Mesh> MeshPtr;
		typedef boost::shared_ptr<const Mesh> ConstMeshPtr;
		mutable std::vector<MeshPtr> m_meshes;

		mutable std::vector<ConstImagePrimitivePtr> m_images;

		const Mesh *cachedMesh( char c ) const
		{
			// see if we have it cached
			if( m_meshes[c] )
			{
				return m_meshes[c].get();
			}

			// not in cache, so load it
			FT_Load_Char( m_face, c, FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE );

			// get the mesh
			Mesher m( (FT_Pos)(m_curveTolerance * m_face->units_per_EM) );
			MeshPrimitivePtr primitive = m.mesh( &(m_face->glyph->outline) );

			// transform it so an EM is 1 unit
			M44f transform; transform.scale( V3f( 1.0f / m_face->units_per_EM ) );
			TransformOpPtr transformOp = new TransformOp;
			transformOp->inputParameter()->setValue( primitive );
			transformOp->matrixParameter()->setValue( new M44fData( transform ) );
			transformOp->copyParameter()->setTypedValue( false );
			transformOp->operate();

			// put it in the cache
			MeshPtr mesh( new Mesh );
			mesh->primitive = primitive;
			mesh->bound = primitive->bound();
			mesh->advance = V2f( m_face->glyph->advance.x, m_face->glyph->advance.y ) / m_face->units_per_EM;
			m_meshes[c] = mesh;

			// return it
			return mesh.get();
		}


		const ImagePrimitive *cachedImage( char c ) const
		{

			// see if we have it cached
			if( m_images[c] )
			{
				return m_images[c].get();
			}

			// not in cache, so load it
			FT_Load_Char( m_face, c, FT_LOAD_RENDER );

			// convert to ImagePrimitive
			FT_Bitmap &bitmap = m_face->glyph->bitmap;
			assert( bitmap.num_grays==256 );
			assert( bitmap.pixel_mode==FT_PIXEL_MODE_GRAY );

			Box2i displayWindow = boundingWindow();

			// datawindow is the bitmap bound, but adjusted to account for the y transformation described
			// in boundingWindow.
			Box2i dataWindow(
				V2i( m_face->glyph->bitmap_left, -m_face->glyph->bitmap_top ),
				V2i( m_face->glyph->bitmap_left + bitmap.width - 1, -m_face->glyph->bitmap_top + bitmap.rows - 1 )
			);

			ImagePrimitivePtr image = new ImagePrimitive( dataWindow, displayWindow );

			FloatVectorDataPtr luminanceData = image->createChannel<float>( "Y" );
			float *luminance = &*(luminanceData->writable().begin());
			for( int y=0; y<bitmap.rows; y++ )
			{
				unsigned char *row = bitmap.buffer + y * bitmap.pitch;
				/// \todo Do we have to reverse gamma correction to get a linear image?
				for( int x=0; x<bitmap.width; x++ )
				{
					*luminance++ = *row++ / 255.0f;
				}
			}

			// put it in the cache
			m_images[c] = image;

			// return it
			return image.get();
		}
		
		Imath::Box2i boundingWindow() const
		{
			// display window is the maximum possible character bound. unfortunately the ImagePrimitive
			// defines it's windows with y increasing from top to bottom, whereas the freetype coordinate
			// systems have y increasing in the bottom to top direction. there's not an ideal way of mapping
			// between these two - what we choose to do here is map the 0 of our displayWindow to the baseline of the
			// freetype coordinate system.
			float scale = (float)m_face->size->metrics.x_ppem / (float)m_face->units_per_EM;
			return Box2i(
				V2i( (int)roundf( m_face->bbox.xMin * scale ), (int)roundf( -m_face->bbox.yMax * scale ) ),
				V2i( (int)roundf( m_face->bbox.xMax * scale ) - 1, (int)roundf( -m_face->bbox.yMin * scale ) - 1)
			);
		}

		static FT_Library library()
		{
			static FT_Library l;
			static bool init = false;
			if( !init )
			{
				FT_Error e = FT_Init_FreeType( &l );
				if( e )
				{
					throw Exception( "Error initialising FreeType library." );
				}
				init = true;
			}
			return l;
		}

};

//////////////////////////////////////////////////////////////////////////////////////////
// Font
//////////////////////////////////////////////////////////////////////////////////////////

Font::Font( const std::string &fontFile )
	:	m_implementation( new Implementation( fontFile ) )
{
}

Font::~Font()
{
}

const std::string &Font::fileName() const
{
	return m_implementation->fileName();
}

void Font::setKerning( float kerning )
{
	m_implementation->setKerning( kerning );
}

float Font::getKerning() const
{
	return m_implementation->getKerning();
}

void Font::setCurveTolerance( float tolerance )
{
	m_implementation->setCurveTolerance( tolerance );
}

float Font::getCurveTolerance() const
{
	return m_implementation->getCurveTolerance();
}

void Font::setResolution( float pixelsPerEm )
{
	m_implementation->setResolution( pixelsPerEm );
}

float Font::getResolution() const
{
	return m_implementation->getResolution();
}

const MeshPrimitive *Font::mesh( char c ) const
{
	return m_implementation->mesh( c );
}

MeshPrimitivePtr Font::mesh( const std::string &text ) const
{
	return m_implementation->mesh( text );
}

GroupPtr Font::meshGroup( const std::string &text ) const
{
	return m_implementation->meshGroup( text );
}

Imath::V2f Font::advance( char first, char second ) const
{
	return m_implementation->advance( first, second );
}

Imath::Box2f Font::bound() const
{
	return m_implementation->bound();
}

Imath::Box2f Font::bound( char c ) const
{
	return m_implementation->bound( c );
}

Imath::Box2f Font::bound( const std::string &text ) const
{
	return m_implementation->bound( text );
}

const ImagePrimitive *Font::image( char c ) const
{
	return m_implementation->image( c );
}

ImagePrimitivePtr Font::image() const
{
	return m_implementation->image();
}


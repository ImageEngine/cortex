//////////////////////////////////////////////////////////////////////////
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

#include "boost/multi_array.hpp"

#include "OpenEXR/ImathRandom.h"

#include "IECore/CurveTracer.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/ObjectWriter.h"
#include "IECore/VecAlgo.h"
#include "IECore/TransformOp.h"

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( CurveTracer );

CurveTracer::CurveTracer()
	:
	Op(
		"Traces curves from an ImagePrimitive prepared previously using the ImageThinner.",
		new ObjectParameter( "result",
			"A CurvesPrimitive with curves traced from the input.",
			NullObject::defaultNullObject(),
			CurvesPrimitive::staticTypeId()
		)
	)
{
	ImagePrimitiveParameterPtr imageParameter = new ImagePrimitiveParameter(
		"image",
		"The image to sample curves from.",
		new ImagePrimitive
	);

	parameters()->addParameter( imageParameter );
	
	StringParameterPtr channelNameParameter = new StringParameter(
		"channelName",
		"The channel the curves are to be traced from.",
		"Y"
	);

	parameters()->addParameter( channelNameParameter );
	
	FloatParameterPtr vertexMergeDistanceParameter = new FloatParameter(
		"vertexMergeDistance",
		"Vertices closer together than this distance will be merged together.",
		1,
		0
	);

	parameters()->addParameter( vertexMergeDistanceParameter );
	
	FloatParameterPtr minimumLengthParameter = new FloatParameter(
		"minimumLength",
		"The minimum length a spur must be to make it into the output.",
		5,
		0
	);

	parameters()->addParameter( minimumLengthParameter );
	
	BoolParameterPtr colorParameter = new BoolParameter(
		"color",
		"When this is on, a random color is given to each curve.",
		false
	);

	parameters()->addParameter( colorParameter );
	
	IntParameter::PresetsContainer outputTypePresets;
	outputTypePresets.push_back( IntParameter::Preset( "linear", Linear ) );
	outputTypePresets.push_back( IntParameter::Preset( "catmullRom", CatmullRom ) );
	
	IntParameterPtr outputTypeParameter = new IntParameter(
		"outputType",
		"The type of curves to output. Linear gives linear curves with approximately one vertex per pixel. "
		"CatmullRom gives catmull rom curves with sparser vertices. The former is intended to be post processed "
		"by some form of curve fitting tool, the latter is intended as a stopgap until such a tool exists.",
		Linear,
		Linear,
		CatmullRom,
		outputTypePresets,
		true
	);

	parameters()->addParameter( outputTypeParameter );
	
	IntParameterPtr catmullRomStepParameter = new IntParameter(
		"catmullRomStep",
		"The number of pixels between vertices when outputting catmull rom curves.",
		10,
		1
	);

	parameters()->addParameter( catmullRomStepParameter );

	IntParameter::PresetsContainer outputSpacePresets;
	outputSpacePresets.push_back( IntParameter::Preset( "pixel", ImagePrimitive::Pixel ) );
	outputSpacePresets.push_back( IntParameter::Preset( "uv", ImagePrimitive::UV ) );
	outputSpacePresets.push_back( IntParameter::Preset( "object", ImagePrimitive::Object ) );
	
	IntParameterPtr outputSpaceParameter = new IntParameter(
		"outputSpace",
		"The coordinate system to output the curves in.",
		ImagePrimitive::Pixel,
		ImagePrimitive::Pixel,
		ImagePrimitive::Object,
		outputSpacePresets,
		true
	);

	parameters()->addParameter( outputSpaceParameter );
	
}

CurveTracer::~CurveTracer()
{
}

ImagePrimitiveParameter * CurveTracer::imageParameter()
{
	return parameters()->parameter<ImagePrimitiveParameter>( "image" );
}

const ImagePrimitiveParameter * CurveTracer::imageParameter() const
{
	return parameters()->parameter<ImagePrimitiveParameter>( "image" );
}

StringParameter * CurveTracer::channelNameParameter()
{
	return parameters()->parameter<StringParameter>( "channelName" );
}

const StringParameter * CurveTracer::channelNameParameter() const
{
	return parameters()->parameter<StringParameter>( "channelName" );
}

FloatParameter * CurveTracer::vertexMergeDistanceParameter()
{
	return parameters()->parameter<FloatParameter>( "vertexMergeDistance" );
}

const FloatParameter * CurveTracer::vertexMergeDistanceParameter() const
{
	return parameters()->parameter<FloatParameter>( "vertexMergeDistance" );
}
		
FloatParameter * CurveTracer::minimumLengthParameter()
{
	return parameters()->parameter<FloatParameter>( "minimumLength" );
}

const FloatParameter * CurveTracer::minimumLengthParameter() const
{
	return parameters()->parameter<FloatParameter>( "minimumLength" );
}

IntParameter * CurveTracer::outputTypeParameter()
{
	return parameters()->parameter<IntParameter>( "outputType" );
}

const IntParameter * CurveTracer::outputTypeParameter() const
{
	return parameters()->parameter<IntParameter>( "outputType" );
}

IntParameter * CurveTracer::catmullRomStepParameter()
{
	return parameters()->parameter<IntParameter>( "catmullRomStep" );
}

const IntParameter * CurveTracer::catmullRomStepParameter() const
{
	return parameters()->parameter<IntParameter>( "catmullRomStep" );
}

IntParameter * CurveTracer::outputSpaceParameter()
{
	return parameters()->parameter<IntParameter>( "outputSpace" );
}

const IntParameter * CurveTracer::outputSpaceParameter() const
{
	return parameters()->parameter<IntParameter>( "outputSpace" );
}
		
ObjectPtr CurveTracer::doOperation( const CompoundObject * operands )
{
	
	// find the channel to operate on
	
	ConstImagePrimitivePtr image = operands->member<ImagePrimitive>( "image" );
	const std::string &channelName = operands->member<StringData>( "channelName" )->readable();
	ConstFloatVectorDataPtr channelData = image->getChannel<float>( channelName );

	if( !channelData )
	{
		throw Exception( boost::str( boost::format( "Image does not contain float channel named \"%s\"." ) % channelName ) );
	}

	// build the graph
	
	Graph graph;
	buildGraph( channelData->readable(), image->getDataWindow().size() + V2i( 1 ), graph );
	
	// postprocess it to deal with things we don't like

	removeSpurs( graph, operands->member<FloatData>( "minimumLength" )->readable() );
	mergeVertices( graph, operands->member<FloatData>( "vertexMergeDistance" )->readable() );
	
	// turn it into a curves primitive
	
	CurvesPrimitivePtr result = buildCurves(
		graph,
		(OutputType)operands->member<IntData>( "outputType" )->readable(),
		operands->member<IntData>( "catmullRomStep" )->readable()
	);
	
	// transform it to the output space
	ImagePrimitive::Space space = (ImagePrimitive::Space)operands->member<IntData>( "outputSpace" )->readable();
	M33f t = image->matrix( ImagePrimitive::Pixel, space );
	M44f t3d( 
		t[0][0], t[0][1], 0.0f, t[0][2],
		t[1][0], t[1][1], 0.0f, t[1][2],
		   0.0f,	0.0f, 0.0f, 0.0f, 	
		t[2][0], t[2][1], 0.0f, t[2][2]
	);
	t3d.translate( V3f( -1, -1, 0 ) ); // to correct for the padding introduced during tracing.
	TransformOpPtr transformOp = new TransformOp();
	transformOp->inputParameter()->setValue( result );
	transformOp->copyParameter()->setTypedValue( false );
	transformOp->matrixParameter()->setValue( new M44fData( t3d ) );
	transformOp->operate();
	
	// add color if requested
	if( operands->member<BoolData>( "color" )->readable() )
	{
		colorCurves( result.get() );
	}
	
	return result;
}

void CurveTracer::buildGraph( const std::vector<float> &channel, const Imath::V2i &size, Graph &graph ) const
{
	// copy the channel into a temporary buffer which we can modify. pad the edges while we're at
	// it so we don't have to worry about going outside the bound.
	
	V2i paddedSize = size + V2i( 2 );
	
	vector<float>::const_iterator fIt = channel.begin();
	
	vector<char> pixels;
	pixels.resize( paddedSize.x * paddedSize.y, 0 );
	vector<char>::iterator it = pixels.begin() + paddedSize.x + 1;
		
	for( int y=0; y<size.y; y++ )
	{
		for( int x=0; x<size.x; x++ )
		{
			*it++ = *fIt++ > 0.5 ? 1 : 0;
		}
		it += 2; // skip padding
	}
	
	boost::multi_array_ref<char, 2> array( &pixels[0], extents[paddedSize.y][paddedSize.x] );
	
	// make the graph we'll use to process everything
	
	property_map<Graph, vertex_name_t>::type vertexPositions = get( vertex_name, graph );
	property_map<Graph, edge_name_t>::type edgePixels = get( edge_name, graph );
	VertexMap vertexMap;

	// find all the vertices and insert them in the graph.
	// vertices are foreground pixels which have anything other than 2 connections.
	for( int y=1; y<paddedSize.y-1; y++ )
	{
		for( int x=1; x<paddedSize.x-1; x++ )
		{
			if( array[y][x] )
			{
				int connectivity = 0;
				for( int yo=-1; yo<2; yo++ )
				{
					for( int xo=-1; xo<2; xo++ )
					{
						if( array[y+yo][x+xo] )
						{
							connectivity += 1;
						}
					}
				}
				if( connectivity!=3 )
				{
					Vertex v = add_vertex( graph );
					vertexPositions[v] = V2i( x, y );
					array[y][x] = 2; // mark as vertex
					vertexMap[V2i( x, y )] = v;
				}
			}
		}
	}

	// for each of the vertices trace lines to form edges in the graph.
	VertexIteratorRange vertRange = vertices( graph );
	for( VertexIterator vIt = vertRange.first; vIt != vertRange.second; vIt++ )
	{
		V2i vertexPosition = vertexPositions[*vIt];
		
		V2i startPixel;
		for( startPixel.y = vertexPosition.y - 1; startPixel.y <= vertexPosition.y + 1; startPixel.y++ )
		{
			for( startPixel.x = vertexPosition.x - 1; startPixel.x <= vertexPosition.x + 1; startPixel.x++ )
			{

				if( startPixel==vertexPosition || array[startPixel.y][startPixel.x]==0 )
				{
					continue;
				}

				// trace a line till we hit a vertex
				V2i thisPixel = startPixel;
				PixelVector thisEdge;
				Vertex endVertex;
				while( 1 )
				{
					// stop if we've hit a vertex
					if( array[thisPixel.y][thisPixel.x]==2 )
					{
						VertexMap::const_iterator it = vertexMap.find( thisPixel );
						assert( it!=vertexMap.end() );
						endVertex = it->second;
						break;
					}

					// add this pixel to the edge we're making
					thisEdge.push_back( thisPixel );

					array[thisPixel.y][thisPixel.x] = 0;

					// find the next pixel to visit.	
					V2i nextPixel( -1 );
					for( int yo=-1; yo<2; yo++ )
					{
						for( int xo=-1; xo<2; xo++ )
						{
							if( array[thisPixel.y+yo][thisPixel.x+xo] )
							{
								V2i tmp = thisPixel + V2i( xo, yo );
								if( tmp != vertexPosition ) // avoid jumping straight back to the start pixel
								{
									nextPixel = tmp;
								}
							}
						}
					}

					if( nextPixel.x < 0 )
					{
						// didn't find a suitable next pixel. this should mean that we've looped back
						// to the start vertex.
						assert( abs( thisPixel.x - vertexPosition.x ) <= 1 );
						assert( abs( thisPixel.y - vertexPosition.y ) <= 1 );
						nextPixel = vertexPosition;
					}

					assert( nextPixel.x >= 0 );

					thisPixel = nextPixel;
				}

				// insert the edge for the line in the graph
				EdgeBool addedEdge = add_edge( *vIt, endVertex, graph );
				assert( addedEdge.second );
				edgePixels[addedEdge.first] = thisEdge;

			}
		}

	}
}

void CurveTracer::removeSpurs( Graph &graph, float minimumLength ) const
{	
	if( minimumLength <= 0.0f )
	{
		return;
	}

	property_map<Graph, edge_name_t>::type edgePixels = get( edge_name, graph );

	EdgeIteratorRange edgeRange = edges( graph );
	for( EdgeIterator eIt = edgeRange.first; eIt != edgeRange.second; )
	{
		EdgeIterator nextIt = eIt; ++nextIt;

		const PixelVector &pixels = edgePixels[*eIt];
		if( 1 + pixels.size() < minimumLength )
		{

			Vertex v1 = source( *eIt, graph );
			Vertex v2 = target( *eIt, graph );

			int d1 = out_degree( v1, graph ) + in_degree( v1, graph );
			int d2 = out_degree( v2, graph ) + in_degree( v2, graph );

			if( (d1==1 && d2>1) || (d2==1 && d1>1) )
			{
				remove_edge( *eIt, graph );
			}
		}

		eIt = nextIt;
	}
}

void CurveTracer::mergeVertices( Graph &graph, float mergeDistance ) const
{
	if( mergeDistance <= 0.0f )
	{
		return;
	}

	property_map<Graph, edge_name_t>::type edgePixels = get( edge_name, graph );

	EdgeIteratorRange edgeRange = edges( graph );
	for( EdgeIterator eIt = edgeRange.first; eIt!=edgeRange.second; )
	{
		EdgeIterator nextEdgeIt = eIt; nextEdgeIt++;
	
		if( 1 + edgePixels[*eIt].size() <= mergeDistance )
		{
			Vertex v0 = source( *eIt, graph );
			Vertex v1 = target( *eIt, graph );
		
			if( v0 == v1 )
			{
				// we've found a very short loop from a vertex back to itself.
				// it seems reasonable to just remove the edge. it might perhaps be
				// better to deal with these guys in a different step though.
				remove_edge( *eIt, graph );
			}
			else
			{
				// we've found a short edge to another vertex. collapse the first
				// vertex of the edge into the second.

				OutEdgeIteratorRange outEdgeRange = out_edges( v0, graph );
				for( OutEdgeIterator oIt = outEdgeRange.first; oIt != outEdgeRange.second; )
				{
					OutEdgeIterator nextOIt = oIt; nextOIt++;
					if( edgePixels[*oIt].size() )
					{
						EdgeBool transferredEdge = add_edge( v1, target( *oIt, graph ), graph );
						assert( transferredEdge.second );
						edgePixels[transferredEdge.first] = edgePixels[*oIt];
					}
					if( *oIt==*nextEdgeIt )
					{
						nextEdgeIt++;
					}
					remove_edge( *oIt, graph );
					oIt = nextOIt;
				}

				InEdgeIteratorRange inEdgeRange = in_edges( v0, graph );
				for( InEdgeIterator iIt = inEdgeRange.first; iIt != inEdgeRange.second; )
				{
					InEdgeIterator nextIIt = iIt; nextIIt++;
					if( edgePixels[*iIt].size() )
					{
						EdgeBool transferredEdge = add_edge( source( *iIt, graph ), v1, graph );
						assert( transferredEdge.second );
						edgePixels[transferredEdge.first] = edgePixels[*iIt];
					}
					if( *iIt==*nextEdgeIt )
					{
						nextEdgeIt++;
					}
					remove_edge( *iIt, graph );
					iIt = nextIIt;
				}

				remove_vertex( v0, graph );
			}
			
		}
		
		eIt = nextEdgeIt;
	}

}

CurvesPrimitivePtr CurveTracer::buildCurves( Graph &graph, OutputType type, int catmullRomStep ) const
{
	IntVectorDataPtr vertsPerCurve = new IntVectorData;
	V3fVectorDataPtr verts = new V3fVectorData;
	
	property_map<Graph, edge_name_t>::type edgePixels = get( edge_name, graph );
	property_map<Graph, vertex_name_t>::type vertexPositions = get( vertex_name, graph );

	EdgeIteratorRange edgeRange = edges( graph );
	for( EdgeIterator eIt = edgeRange.first; eIt != edgeRange.second; eIt++ )
	{
		Vertex v1 = source( *eIt, graph );
		Vertex v2 = target( *eIt, graph );

		const PixelVector &pixels = edgePixels[*eIt];
		
		int numVerts = 2;		
		V2i vp = vertexPositions[v1];
		verts->writable().push_back( V3f( vp.x, vp.y, 0.0f ) );
		if( type==CatmullRom )
		{
			numVerts++;
			verts->writable().push_back( V3f( vp.x, vp.y, 0.0f ) );
		}

		for( unsigned i = type==Linear ? 0 : catmullRomStep; i<pixels.size(); i += type==Linear ? 1 : catmullRomStep )
		{
			numVerts++;
			verts->writable().push_back( V3f( pixels[i].x, pixels[i].y, 0.f ) );
		}
		
		vp = vertexPositions[v2];
		verts->writable().push_back( V3f( vp.x, vp.y, 0.0f ) );
		if( type==CatmullRom )
		{
			numVerts++;
			verts->writable().push_back( V3f( vp.x, vp.y, 0.0f ) );
		}	

		vertsPerCurve->writable().push_back( numVerts );
	}

	return new CurvesPrimitive(
		vertsPerCurve,
		type==Linear ? CubicBasisf::linear() : CubicBasisf::catmullRom(),
		false,
		verts
	);
}

void CurveTracer::colorCurves( CurvesPrimitive * curves ) const
{
	Color3fVectorDataPtr colors = new Color3fVectorData;

	Rand32 random;
	
	int numCurves = curves->numCurves();
	colors->writable().resize( curves->numCurves() );
	for( int i=0; i<numCurves; i++ )
	{
		colors->writable()[i] = Color3f( random.nextf( 0, 1 ), random.nextf( 0, 1 ), random.nextf( 0, 1 ) );
	}

	curves->variables["Cs"] = PrimitiveVariable( PrimitiveVariable::Uniform, colors );
}

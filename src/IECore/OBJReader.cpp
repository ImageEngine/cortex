//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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


#include <iostream>
#include <iterator>
#include <fstream>

#include "boost/bind.hpp"
#include "boost/version.hpp"
#if BOOST_VERSION >= 103600
#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#include "boost/spirit/include/classic.hpp"
#else
#include "boost/spirit.hpp"
#endif

#include "IECore/OBJReader.h"
#include "IECore/CompoundData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;
using namespace boost::spirit;

IE_CORE_DEFINERUNTIMETYPED(OBJReader);

// syntactic sugar for specifying our grammar
typedef boost::spirit::rule<boost::spirit::phrase_scanner_t> srule;

const Reader::ReaderDescription<OBJReader> OBJReader::m_readerDescription("obj");

OBJReader::OBJReader( const std::string &fileName )
	: Reader( "Alias Wavefront OBJ 3D data reader", new ObjectParameter("result", "the loaded 3D object", new
	NullObject, MeshPrimitive::staticTypeId()))
{
	m_fileNameParameter->setTypedValue( fileName );
}

bool OBJReader::canRead( const string &fileName )
{
	// there really are no magic numbers, .obj is a simple ascii text file

	// so: enforce at least that the file has '.obj' extension
	if(fileName.rfind(".obj") != fileName.length() - 4)
		return false;

	// attempt to open the file
	ifstream in(fileName.c_str());
	return in.is_open();
}

ObjectPtr OBJReader::doOperation(const CompoundObject * operands)
{
	// for now we are going to retrieve vertex, texture, normal coordinates, faces.
	// later (when we have the primitives), we will handle a larger subset of the
	// OBJ format

	IntVectorDataPtr vpf = new IntVectorData();
	m_vpf = &vpf->writable();

	IntVectorDataPtr vids = new IntVectorData();
	m_vids = &vids->writable();

	V3fVectorDataPtr vertices = new V3fVectorData();
	m_vertices = &vertices->writable();

 	// separate texture coordinates
	FloatVectorDataPtr sTextureCoordinates = new FloatVectorData();
	m_sTextureCoordinates = &sTextureCoordinates->writable();

	FloatVectorDataPtr tTextureCoordinates = new FloatVectorData();
	m_tTextureCoordinates = &tTextureCoordinates->writable();

	// build normals
	V3fVectorDataPtr normals = new V3fVectorData();
	m_normals = &normals->writable();

	// parse the file
	parseOBJ();

	// create our MeshPrimitive
	MeshPrimitivePtr mesh = new MeshPrimitive( vpf, vids, "linear", vertices );
	if( sTextureCoordinates->readable().size() )
	{
		mesh->variables.insert(PrimitiveVariableMap::value_type("s", PrimitiveVariable( PrimitiveVariable::FaceVarying, sTextureCoordinates)));
	
	}
	if( tTextureCoordinates->readable().size() )
	{
		mesh->variables.insert(PrimitiveVariableMap::value_type("t", PrimitiveVariable(  PrimitiveVariable::FaceVarying, tTextureCoordinates)));
	}
	if( normals->readable().size() )
	{
		mesh->variables.insert(PrimitiveVariableMap::value_type("N", PrimitiveVariable(  PrimitiveVariable::FaceVarying, normals)));
	}
	return mesh;
}


// parse a vertex
void OBJReader::parseVertex(const char * begin, const char * end)
{
	vector<float> vec;
	srule vertex = "v" >> real_p[append(vec)] >> real_p[append(vec)] >> real_p[append(vec)];
	parse(begin, vertex, space_p);

	// build v
	V3f v;
	v[0] = vec[0];
 	v[1] = vec[1];
 	v[2] = vec[2];

	// add this vertex
	m_vertices->push_back(v);
}

// parse a texture coordinate
void OBJReader::parseTextureCoordinate(const char * begin, const char * end)
{
	vector<float> vec;
	srule vertex = "vt" >> real_p[append(vec)] >> real_p[append(vec)] >> *(real_p[append(vec)]);
	parse(begin, vertex, space_p);

	// build v
	V3f vt;
	vt[0] = vec[0];
 	vt[1] = vec[1];
	vt[2] = vec.size() == 3 ? vec[2] : 0.0f;

	// add this texture coordinate
	m_introducedTextureCoordinates.push_back(vt);
}

// parse a normal
void OBJReader::parseNormal(const char * begin, const char * end)
{
	vector<float> vec;
	srule vertex = "vn" >> real_p[append(vec)] >> real_p[append(vec)] >> real_p[append(vec)];
	parse(begin, vertex, space_p);

	// build v
	V3f vn;
	vn[0] = vec[0];
	vn[1] = vec[1];
	vn[2] = vec[2];

	// add this normal
	m_introducedNormals.push_back(vn);
}

// parse face
void OBJReader::parseFace(const char * begin, const char * end)
{
	vector<int> vec;
	vector<int> tvec;
	vector<int> nvec;

	srule entry = int_p[append(vec)] >>
		(
			("/" >> (int_p[append(tvec)] | epsilon_p) >> "/" >> (int_p[append(nvec)] | epsilon_p))
			| epsilon_p
		);

	srule face = "f"  >> entry >> entry >> entry >> *(entry);
	parse(begin, face, space_p);

	// push back the degree of the face
	m_vpf->push_back(vec.size());

	// merge in the edges.  we index from 0, so shift them down.
	// also, vertices may be indexed negatively, in which case they are relative to
	// the current set of vertices
	for(vector<int>::const_iterator i = vec.begin(); i != vec.end(); ++i)
	{
		m_vids->push_back(*i > 0 ? *i - 1 : m_vertices->size() + *i);
	}


	// merge in texture coordinates and normals, if present
	// OBJ format requires an encoding for faces which uses one of the vertex/texture/normal specifications
	// consistently across the entire face.  eg. we can have all v/vt/vn, or all v//vn, or all v, but not
	// v//vn then v/vt/vn ...
	if(!nvec.empty())
	{
		if(nvec.size() != vec.size())
			throw Exception("invalid face specification");

		// copy in these references to normal vectors to the mesh's normal vector
		for(vector<int>::const_iterator i = nvec.begin(); i != nvec.end(); ++i)
		{
			m_normals->push_back(m_introducedNormals[*i > 0 ? *i - 1 : m_introducedNormals.size() + *i]);
		}
	}
	// otherwise, check if we have specified normals in some previous face
	// if so, and no normals were given here (examples, encoders that do this?), pump in
	// default normal.  the default normal defined here is the zero normal, which is by
	// definition orthogonal to every other vector.  this might result in odd lighting.
	else
	{
		V3f zero(0.0f, 0.0f, 0.0f);
		for(unsigned int i = 0; i < nvec.size(); ++i)
		{
			m_normals->push_back(zero);
		}
	}

	//
	// merge in texture coordinates, if present
	//
	if(!tvec.empty())
	{
		if(tvec.size() != vec.size())
			throw Exception("invalid face specification");

		for(unsigned int i = 0; i < tvec.size(); ++i)
		{
			int index = tvec[i] > 0 ? tvec[i] - 1 : m_introducedTextureCoordinates.size() + tvec[i];
			m_sTextureCoordinates->push_back(m_introducedTextureCoordinates[index][0]);
			m_tTextureCoordinates->push_back(m_introducedTextureCoordinates[index][1]);
		}
	}
	else
	{
		for(unsigned int i = 0; i < tvec.size(); ++i)
		{
			m_sTextureCoordinates->push_back(0.0f);
			m_tTextureCoordinates->push_back(0.0f);
		}
	}
}

void OBJReader::parseGroup(const char *begin, const char *end)
{
	// set current group
	vector<string> groupNames;
	srule grouping = "g" >> *(lexeme_d[alnum_p >> *(alnum_p)][append(groupNames)]);

	parse(begin, grouping, space_p);

	// from 'http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/':
	// The default group name is default.
	if(groupNames.empty())
	{
		groupNames.push_back("default");
	}

	// \todo associate mesh objects with group names
}

void OBJReader::parseOBJ() {

	srule comment = comment_p("#");

	// see
	// http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/

	// vertices
	srule vertex         = ("v"  >> real_p >> real_p >> real_p) [bind(&OBJReader::parseVertex,            this, _1, _2)];
	srule vertex_texture = ("vt" >> real_p >> real_p)           [bind(&OBJReader::parseTextureCoordinate, this, _1, _2)];
	srule vertex_normal  = ("vn" >> real_p >> real_p >> real_p) [bind(&OBJReader::parseNormal,            this, _1, _2)];
	//srule vertex_parameter_space  = "vp" >> real_p >> real_p >> real_p;

	// srule cs_types = ("bmatrix" | "bezier" | "bspline" | "cardinal" | "taylor");
	// srule vertex_curve_or_surface = "cstype" >> "rat" >> cs_types;
	// srule vertex_degree  = "deg" >> real_p >> real_p;
	// srule vertex_basis_matrix  = "bmat";
	// srule vertex_step_size  = "step" >> int_p >> int_p;
	srule vertex_type = vertex | vertex_texture | vertex_normal;

	// elements
	srule point = "p" >> real_p >> *(real_p);
	srule  line = "l" >> int_p >> int_p >> *(int_p);
	srule  face = (ch_p('f') >> *(anychar_p))[bind(&OBJReader::parseFace, this, _1, _2)];
	// srule curve = "curv";
	// srule curve_2d = "curv2";
	// srule surface = "surf";
	srule element = point | line | face;

 	// free-form curve / surface statements
	// srule parameter = "parm";
	// srule trim_loop = "trim";
	// srule hole_loop = "hole";
	// srule special_curve = "scrv";
	// srule special_point = "sp";
	// srule end_statement = "end";

	// connectivity
	//srule connect = "con";

	// grouping
	srule group_name = ("g" >> *(anychar_p))[bind(&OBJReader::parseGroup, this, _1, _2)];
	//	srule smoothing_group = "s";
	//	srule merging_group = "mg";
	srule object_name = "o" >> int_p;
	srule grouping = group_name | object_name;

	// display and render attributes
	// srule bevel_interpretation = "bevel";
	// srule color_interpolation = "c_interp";
	// srule dissolve_interpolation = "d_interp";
	// srule level_of_detail = "lod";
	// srule material_name = "usemtl";
	// srule material_library = "mtllib";
	// srule shadow_casting = "shadow_obj";
	// srule ray_tracing = "trace_obj";
	// srule curve_approximation_technique = "ctech";
	// srule surface_approximation_technique = "stech";

	ifstream in(fileName().c_str());
	string str;
	while(getline(in, str))
		parse(str.c_str(), vertex_type | element | grouping | comment, space_p);
}

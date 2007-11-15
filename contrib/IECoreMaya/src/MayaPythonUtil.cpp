//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "maya/MGlobal.h"
#include "maya/MCommandResult.h"
#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"

#include "maya/MVector.h"
#include "maya/MVectorArray.h"
#include "maya/MMatrix.h"
#include "maya/MString.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/MayaPythonUtil.h"

namespace IECoreMaya
{

/// \todo Add a parameter to echo the command?
/// \todo Expose the MCommandResult->IECore::Object conversion
/// somewhere useful.
/// \todo Put the M*Array->IECore::*VectorData conversion
/// somewhere useful.
IECore::DataPtr executeMel( const std::string &cmd )
{
	MCommandResult result;
	MStatus s = MGlobal::executeCommand( cmd.c_str(), result, false, false );
	
	if (s)
	{
		switch (result.resultType())
		{
			case MCommandResult::kInvalid:
			{
				// No result
				return 0;
			}
			case MCommandResult::kInt:
			{
				int i;
				s = result.getResult(i);
				assert(s);
				
				IECore::IntDataPtr data = new IECore::IntData();
				data->writable() = i;
				
				return data;
			}
			case MCommandResult::kIntArray:
			{
				MIntArray v;
				s = result.getResult(v);
				assert(s);
				unsigned sz = v.length();
				IECore::IntVectorDataPtr data = new IECore::IntVectorData();
				data->writable().resize(sz);
				for (unsigned i = 0; i < sz; i++)
				{
					(data->writable())[i] = v[i];
				}
				
				return data;
			}
			case MCommandResult::kDouble:
			{
				double d;
				s = result.getResult(d);
				assert(s);
				
				IECore::FloatDataPtr data = new IECore::FloatData();
				data->writable() = static_cast<float>(d);
				
				return data;
			}
			case MCommandResult::kDoubleArray:
			{
				MDoubleArray v;
				s = result.getResult(v);
				assert(s);
				unsigned sz = v.length();
				IECore::DoubleVectorDataPtr data = new IECore::DoubleVectorData();
				data->writable().resize(sz);
				for (unsigned i = 0; i < sz; i++)
				{
					data->writable()[i] = v[i];
				}
				
				return data;
			}
			case MCommandResult::kString:
			{
				MString str;
				s = result.getResult(str);
				assert(s);
				
				IECore::StringDataPtr data = new IECore::StringData();
				data->writable() = std::string(str.asChar());
				
				return data;
			}
			case MCommandResult::kStringArray:
			{
				MStringArray v;
				s = result.getResult(v);
				assert(s);
				unsigned sz = v.length();
				IECore::StringVectorDataPtr data = new IECore::StringVectorData();
				data->writable().resize(sz);
				for (unsigned i = 0; i < sz; i++)
				{
					data->writable()[i] = std::string(v[i].asChar());
				}
				
				return data;
			}
			case MCommandResult::kVector:
			{
				MVector v;
				s = result.getResult(v);
				assert(s);
				
				IECore::V3fDataPtr data = new IECore::V3fData();
				data->writable() = Imath::V3f(v.x, v.y, v.z);
				
				return data;
			}
			case MCommandResult::kVectorArray:
			{
				MVectorArray v;
				s = result.getResult(v);
				assert(s);
				unsigned sz = v.length();
				IECore::V3fVectorDataPtr data = new IECore::V3fVectorData();
				data->writable().resize(sz);					
				for (unsigned i = 0; i < sz; i++)
				{
					data->writable()[i] = Imath::V3f(v[i].x, v[i].y, v[i].z);
				}
				
				return data;
			}
			case MCommandResult::kMatrix:
			{
				MDoubleArray v;
				int numRows, numColumns;
				
				s = result.getResult(v, numRows, numColumns);
				assert(s);
				
				if (numRows > 4 || numColumns > 4)
				{
					throw StatusException( MS::kFailure );
				}
				
				IECore::M44fDataPtr data = new IECore::M44fData();
				
				for (int i = 0; i < numColumns; i++)
				{
					for (int j = 0; j < numRows; j++)
					{
						(data->writable())[i][j] = v[i*numRows+j];
					}
				}		
				
				return data;
			}
			case MCommandResult::kMatrixArray:
			{
				return 0;
			}
			
		}
		
	}
	else
	{
		throw StatusException(s);
	}
	
	return 0;
}

}

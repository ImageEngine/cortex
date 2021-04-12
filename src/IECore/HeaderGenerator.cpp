//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/HeaderGenerator.h"

#include "IECore/CompoundData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/Version.h"

#include "boost/algorithm/string/trim.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"

#ifndef _MSC_VER
#include <pwd.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

using namespace std;
using namespace IECore;



std::vector<HeaderGenerator::DataHeaderFn> HeaderGenerator::m_generators;


CompoundObjectPtr HeaderGenerator::header()
{
	CompoundObjectPtr newHeader = new CompoundObject();
	for ( std::vector<DataHeaderFn>::const_iterator it = m_generators.begin(); it != m_generators.end(); it++ )
	{
		(*it)( newHeader );
	}
	return newHeader;
}

bool HeaderGenerator::registerDataHeaderGenerator( DataHeaderFn generator )
{
	m_generators.push_back( generator );
	return true;
}

/*
 *
 * Standard data header functions.
 *
 */

static void ieCoreHeaderGenerator( CompoundObjectPtr header )
{
	header->members()["ieCoreVersion"] = new StringData( versionString() );
}

static void unameHeaderGenerator( CompoundObjectPtr header )
{
#ifndef _MSC_VER
	struct utsname name;

	if ( !uname( &name ) )
	{
		CompoundDataPtr compound = new CompoundData();
		compound->writable()["systemName"] = new StringData( name.sysname );
		compound->writable()["nodeName"] = new StringData( name.nodename );
		compound->writable()["systemRelease"] = new StringData( name.release );
		compound->writable()["systemVersion"] = new StringData( name.version );
		compound->writable()["machineName"] = new StringData( name.machine );
		header->members()["host"] = compound;
	}
#else
	OSVERSIONINFOEX ovx;
	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	SYSTEM_INFO systemInfo;
	DWORD computerNameSize = sizeof(computerName) / sizeof(computerName[0]);
	std::string arch;

	bool cnSuccess = GetComputerNameA(computerName, &computerNameSize);
	ovx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bool piSuccess = GetVersionEx(reinterpret_cast<OSVERSIONINFOA *>(&ovx));

	if (cnSuccess && piSuccess)
	{
		CompoundDataPtr compound = new CompoundData();
		compound->writable()["systemName"] = new StringData("Windows");
		// Python and MSVC are inconsistent in capitalization of the machine name and
		// there seems to be a weak consensus on using all caps for computer names in Windows networks
		boost::to_upper(computerName);

		compound->writable()["nodeName"] = new StringData(computerName);
		compound->writable()["systemRelease"] = new IntData(ovx.dwMajorVersion);
		compound->writable()["systemVersion"] = new IntData(ovx.dwMinorVersion);

		GetSystemInfo(&systemInfo);

		switch (systemInfo.wProcessorArchitecture) {
			case PROCESSOR_ARCHITECTURE_AMD64: arch = "x86_64"; break;
			case PROCESSOR_ARCHITECTURE_ARM: arch = "ARM"; break;
			case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
			case PROCESSOR_ARCHITECTURE_IA64: arch = "IA64"; break;
			case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
			default: arch = "unknown"; break;
		}
		compound->writable()["machineName"] = new StringData(arch.c_str());

		header->members()["host"] = compound;
	}
	
#endif
}

static void userHeaderGenerator(CompoundObjectPtr header)
{
#ifndef _MSC_VER
	uid_t uid = getuid();
	struct passwd *st = getpwuid(uid);
	if (st)
	{
		header->members()["userName"] = new StringData(st->pw_name);
	}
	else
	{
		header->members()["userID"] = new IntData(uid);
	}
#else
	if ( const char *user = getenv( "USERNAME" ) )
	{
		header->members()["userName"] = new StringData( user );
	}
#endif
}

static void timeStampHeaderGenerator( CompoundObjectPtr header )
{
	time_t tm;
	time( &tm );

#ifndef _MSC_VER
	/// ctime_r manpage suggest that 26 characters should be enough in all cases
	char buf[27];
	std::string strTime ( ctime_r( &tm, &buf[0] ) );
#else
	std::string strTime ( ctime( &tm ) );
#endif

	assert( strTime.length() <= 26 );

	/// Remove the newline at the end
	boost::algorithm::trim_right( strTime );

	header->members()["timeStamp"] = new StringData( strTime );
}

static bool resIeCore = HeaderGenerator::registerDataHeaderGenerator( &ieCoreHeaderGenerator );
static bool resUname = HeaderGenerator::registerDataHeaderGenerator( &unameHeaderGenerator );
static bool resName = HeaderGenerator::registerDataHeaderGenerator( &userHeaderGenerator );
static bool resTime = HeaderGenerator::registerDataHeaderGenerator( &timeStampHeaderGenerator );

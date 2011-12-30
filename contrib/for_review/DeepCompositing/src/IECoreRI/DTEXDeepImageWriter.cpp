#include "boost/algorithm/string.hpp"

#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"

#include "IECoreRI/DTEXDeepImageWriter.h"

using namespace IECore;
using namespace IECoreRI;

IE_CORE_DEFINERUNTIMETYPED( DTEXDeepImageWriter );

const DeepImageWriter::DeepImageWriterDescription<DTEXDeepImageWriter> DTEXDeepImageWriter::g_writerDescription( "dtex" );

DTEXDeepImageWriter::DTEXDeepImageWriter()
	:	DeepImageWriter( "Writes PRMan DTEX deep texture file format." ),
		m_outputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 )
{
	m_tileSizeParameter = new V2iParameter( "tileSize", "The tile size for the image cache. Must be equal or less than resolution.", new V2iData( Imath::V2i( 32, 32 ) ) );
	parameters()->addParameter( m_tileSizeParameter );
}

DTEXDeepImageWriter::DTEXDeepImageWriter( const std::string &fileName )
	:	DeepImageWriter( "Writes PRMan DTEX deep texture file format." ),
		m_outputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );
	
	m_tileSizeParameter = new V2iParameter( "tileSize", "The tile size for the image cache. Must be equal or less than resolution.", new V2iData( Imath::V2i( 32, 32 ) ) );
	parameters()->addParameter( m_tileSizeParameter );
}

DTEXDeepImageWriter::~DTEXDeepImageWriter()
{
	cleanRixInterface();
}

bool DTEXDeepImageWriter::canWrite( const std::string &fileName )
{
	RixDeepTexture *dtexInterface = (RixDeepTexture *)RixGetContext()->GetRixInterface( k_RixDeepTexture );
	RixDeepTexture::DeepCache *dtexCache = dtexInterface->CreateCache( 1 );
	RixDeepTexture::DeepFile *dtexFile = 0;
	
	int status = dtexInterface->CreateFile( fileName.c_str(), dtexCache, &dtexFile );
	
	if ( dtexFile )
	{
		dtexFile->Close();
		dtexInterface->DestroyFile( dtexFile );
	}
	
	dtexInterface->DestroyCache( dtexCache );
	
	return ( status == RixDeepTexture::k_ErrNOERR );
}

void DTEXDeepImageWriter::doWritePixel( int x, int y, const DeepPixel *pixel )
{
	open();
	
	m_dtexPixel->Clear( pixel->numChannels() );
	
	unsigned numSamples = pixel->numSamples();
	for ( unsigned i=0; i < numSamples; ++i )
	{
		m_dtexPixel->Append( pixel->getDepth( i ), const_cast<float*>( pixel->channelData( i ) ), 0 );
	}
	
	m_dtexPixel->Finish();
	m_dtexImage->SetPixel( x, y, m_dtexPixel );
}

void DTEXDeepImageWriter::open()
{
	if ( m_outputFile && fileName() == m_outputFileName )
	{
		// we already opened the right file successfully
		return;
	}

	cleanRixInterface();
	m_outputFileName = "";
	
	const std::vector<std::string> &channelNames = m_channelsParameter->getTypedValue();
	
	std::string channels;
	unsigned numChannels = channelNames.size();
	for ( unsigned i=0; i < numChannels; ++i )
	{
		if ( channelNames[i].size() > 1 )
		{
			throw InvalidArgumentException( std::string( "Channel names must be single characters. \"" + channelNames[i] + "\" is too long." ) );
		}
		
		channels += channelNames[i];
	}
	std::string imageExtension = boost::to_lower_copy( channels );
	
	const Imath::V2i &resolution = m_resolutionParameter->getTypedValue();
	const Imath::V2i &tileSize = m_tileSizeParameter->getTypedValue();
	
	if ( tileSize.x > resolution.x || tileSize.y > resolution.y )
	{
		throw InvalidArgumentException( std::string( "Tile size must be equal to or less than resolution." ) );
	}
	
	if ( ( ( tileSize.x & ( tileSize.x - 1 ) ) != 0 ) || ( ( tileSize.x & ( tileSize.x - 1 ) ) != 0 ) )
	{
		throw InvalidArgumentException( std::string( "Tile width and height must be a power of two." ) );
	}
	
	RixDeepTexture *dtexInterface = (RixDeepTexture *)RixGetContext()->GetRixInterface( k_RixDeepTexture );
	m_dtexCache = dtexInterface->CreateCache( resolution.x / tileSize.x );
	
	if ( dtexInterface->CreateFile( fileName().c_str(), m_dtexCache, &m_outputFile ) != RixDeepTexture::k_ErrNOERR )
	{
		cleanRixInterface();
		m_outputFileName = "";
		throw IOException( std::string( "Failed to open file \"" ) + fileName() + "\" for writing." );
	}
	
	m_outputFileName = fileName();

	/// \todo: what are these used for? should they come from a parameter?
	float NP[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 };
	float NL[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 };
	
	/// \todo: should compression style be a parameter?
	int status = m_outputFile->AddImage(
		( "main." + imageExtension ).c_str(), numChannels, resolution.x, resolution.y, tileSize.x, tileSize.y,
		NP, NL, RixDeepTexture::k_CmpLZW, RixDeepTexture::k_TypeFLOAT, &m_dtexImage
	);
	
	if ( status != RixDeepTexture::k_ErrNOERR )
	{
		cleanRixInterface();
		m_outputFileName = "";
		throw IOException( std::string( "Failed to create the main sub-image in \"" ) + fileName() + "\" for writing." );
	}
	
	m_dtexPixel = dtexInterface->CreatePixel( numChannels );
}

void DTEXDeepImageWriter::cleanRixInterface()
{
	RixDeepTexture *dtexInterface = (RixDeepTexture *)RixGetContext()->GetRixInterface( k_RixDeepTexture );
	
	if ( m_dtexPixel )
	{
		dtexInterface->DestroyPixel( m_dtexPixel );
	}

	if ( m_outputFile )
	{
		m_outputFile->Close();
		dtexInterface->DestroyFile( m_outputFile );
	}

	if ( m_dtexCache )
	{
		dtexInterface->DestroyCache( m_dtexCache );
	}

	m_outputFile = 0;
	m_dtexCache = 0;
	m_dtexImage = 0;
	m_dtexPixel = 0;
}

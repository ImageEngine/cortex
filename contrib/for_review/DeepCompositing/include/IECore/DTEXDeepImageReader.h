#ifndef IECORERI_DTEXDEEPIMAGEREADER_H
#define IECORERI_DTEXDEEPIMAGEREADER_H

#include "RixDeepTexture.h"

#include "IECore/DeepImageReader.h"

#include "IECoreRI/TypeIds.h"

namespace IECoreRI
{

/// The DTEXDeepImageReader class reads PRMan deep texture files. Note that it will only
/// read the first RixDeepTexture::DeepImage in the RixDeepTexture::DeepFile.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class DTEXDeepImageReader : public IECore::DeepImageReader
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( DTEXDeepImageReader, DTEXDeepImageReaderTypeId, IECore::DeepImageReader );

		DTEXDeepImageReader();
		DTEXDeepImageReader( const std::string &filename );

		virtual ~DTEXDeepImageReader();

		static bool canRead( const std::string &filename );

		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();

	protected :

		virtual IECore::DeepPixelPtr doReadPixel( int x, int y );

	private :

		static const ReaderDescription<DTEXDeepImageReader> g_readerDescription;

		/// Tries to open the file, returning true on success and false on failure. On success,
		/// all of the private members will be valid. If throwOnFailure is true then a descriptive
		/// Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );
		void cleanRixInterface();
		
		RixDeepTexture::DeepFile *m_inputFile;
		RixDeepTexture::DeepCache *m_dtexCache;
		RixDeepTexture::DeepImage *m_dtexImage;
		RixDeepTexture::DeepPixel *m_dtexPixel;
		Imath::Box2i m_dataWindow;
		std::string m_inputFileName;
		std::string m_channelNames;

};

IE_CORE_DECLAREPTR( DTEXDeepImageReader );

} // namespace IECoreRI

#endif // IECORERI_DTEXDEEPIMAGEREADER_H

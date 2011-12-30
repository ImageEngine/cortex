#ifndef IECORERI_DTEXDEEPIMAGEWRITER_H
#define IECORERI_DTEXDEEPIMAGEWRITER_H

#include "RixDeepTexture.h"

#include "IECore/DeepImageWriter.h"

#include "IECoreRI/TypeIds.h"

namespace IECoreRI
{

/// The DTEXDeepImageWriter class writes PRMan deep texture files.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class DTEXDeepImageWriter : public IECore::DeepImageWriter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( DTEXDeepImageWriter, DTEXDeepImageWriterTypeId, IECore::DeepImageWriter );

		DTEXDeepImageWriter();
		DTEXDeepImageWriter( const std::string &filename );

		virtual ~DTEXDeepImageWriter();

		static bool canWrite( const std::string &filename );

	private :

		static const DeepImageWriterDescription<DTEXDeepImageWriter> g_writerDescription;

		virtual void doWritePixel( int x, int y, const IECore::DeepPixel *pixel );

		/// Tries to open the file for writing, throwing on failure. On success,
		/// all of the private members will be valid.
		void open();
		void cleanRixInterface();
		
		IECore::V2iParameterPtr m_tileSizeParameter;
		
		RixDeepTexture::DeepFile *m_outputFile;
		RixDeepTexture::DeepCache *m_dtexCache;
		RixDeepTexture::DeepImage *m_dtexImage;
		RixDeepTexture::DeepPixel *m_dtexPixel;
		std::string m_outputFileName;

};

IE_CORE_DECLAREPTR( DTEXDeepImageWriter );

} // namespace IECoreRI

#endif // IECORERI_DTEXDEEPIMAGEWRITER_H

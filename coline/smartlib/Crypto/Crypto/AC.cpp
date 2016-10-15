//AC编码算法
#include <Crypto/AC.h>
#include <memory>

using namespace std;

CACEncoder::CACEncoder(): 
            m_pBufferOut(NULL), 
			m_dwDestCount(0)
{
	//
}

CACEncoder::CACEncoder(char * pBufferIn, DWORD dwSrcCount):
            m_pBufferIn(pBufferIn), 
			m_dwSrcCount(dwSrcCount), 
			m_pBufferOut(NULL), 
			m_dwDestCount(0)
{
	// Initialize the modeling data arrays
	InitModel();
}

// Set the input buffer, clear the output
void CACEncoder::SetBuffer(char * pBufferIn, DWORD dwSrcCount)
{
	// Set the buffer
	m_pBufferIn = pBufferIn;
	m_dwSrcCount = dwSrcCount;

	// Clean up
	if (m_pBufferOut)
		delete [] m_pBufferOut;
	m_pBufferOut = NULL;
	m_dwDestCount = 0;

	// Initialize the modeling data arrays
	InitModel();
}

// Get the output buffer
void CACEncoder::GetBuffer(char ** ppBufferOut, DWORD ** ppdwDestCount)
{
	// Set the buffer
	*ppBufferOut = m_pBufferOut;
	*ppdwDestCount = &m_dwDestCount;
}

CACEncoder::~CACEncoder()
{
	// Cleanup the output
	if (m_pBufferOut)
		delete [] m_pBufferOut;
}

// Initialze the modeling data
void CACEncoder::InitModel()
{
	memset(&m_ac, 0, sizeof(m_ac));
	memset(&m_ac2, 0, sizeof(m_ac2));
	memset(&m_ar, 0, sizeof(m_ar));
	memset(&m_aMap, 0, sizeof(m_aMap));
}

// Scale the counts for AC
__inline void CACEncoder::ScaleCounts()
{
	// Make a copy for 0 count checking
	for (int i = 0;i < 256;++i)
		m_ac2[i] = m_ac[i];

	// Scale to restrict to 14 bits for precision
	for (;;)
	{
		DWORD dwTotal = 0;
		for (int i = 0; i < 256; ++i)
		{
			dwTotal += m_ac2[i];
			if (dwTotal > 16383)
			{
				for (int j = 0; j < 256; ++j)
				{
					m_ac2[j] >>= 1;
					if (m_ac2[j] == 0 && m_ac[j] != 0)
						m_ac2[j] = 1;
				}
				break;
			}
		}
		if (dwTotal > 16383)
			continue;
		break;
	}
}

// Build the scaled range values
__inline DWORD CACEncoder::RangeCounts()
{
	DWORD dwScale = 0;
	for (int i = 0; i < 256; ++i)
	{
		m_ar[i] = dwScale;
		dwScale += m_ac2[i];
	}
	m_ar[256] = dwScale;

	// Return the scale
	return dwScale;
}

// Build the map that maps the range values back to a symbol for fast decoding
__inline void CACEncoder::BuildMap()
{
	for (DWORD i = 0; i < 256; ++i)
		if (m_ac2[i])
			for (DWORD dwRange = m_ar[i]; dwRange < m_ar[i + 1]; ++dwRange)
				m_aMap[dwRange] = i;
}

// Output a bit
__inline void CACEncoder::OutputBit(WORD wBit, BYTE  &byteOutSymbol, BYTE &byteBitMask,
									DWORD &dwDestCount, BYTE *pBuffer)
{
	// Output the most significant bit
	if (wBit)
		byteOutSymbol |= byteBitMask;
	byteBitMask >>= 1;

	// Test for output
	if (byteBitMask == 0)
	{
		// Output encoded byte
		pBuffer[dwDestCount++] = byteOutSymbol;
		byteOutSymbol = 0;
		byteBitMask = BITMASK;
	}
}

// Output the underflow bits
__inline void CACEncoder::OutputUnderflowBits(WORD wBit, DWORD &dwUnderflowBits,  BYTE &byteOutSymbol, 
											  BYTE &byteBitMask, DWORD &dwDestCount, BYTE *pBuffer)
{
	// Account for all underflow bits
	while (dwUnderflowBits)
	{
		// Output the most significant bit
		OutputBit(wBit, byteOutSymbol, byteBitMask, dwDestCount, pBuffer);

		// Decrement the count
		dwUnderflowBits--;
	}
}

// Flush the bitmask, simulating shifting in bits
__inline void CACEncoder::FlushBitMask(BYTE  &byteBitMask, BYTE &byteOutSymbol,
									   DWORD &dwDestCount, BYTE *pBuffer)
{
	while (byteBitMask)
	{
		byteOutSymbol |= byteBitMask;
		byteBitMask >>= 1;
	}

	// Output remaining encoded byte
	pBuffer[dwDestCount++] = byteOutSymbol;
	byteOutSymbol = 0;
	byteBitMask = BITMASK;
}

// Implementation of Encode
BOOL CACEncoder::EncodeBuffer(char * pBufferIn, DWORD dwSrcCount, char **ppBufferOut, DWORD *pdwDestCount)
{
	// Allocate the maximum output buffer (Total bytes encoded + Total symbol count + Symbols and counts + Max Output data)
	unsigned long ulnDestCount = 4 + 1 + 1280 + dwSrcCount;
	*ppBufferOut = new char[ulnDestCount];
	unsigned char * pBufferOut = (unsigned char *)*ppBufferOut;
	return Encode(pBufferIn, dwSrcCount, pBufferOut, *pdwDestCount);
}

//已经分配好内存的编解码
BOOL CACEncoder::Encode(char *pBufferIn, DWORD dwSrcCount, BYTE *pBufferOut, DWORD &dwDestCount)
{
	// Initialize the modeling data arrays
	InitModel();

	// Input buffer
	unsigned char * pBuffer = (unsigned char *)pBufferIn;

	// The symbol and count
	unsigned char ucSymbol = 0;
	unsigned int uinSymbol = 0;

	// Counts and scaled counts
	unsigned long ulByte;
	for (ulByte = 0; ulByte < dwSrcCount; ++ulByte)
		uinSymbol += (m_ac[pBuffer[ulByte]]++ ? 0 : 1);
	if (!uinSymbol)
		return false;

	// Scale the counts
	ScaleCounts();

	// Get the count ranges
	unsigned int uiScale = RangeCounts();
	// Starting buffer position is at the total symbol count
	unsigned long ulDestCount = 4;

	// Write the count of symbols modeling data
	pBufferOut[ulDestCount++] = (unsigned char)(uinSymbol - 1);

	// Write the symbol counts modeling data
	ucSymbol = 0; 
	do
	{
		if (m_ac[ucSymbol])
		{
			pBufferOut[ulDestCount++] = ucSymbol;
			pBufferOut[ulDestCount++] = (unsigned char)((m_ac[ucSymbol] & 0xFF000000) >> 24);
			pBufferOut[ulDestCount++] = (unsigned char)((m_ac[ucSymbol] & 0x00FF0000) >> 16);
			pBufferOut[ulDestCount++] = (unsigned char)((m_ac[ucSymbol] & 0x0000FF00) >> 8);
			pBufferOut[ulDestCount++] = (unsigned char)(m_ac[ucSymbol] & 0x000000FF);
		}
	} while (++ucSymbol);

	// Encode the data
	unsigned short int usiLow = 0,usiHigh = 0xFFFF;
	unsigned int uiRange = 0;
	unsigned long ulUnderflowBits = 0;

	// Output tracking
	unsigned char ucOutSymbol = 0;
	unsigned char ucBitMask = BITMASK;

	// Output bit
	unsigned short int usiBit = 0;

	// Main loop
	for (ulByte = 0;ulByte < dwSrcCount;++ulByte)
	{
		// Get the symbol
		ucSymbol = pBuffer[ulByte];

		// Calculate the range, high value, and low value
		uiRange = (unsigned int)(usiHigh - usiLow + 1);
		usiHigh = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol + 1]) / uiScale - 1);
		usiLow = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol]) / uiScale);

		// Build onto the output
		for (;;)
		{
			// Check for output
			usiBit = usiHigh & MSB;
			if (usiBit == (usiLow & MSB))
			{
				// Output the most significant bit
				OutputBit(usiBit,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);

				// Output previous underflow bits first
				if (ulUnderflowBits > 0)
				{
					// Get the underflow bit
					usiBit = ~usiHigh & MSB;

					// Output the underflow bits
					OutputUnderflowBits(usiBit,ulUnderflowBits,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);
				}
			}
			else if ((usiLow & NSB) && !(usiHigh & NSB))
			{
				// Underflow prevention
				ulUnderflowBits++;
				usiHigh |= NSB;
				usiLow &= USB;
			}
			else
				break;

			// Shift the inputs
			usiHigh = usiHigh << 1;
			usiHigh |= 1;
			usiLow = usiLow << 1;
		}

		// Update the symbol count
		if (!(--m_ac[ucSymbol]))
		{
			// Scale the counts for the removed symbol
			ScaleCounts();

			// Get the range for the counts
			uiScale = RangeCounts();
		}
	}

	// Flush the encoder of the 2 low MSB's and any remaing underflow
	usiBit = usiLow & NSB;
	OutputBit(usiBit,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);
	usiBit = ~usiLow & NSB;
	OutputBit(usiBit,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);

	// Output the remaining underflow bits
	if (ulUnderflowBits > 0)
		OutputUnderflowBits(usiBit,ulUnderflowBits,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);

	// Flush out the bitmask
	if (ucBitMask)
		FlushBitMask(ucBitMask, ucOutSymbol, ulDestCount, pBufferOut);

	// Output the total bytes encoded
	pBufferOut[0] = (unsigned char)((dwSrcCount & 0xFF000000) >> 24);
	pBufferOut[1] = (unsigned char)((dwSrcCount & 0x00FF0000) >> 16);
	pBufferOut[2] = (unsigned char)((dwSrcCount & 0x0000FF00) >> 8);
	pBufferOut[3] = (unsigned char) (dwSrcCount & 0x000000FF);

	// Update the destination count
	dwDestCount = ulDestCount;

	return true;
}

// Implementation of Encode
BOOL CACEncoder::EncodeBuffer()
{
	return EncodeBuffer(m_pBufferIn, m_dwSrcCount, &m_pBufferOut, &m_dwDestCount);
}


// Implementation of decode
BOOL CACEncoder::DecodeBuffer(char * pBufferIn, DWORD dwSrcCount, char **ppBufferOut, DWORD *pdwDestCount)
{
	// Input buffer
	unsigned char * pBuffer = (unsigned char *)pBufferIn;
	unsigned long ulSrcCount = 0;

	// Read the total bytes encoded
	unsigned char uc4 = pBuffer[ulSrcCount++];
	unsigned char uc3 = pBuffer[ulSrcCount++];
	unsigned char uc2 = pBuffer[ulSrcCount++];
	unsigned char uc1 = pBuffer[ulSrcCount++];

	// Calculate the total bytes
	unsigned long ulTotal = (uc4 << 24) + (uc3 << 16) + (uc2 << 8) + uc1;
	*ppBufferOut = new char[ulTotal];
	unsigned char * pBufferOut = (unsigned char *)*ppBufferOut;
	*pdwDestCount = ulTotal;
	return Decode(pBufferIn, dwSrcCount, (char *)pBufferOut, *pdwDestCount);
}

BOOL CACEncoder::Decode(char *pBufferIn, DWORD dwSrcCount, char *pBufferOut, DWORD &dwDestCount)
{
	// Input buffer
	unsigned char * pBuffer = (unsigned char *)pBufferIn;
	unsigned long ulSrcCount = 0;

	// Read the total bytes encoded
	unsigned char uc4 = pBuffer[ulSrcCount++];
	unsigned char uc3 = pBuffer[ulSrcCount++];
	unsigned char uc2 = pBuffer[ulSrcCount++];
	unsigned char uc1 = pBuffer[ulSrcCount++];

	// Calculate the total bytes
	unsigned long ulTotal = (uc4 << 24) + (uc3 << 16) + (uc2 << 8) + uc1;
	if (dwDestCount < ulTotal)
		return FALSE;
	dwDestCount = ulTotal;

	// Initialize the modeling data arrays
	InitModel();
	// The symbol
	unsigned char ucSymbol = 0;

	// Read the count of modeling data
	unsigned int uinSymbol = pBuffer[ulSrcCount++] + 1;

	// Read the modeling data
	for (unsigned int uin = 0;uin < uinSymbol;++uin)
	{
		ucSymbol = pBuffer[ulSrcCount++];
		uc4 = pBuffer[ulSrcCount++];
		uc3 = pBuffer[ulSrcCount++];
		uc2 = pBuffer[ulSrcCount++];
		uc1 = pBuffer[ulSrcCount++];
		unsigned long ulSymbolTotal = (uc4 << 24) + (uc3 << 16) + (uc2 << 8) + uc1;
		m_ac[ucSymbol] = ulSymbolTotal;
	}

	// Scale the counts
	ScaleCounts();

	// Get the range of counts
	unsigned int uiScale = RangeCounts();

	// Build the decode map of range to symbol for fast lookup
	BuildMap();

	// Initialize the code variables
	unsigned short int usiCode = 0;
	unsigned char ucCode = 0;

	// Read the first code word
	ucCode = pBuffer[ulSrcCount++];
	usiCode |= ucCode;
	usiCode <<= 8;
	ucCode = pBuffer[ulSrcCount++];
	usiCode |= ucCode;

	// Initialize the count of decoded characters and code
	unsigned long ulDestCount = 0;

	// Initialize the range
	unsigned short int usiLow = 0,usiHigh = 0xFFFF;
	unsigned int uiRange = 0;
	unsigned long ulUnderflowBits = 0;
	unsigned short int usiCount = 0;

	// The bit mask tracks the remaining bits in the input code buffer
	unsigned char ucBitMask = 0;

	// The main decoding loop
	do
	{
		// Get the range and count for the current arithmetic code
		uiRange = (unsigned int)(usiHigh - usiLow + 1);
		usiCount = (unsigned short int)((((usiCode - usiLow) + 1) * uiScale - 1) / uiRange);

		// Look up the symbol in the map
		ucSymbol = m_aMap[usiCount];

		// Output the symbol
		pBufferOut[ulDestCount++] = ucSymbol;
		if (ulDestCount >= ulTotal)
			break;

		// Calculate the high and low value of the symbol
		usiHigh = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol + 1]) / uiScale - 1);
		usiLow = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol]) / uiScale);

		// Remove the symbol from the stream
		for (;;)
		{
			if ((usiHigh & MSB) == (usiLow & MSB))
			{
				// Fall through to shifting
			}
			else if ((usiLow & NSB) == NSB && (usiHigh & NSB) == 0)
			{
				// Account for underflow
				usiCode ^= NSB;
				usiHigh |= NSB;
				usiLow &= USB;
			}
			else
				break;

			// Shift out a bit from the low and high values
			usiHigh <<= 1;
			usiHigh |= 1;
			usiLow <<= 1;

			// Shift out a bit from the code
			usiCode <<= 1;

			// Test for a needing an input symbol
			if (ucBitMask)
			{
shift:

				// Test for shifting in bits
				if (ucCode & ucBitMask)
					usiCode |= 1;
				ucBitMask >>= 1;
			}
			else
			{
				// Load up a new code
				if (ulSrcCount < dwSrcCount)
				{
					ucCode = pBuffer[ulSrcCount++];
					ucBitMask = BITMASK;
					goto shift;
				}
				else if (ulDestCount < ulTotal)
				{
					// Helper with the last remaining symbols
					ucBitMask = BITMASK;
					goto shift;
				}
			}
		}

		// Update the symbol count
		if (!(--m_ac[ucSymbol]))
		{
			// Scale the counts for the removed symbol
			ScaleCounts();

			// Get the range of counts
			uiScale = RangeCounts();

			// Build the lookup map
			BuildMap();
		}
	} while (ulDestCount < ulTotal);

	return true;
}

// Implementation of decode
BOOL CACEncoder::DecodeBuffer()
{
	return DecodeBuffer(m_pBufferIn, m_dwSrcCount, &m_pBufferOut, &m_dwDestCount);
}

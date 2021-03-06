/*
 * RfbLZORLE.cpp - LZO+RLE-based VNC rectangle encoding
 *
 * Copyright (c) 2010-2017 Tobias Doerffel <tobydox/at/users/dot/sf/dot/net>
 *
 * This file is part of iTALC - http://italc.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "RfbLZORLE.h"

#include <lzo/lzo1x.h>

#include <QtGui/QColor>

#include <rfb/rfb.h>
#include <rfb/rfbclient.h>


static void CopyRectangle( rfbClient* client, uint8_t* buffer, int x, int y, int w, int h )
{
	const int rs = w*4, rs2 = client->width * 4;
	for( int j = (x *4) + (y * rs2); j < (y + h) * rs2; j += rs2 )
	{
		memcpy( client->frameBuffer + j, buffer, rs );
		buffer += rs;
	}
}


static bool handleRaw( rfbClient *c, int rx, int ry, int rw, int rh )
{
	int y=ry, h=rh;
	int bytesPerLine = rw * c->format.bitsPerPixel / 8;
	int linesToRead = RFB_BUFFER_SIZE / bytesPerLine;

	while( h > 0 )
	{
		if( linesToRead > h )
			linesToRead = h;

		if( !ReadFromRFBServer( c, c->buffer,bytesPerLine * linesToRead ) )
			return false;

		CopyRectangle( c, (uint8_t *)c->buffer, rx, y, rw,linesToRead );

		h -= linesToRead;
		y += linesToRead;
	}

	return true;
}




#define rfbClientSwap24(l) ((((l) & 0xff) << 16) | (((l) >> 16) & 0xff) | \
				   (((l) & 0x00ff00)))
#define rfbClientSwap24IfLE(l) (*(char *)&client->endianTest ? Swap24(l) : (l))



static rfbBool handleEncodingLZORLE( rfbClient *client,
										rfbFramebufferUpdateRectHeader *r )
{
	if( r->encoding != rfbEncodingLZORLE )
	{
		return false;
	}

	uint16_t rx = r->r.x;
	uint16_t ry = r->r.y;
	const uint16_t rw = r->r.w;
	const uint16_t rh = r->r.h;

	RfbLZORLE::Header hdr;
	if( !ReadFromRFBServer( client, (char *) &hdr, sizeof( hdr ) ) )
	{
		qWarning( "failed reading RfbLZORLE::Header from server" );
		return false;
	}

	if( !hdr.compressed )
	{
		return handleRaw( client, rx, ry, rw, rh );
	}

	hdr.bytesLZO = rfbClientSwap32IfLE( hdr.bytesLZO );
	hdr.bytesRLE = rfbClientSwap32IfLE( hdr.bytesRLE );

	auto lzo_data = new uint8_t[hdr.bytesLZO];

	if( !ReadFromRFBServer( client, (char *) lzo_data, hdr.bytesLZO ) )
	{
		qWarning( "failed reading LZO data from server" );
		delete[] lzo_data;
		return false;
	}

	auto rle_data = new uint8_t[hdr.bytesRLE];

	lzo_uint decomp_bytes = hdr.bytesRLE;
	lzo1x_decompress_safe( (const unsigned char *) lzo_data,
				(lzo_uint) hdr.bytesLZO,
				(unsigned char *) rle_data,
				(lzo_uint *) &decomp_bytes, nullptr );
	if( decomp_bytes != hdr.bytesRLE )
	{
		delete[] rle_data;
		delete[] lzo_data;
		qCritical( "handleEncodingLZORLE(...): expected and real "
					"size of decompressed data do not match!" );
		return false;
	}

	QRgb *dst = ( (QRgb *) client->frameBuffer ) + client->width*ry + rx;
	int dx = 0;
	bool done = FALSE;
	const int sh = client->height;
	for( uint32_t i = 0; i < hdr.bytesRLE && done == false; i+=4 )
	{
		const QRgb val = rfbClientSwap24IfLE( *( (QRgb*)( rle_data + i ) ) );
		const uint8_t rleCount = rle_data[i+3];
		for( uint16_t j = 0; j <= rleCount; ++j )
		{
			*dst = val;
			if( ++dx >= rw )
			{
				dx = 0;
				if( ry+1 < sh )
				{
					++ry;
					dst = ( (QRgb *) client->frameBuffer ) + client->width*ry + rx;
				}
				else
				{
					done = true;
					break;
				}
			}
			else
			{
				++dst;
			}
		}
	}

	if( dx != 0 )
	{
		qWarning( "handleEncodingLZORLE(...): dx(%d) != 0", dx );
	}

	delete[] lzo_data;
	delete[] rle_data;

	return true;
}




static rfbClientProtocolExtension * __lzoRleProtocolExt = nullptr;


RfbLZORLE::RfbLZORLE()
{
	if( __lzoRleProtocolExt == nullptr )
	{
		__lzoRleProtocolExt = new rfbClientProtocolExtension;
		__lzoRleProtocolExt->encodings = new int[2];
		__lzoRleProtocolExt->encodings[0] = rfbEncodingLZORLE;
		__lzoRleProtocolExt->encodings[1] = 0;
		__lzoRleProtocolExt->handleEncoding = handleEncodingLZORLE;
		__lzoRleProtocolExt->handleMessage = nullptr;

		rfbClientRegisterExtension( __lzoRleProtocolExt );
	}
}

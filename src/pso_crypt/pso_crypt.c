#include <stdint.h>
#include "common/def_structs.h"
#include "pso_crypt.h"

/* Blue Burst encryption routines */

void pso_crypt_init_key_bb(uint8_t *data)
{
  uint32_t x;
  for (x = 0; x < 48; x += 3)
  {
    data[x] ^= 0x19;
    data[x + 1] ^= 0x16;
    data[x + 2] ^= 0x18;
  }
}

void pso_crypt_decrypt_bb(PSO_CRYPT *pcry, uint8_t *data, unsigned
              length)
{
  uint32_t edx, ebx, ebp, esi, edi;

  edx = 0;
  while (edx < length)
  {
    ebx = *(uint32_t *) &data[edx];
    ebx = ebx ^ pcry->tbl[5];
    ebp = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
    ebp = ebp ^ pcry->tbl[4];
    ebp ^= *(uint32_t *) &data[edx+4];
    edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
    edi = edi ^ pcry->tbl[3];
    ebx = ebx ^ edi;
    esi = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
    ebp = ebp ^ esi ^ pcry->tbl[2];
    edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
    edi = edi ^ pcry->tbl[1];
    ebp = ebp ^ pcry->tbl[0];
    ebx = ebx ^ edi;
    *(uint32_t *) &data[edx] = ebp;
    *(uint32_t *) &data[edx+4] = ebx;
    edx = edx+8;
  }
}


void pso_crypt_encrypt_bb(PSO_CRYPT *pcry, uint8_t *data, unsigned
              length)
{
  uint32_t edx, ebx, ebp, esi, edi;

  edx = 0;
  while (edx < length)
  {
    ebx = *(uint32_t *) &data[edx];
    ebx = ebx ^ pcry->tbl[0];
    ebp = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
    ebp = ebp ^ pcry->tbl[1];
    ebp ^= *(uint32_t *) &data[edx+4];
    edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
    edi = edi ^ pcry->tbl[2];
    ebx = ebx ^ edi;
    esi = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
    ebp = ebp ^ esi ^ pcry->tbl[3];
    edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
      ^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
    edi = edi ^ pcry->tbl[4];
    ebp = ebp ^ pcry->tbl[5];
    ebx = ebx ^ edi;
    *(uint32_t *) &data[edx] = ebp;
    *(uint32_t *) &data[edx+4] = ebx;
    edx = edx+8;
  }
}

void encryptcopy (BANANA* client, const uint8_t* src, uint32_t size)
{
  uint8_t* dest;

  if (TCP_BUFFER_SIZE - client->snddata < ( (int) size + 7 ) )
    client->todc = 1;
  else
  {
    dest = &client->sndbuf[client->snddata];
    memcpy (dest,src,size);
    while (size % 8)
      dest[size++] = 0x00;
    client->snddata += (int) size;
    pso_crypt_encrypt_bb(cipher_ptr,dest,size);
  }
}


void decryptcopy (uint8_t* dest, const uint8_t* src, uint32_t size)
{
  memcpy (dest,src,size);
  pso_crypt_decrypt_bb(cipher_ptr,dest,size);
}


void pso_crypt_table_init_bb(PSO_CRYPT *pcry, const uint8_t *salt)
{
  uint32_t eax, ecx, edx, ebx, ebp, esi, edi, ou, x;
  uint8_t s[48];
  uint16_t* pcryp;
  uint16_t* bbtbl;
  uint16_t dx;

  pcry->cur = 0;
  pcry->mangle = NULL;
  pcry->size = 1024 + 18;

  memcpy(s, salt, sizeof(s));
  pso_crypt_init_key_bb(s);

  bbtbl = (uint16_t*) &bbtable[0];
  pcryp = (uint16_t*) &pcry->tbl[0];

  eax = 0;
  ebx = 0;

  for(ecx=0;ecx<0x12;ecx++)
  {
    dx = bbtbl[eax++];
    dx = ( ( dx & 0xFF ) << 8 ) + ( dx >> 8 );
    pcryp[ebx] = dx;
    dx = bbtbl[eax++];
    dx ^= pcryp[ebx++];
    pcryp[ebx++] = dx;
  }

  /*

  pcry->tbl[0] = 0x243F6A88;
  pcry->tbl[1] = 0x85A308D3;
  pcry->tbl[2] = 0x13198A2E;
  pcry->tbl[3] = 0x03707344;
  pcry->tbl[4] = 0xA4093822;
  pcry->tbl[5] = 0x299F31D0;
  pcry->tbl[6] = 0x082EFA98;
  pcry->tbl[7] = 0xEC4E6C89;
  pcry->tbl[8] = 0x452821E6;
  pcry->tbl[9] = 0x38D01377;
  pcry->tbl[10] = 0xBE5466CF;
  pcry->tbl[11] = 0x34E90C6C;
  pcry->tbl[12] = 0xC0AC29B7;
  pcry->tbl[13] = 0xC97C50DD;
  pcry->tbl[14] = 0x3F84D5B5;
  pcry->tbl[15] = 0xB5470917;
  pcry->tbl[16] = 0x9216D5D9;
  pcry->tbl[17] = 0x8979FB1B;

  */

  memcpy(&pcry->tbl[18], &bbtable[18], 4096);

  ecx=0;
  //total key[0] length is min 0x412
  ebx=0;

  while (ebx < 0x12)
  {
    //in a loop
    ebp=((uint32_t) (s[ecx])) << 0x18;
    eax=ecx+1;
    edx=eax-((eax / 48)*48);
    eax=(((uint32_t) (s[edx])) << 0x10) & 0xFF0000;
    ebp=(ebp | eax) & 0xffff00ff;
    eax=ecx+2;
    edx=eax-((eax / 48)*48);
    eax=(((uint32_t) (s[edx])) << 0x8) & 0xFF00;
    ebp=(ebp | eax) & 0xffffff00;
    eax=ecx+3;
    ecx=ecx+4;
    edx=eax-((eax / 48)*48);
    eax=(uint32_t) (s[edx]);
    ebp=ebp | eax;
    eax=ecx;
    edx=eax-((eax / 48)*48);
    pcry->tbl[ebx]=pcry->tbl[ebx] ^ ebp;
    ecx=edx;
    ebx++;
  }

  ebp=0;
  esi=0;
  ecx=0;
  edi=0;
  ebx=0;
  edx=0x48;

  while (edi < edx)
  {
    esi=esi ^ pcry->tbl[0];
    eax=esi >> 0x18;
    ebx=(esi >> 0x10) & 0xff;
    eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
    ebx=(esi >> 8) & 0xFF;
    eax=eax ^ pcry->tbl[ebx+0x212];
    ebx=esi & 0xff;
    eax=eax + pcry->tbl[ebx+0x312];

    eax=eax ^ pcry->tbl[1];
    ecx= ecx ^ eax;
    ebx=ecx >> 0x18;
    eax=(ecx >> 0x10) & 0xFF;
    ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
    eax=(ecx >> 8) & 0xff;
    ebx=ebx ^ pcry->tbl[eax+0x212];
    eax=ecx & 0xff;
    ebx=ebx + pcry->tbl[eax+0x312];

    for (x = 0; x <= 5; x++)
    {
      ebx=ebx ^ pcry->tbl[(x*2)+2];
      esi= esi ^ ebx;
      ebx=esi >> 0x18;
      eax=(esi >> 0x10) & 0xFF;
      ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
      eax=(esi >> 8) & 0xff;
      ebx=ebx ^ pcry->tbl[eax+0x212];
      eax=esi & 0xff;
      ebx=ebx + pcry->tbl[eax+0x312];

      ebx=ebx ^ pcry->tbl[(x*2)+3];
      ecx= ecx ^ ebx;
      ebx=ecx >> 0x18;
      eax=(ecx >> 0x10) & 0xFF;
      ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
      eax=(ecx >> 8) & 0xff;
      ebx=ebx ^ pcry->tbl[eax+0x212];
      eax=ecx & 0xff;
      ebx=ebx + pcry->tbl[eax+0x312];
    }

    ebx=ebx ^ pcry->tbl[14];
    esi= esi ^ ebx;
    eax=esi >> 0x18;
    ebx=(esi >> 0x10) & 0xFF;
    eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
    ebx=(esi >> 8) & 0xff;
    eax=eax ^ pcry->tbl[ebx+0x212];
    ebx=esi & 0xff;
    eax=eax + pcry->tbl[ebx+0x312];

    eax=eax ^ pcry->tbl[15];
    eax= ecx ^ eax;
    ecx=eax >> 0x18;
    ebx=(eax >> 0x10) & 0xFF;
    ecx=pcry->tbl[ecx+0x12]+pcry->tbl[ebx+0x112];
    ebx=(eax >> 8) & 0xff;
    ecx=ecx ^ pcry->tbl[ebx+0x212];
    ebx=eax & 0xff;
    ecx=ecx + pcry->tbl[ebx+0x312];

    ecx=ecx ^ pcry->tbl[16];
    ecx=ecx ^ esi;
    esi= pcry->tbl[17];
    esi=esi ^ eax;
    pcry->tbl[(edi / 4)]=esi;
    pcry->tbl[(edi / 4)+1]=ecx;
    edi=edi+8;
  }


  eax=0;
  edx=0;
  ou=0;
  while (ou < 0x1000)
  {
    edi=0x48;
    edx=0x448;

    while (edi < edx)
    {
      esi=esi ^ pcry->tbl[0];
      eax=esi >> 0x18;
      ebx=(esi >> 0x10) & 0xff;
      eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
      ebx=(esi >> 8) & 0xFF;
      eax=eax ^ pcry->tbl[ebx+0x212];
      ebx=esi & 0xff;
      eax=eax + pcry->tbl[ebx+0x312];

      eax=eax ^ pcry->tbl[1];
      ecx= ecx ^ eax;
      ebx=ecx >> 0x18;
      eax=(ecx >> 0x10) & 0xFF;
      ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
      eax=(ecx >> 8) & 0xff;
      ebx=ebx ^ pcry->tbl[eax+0x212];
      eax=ecx & 0xff;
      ebx=ebx + pcry->tbl[eax+0x312];

      for (x = 0; x <= 5; x++)
      {
        ebx=ebx ^ pcry->tbl[(x*2)+2];
        esi= esi ^ ebx;
        ebx=esi >> 0x18;
        eax=(esi >> 0x10) & 0xFF;
        ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
        eax=(esi >> 8) & 0xff;
        ebx=ebx ^ pcry->tbl[eax+0x212];
        eax=esi & 0xff;
        ebx=ebx + pcry->tbl[eax+0x312];

        ebx=ebx ^ pcry->tbl[(x*2)+3];
        ecx= ecx ^ ebx;
        ebx=ecx >> 0x18;
        eax=(ecx >> 0x10) & 0xFF;
        ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
        eax=(ecx >> 8) & 0xff;
        ebx=ebx ^ pcry->tbl[eax+0x212];
        eax=ecx & 0xff;
        ebx=ebx + pcry->tbl[eax+0x312];
      }

      ebx=ebx ^ pcry->tbl[14];
      esi= esi ^ ebx;
      eax=esi >> 0x18;
      ebx=(esi >> 0x10) & 0xFF;
      eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
      ebx=(esi >> 8) & 0xff;
      eax=eax ^ pcry->tbl[ebx+0x212];
      ebx=esi & 0xff;
      eax=eax + pcry->tbl[ebx+0x312];

      eax=eax ^ pcry->tbl[15];
      eax= ecx ^ eax;
      ecx=eax >> 0x18;
      ebx=(eax >> 0x10) & 0xFF;
      ecx=pcry->tbl[ecx+0x12]+pcry->tbl[ebx+0x112];
      ebx=(eax >> 8) & 0xff;
      ecx=ecx ^ pcry->tbl[ebx+0x212];
      ebx=eax & 0xff;
      ecx=ecx + pcry->tbl[ebx+0x312];

      ecx=ecx ^ pcry->tbl[16];
      ecx=ecx ^ esi;
      esi= pcry->tbl[17];
      esi=esi ^ eax;
      pcry->tbl[(ou / 4)+(edi / 4)]=esi;
      pcry->tbl[(ou / 4)+(edi / 4)+1]=ecx;
      edi=edi+8;
    }
    ou=ou+0x400;
  }
}

uint32_t RleEncode(uint8_t *src, uint8_t *dest, uint32_t src_size)
{
  uint8_t currChar, prevChar;             /* current and previous characters */
  uint16_t count;                /* number of characters in a run */
  uint8_t *src_end, *dest_start;

  dest_start = dest;
  src_end = src + src_size;

  prevChar  = 0xFF - *src;

  while ( src < src_end)
  {
    currChar = *(dest++) = *(src++);

    if ( currChar == prevChar )
    {
      if ( src == src_end )
      {
        *(dest++) = 0;
        *(dest++) = 0;
      }
      else
      {
        count = 0;
        while (( src < src_end) && (count < 0xFFF0))
        {
          if (*src == prevChar)
          {
            count++;
            src++;
            if ( src == src_end )
            {
              *(uint16_t*) dest = count;
              dest += 2;
            }
          }
          else
          {
            *(uint16_t*) dest = count;
            dest += 2;
            prevChar = 0xFF - *src;
            break;
          }
        }
      }
    }
    else
      prevChar = currChar;
  }
  return dest - dest_start;
}

void RleDecode(uint8_t *src, uint8_t *dest, uint32_t src_size)
{
    uint8_t currChar, prevChar;             /* current and previous characters */
    uint16_t count;                /* number of characters in a run */
  uint8_t *src_end;

  src_end = src + src_size;

    /* decode */

    prevChar = 0xFF - *src;     /* force next int8_t to be different */

    /* read input until there's nothing left */

    while ( src < src_end)
    {
    currChar = *(src++);

    *(dest++) = currChar;

        /* check for run */
        if (currChar == prevChar)
        {
            /* we have a run.  write it out. */
            count = *(uint16_t*) src;
            src += 2;
            while (count > 0)
            {
              *(dest++) = currChar;
              count--;
            }

            prevChar = 0xFF - *src;     /* force next int8_t to be different */
        }
        else
        {
            /* no run */
            prevChar = currChar;
        }
    }
}


/* expand a key (makes a rc4_key) */

void prepare_key(uint8_t *keydata, uint32_t len, struct rc4_key *key)
{
    uint32_t index1, index2, counter;
    uint8_t *state;

    state = key->state;

    for (counter = 0; counter < 256; counter++)
        state[counter] = counter;

    key->x = key->y = index1 = index2 = 0;

    for (counter = 0; counter < 256; counter++) {
        index2 = (keydata[index1] + state[counter] + index2) & 255;

        /* swap */
        state[counter] ^= state[index2];
        state[index2]  ^= state[counter];
        state[counter] ^= state[index2];

        index1 = (index1 + 1) % len;
    }
}

/* reversible encryption, will encode a buffer updating the key */

void rc4(uint8_t *buffer, uint32_t len, struct rc4_key *key)
{
    uint32_t x, y, xorIndex, counter;
    uint8_t *state;

    /* get local copies */
    x = key->x; y = key->y;
    state = key->state;

    for (counter = 0; counter < len; counter++) {
        x = (x + 1) & 255;
        y = (state[x] + y) & 255;

        /* swap */
        state[x] ^= state[y];
        state[y] ^= state[x];
        state[x] ^= state[y];

        xorIndex = (state[y] + state[x]) & 255;

        buffer[counter] ^= state[xorIndex];
    }

    key->x = x; key->y = y;
}

void compressShipPacket ( ORANGE* ship, uint8_t* src, uint32_t src_size )
{
  uint8_t* dest;
  uint32_t result;

  if (ship->shipSockfd >= 0)
  {
    if (PACKET_BUFFER_SIZE - ship->snddata < (int) ( src_size + 100 ) )
      initialize_ship(ship);
    else
    {
      if ( ship->crypt_on )
      {
        dest = &ship->sndbuf[ship->snddata];
        // Store the original packet size before RLE compression at offset 0x04 of the new packet.
        dest += 4;
        *(unsigned *) dest = src_size;
        // Compress packet using RLE, storing at offset 0x08 of new packet.
        //
        // result = size of RLE compressed data + a DWORD for the original packet size.
        result = RleEncode (src, dest+4, src_size) + 4;
        // Encrypt with RC4
        rc4 (dest, result, &ship->sc_key);
        // Increase result by the size of a DWORD for the final ship packet size.
        result += 4;
        // Copy it to the front of the packet.
        *(unsigned *) &ship->sndbuf[ship->snddata] = result;
        ship->snddata += (int) result;
      }
      else
      {
        memcpy ( &ship->sndbuf[ship->snddata+4], src, src_size );
        src_size += 4;
        *(unsigned *) &ship->sndbuf[ship->snddata] = src_size;
        ship->snddata += src_size;
      }
    }
  }
}

void decompressShipPacket ( ORANGE* ship, uint8_t* dest, uint8_t* src )
{
  uint32_t src_size, dest_size;
  uint8_t *srccpy;

  if (ship->crypt_on)
  {
    src_size = *(unsigned *) src;
    src_size -= 8;
    src += 4;
    srccpy = src;
    // Decrypt RC4
    rc4 (src, src_size+4, &ship->cs_key);
    // The first four bytes of the src should now contain the expected uncompressed data size.
    dest_size = *(unsigned *) srccpy;
    // Increase expected size by 4 before inserting into the destination buffer.  (To take account for the packet
    // size DWORD...)
    dest_size += 4;
    *(unsigned *) dest = dest_size;
    // Decompress the data...
    RleDecode (srccpy+4, dest+4, src_size);
  }
  else
  {
    src_size = *(unsigned *) src;
    memcpy (dest + 4, src + 4, src_size);
    src_size += 4;
    *(unsigned *) dest = src_size;
  }
}

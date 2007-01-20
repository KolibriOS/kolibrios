#include "mpg123.h"
#include "..\kolibri.h"

#define MAXFRAMESIZE 3456

static int fsizeold=0,ssize;
static unsigned char bsspace[2][MAXFRAMESIZE+512]; /* MAXFRAMESIZE */
static unsigned char *bsbuf=bsspace[1],*bsbufold;
static int bsnum=0;

static unsigned long oldhead = 0;
unsigned long firsthead=0;

struct bitstream_info bsi;

int tabsel_123[2][3][16] = {
   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} },

   { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,} }
};

int freqs[9] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 , 12000 , 8000 };

int stream_head_read(struct reader *rd,unsigned long *newhead);
int stream_read_raw(struct reader *rd,unsigned char *buf, int size);

void set_synth_functions(struct frame *fr)
{
#ifdef USE_3DNOW	
	static func_dct36 funcs_dct36[2] = {dct36 , dct36_3dnow};
#endif

	fr->synth = synth_1to1; 
	fr->synth_mono = synth_1to1_mono2stereo;;

/* TODO: make autodetection for _all_ x86 optimizations (maybe just for i586+ and keep separate 486 build?) */
#ifdef USE_3DNOW
	/* check cpuflags bit 31 (3DNow!) and 23 (MMX) */
	if((param.stat_3dnow < 2) && 
	   ((param.stat_3dnow == 1) ||
	    (getcpuflags() & 0x80800000) == 0x80800000))
      	{
	  fr->synth = funcs[2][ds]; /* 3DNow! optimized synth_1to1() */
	  fr->dct36 = funcs_dct36[1]; /* 3DNow! optimized dct36() */
	}
	else
	{
	       	  fr->dct36 = funcs_dct36[0];
      	}
#endif
}

int __stdcall create_reader(struct reader *rd,byte *buffer, int buffsize)
{  rd->head_read = stream_head_read;
    rd->read_frame_body = stream_read_raw; 

    rd->buffer = buffer;
    rd->stream = buffer; 
    rd->strpos = 0;
      
    rd->strremain = 0;
    rd->filepos = 0;
    return 1; 
};

int __stdcall init_reader(struct reader *rd, char *file)
{  FILEINFO fileinfo;
    int retval;
    int bytes;
  
    rd->hFile = file;
    get_fileinfo(file, &fileinfo);

    rd->filelen = fileinfo.size; 
    rd->strpos = 0;
    retval=read_file (file,rd->buffer,0,0x10000,&bytes);
  
    if (retval) return 0; 
      
    rd->strremain = bytes;
    rd->filepos = bytes;
    return 1; 
};

static int fill_reader(struct reader *rd)
{  int retval;
    int bytes;
    
    mem_cpy(rd->buffer,rd->stream,rd->strremain);
    rd->stream = rd->buffer;
   
    retval=read_file (rd->hFile,rd->buffer+rd->strremain,rd->filepos,
                             0x10000-rd->strremain,&bytes);
    if (retval) return 0; 
    if(!bytes) return 0;    
    rd->strremain+=bytes;
    rd->filepos+=bytes;
    rd->strpos = 0;  
    return 1;
};

int __stdcall set_reader(struct reader *rd, unsigned int filepos)
{  int retval;
    unsigned int bytes;
    retval=read_file (rd->hFile,rd->buffer,filepos,0x10000,&bytes);
    if (retval) return 0; 
    rd->stream = rd->buffer; 
    rd->strremain=bytes;
    rd->filepos=filepos+bytes;
    rd->strpos = 0;
 
    fsizeold=0;
    firsthead=0;
    bsbufold = 0;
    bsbuf = bsspace[1];
    bsnum = 0;
    ssize=0;
    oldhead=0;   
    memset(bsspace,0,sizeof(bsspace));    
    return 1; 
};
    
static int stream_head_read(struct reader *rd,unsigned long *newhead)
{  
    if(rd->strremain < 4)
      if( !fill_reader(rd))
          return 0; 
    *newhead = (rd->stream[0]<<24)|(rd->stream[1] << 16)|
                      (rd->stream[2] << 8)| rd->stream[3];
    rd->strpos+=4;
    rd->stream+=4;
    rd->strremain-=4;  
    return TRUE;
};

int stream_read_raw(struct reader *rd,unsigned char *buf, int size)
{
    if(rd->strremain < size)
         if( !fill_reader(rd))
          return 0; 
 
    mem_cpy(buf,rd->stream,size);
    rd->strpos+=size;
    rd->stream+=size;
    rd->strremain-=size;  
    return 1;
};

void set_pointer(long backstep)
{
  bsi.wordpointer = bsbuf + ssize - backstep;
  if (backstep)
    mem_cpy(bsi.wordpointer,bsbufold+fsizeold-backstep,backstep);
  bsi.bitindex = 0; 
}

int head_check(unsigned long head)
{ 	if
	  (
		/* first 11 bits are set to 1 for frame sync */
		((head & 0xffe00000) != 0xffe00000)
		||
		/* layer: 01,10,11 is 1,2,3; 00 is reserved */
		(!((head>>17)&3))
		||
		/* 1111 means bad bitrate */
		(((head>>12)&0xf) == 0xf)
		||
		/* 0000 means free format... */
		(((head>>12)&0xf) == 0x0)
		||
		/* sampling freq: 11 is reserved */
		(((head>>10)&0x3) == 0x3 )
		/* here used to be a mpeg 2.5 check... re-enabled 2.5 decoding due to lack of evidence that it is really not good */
	)
	{
		return FALSE;
	}
	/* if no check failed, the header is valid (hopefully)*/
	else
	{
		return TRUE;
	}
}

int __stdcall decode_header(struct frame *fr,unsigned long newhead)
{  
    if(!head_check(newhead))
        return 0;
    if( newhead & (1<<20) )
    {  fr->lsf = (newhead & (1<<19)) ? 0x0 : 0x1;
        fr->mpeg25 = 0;
    }
    else
    {  fr->lsf = 1;
        fr->mpeg25 = 1;
    };

    fr->lay = 4-((newhead>>17)&3);
    if(fr->mpeg25)
        fr->sampling_frequency = 6 + ((newhead>>10)&0x3);
    else
        fr->sampling_frequency = ((newhead>>10)&0x3) + (fr->lsf*3);
        fr->error_protection = ((newhead>>16)&0x1)^0x1;
    
    fr->bitrate_index = ((newhead>>12)&0xf);
    fr->padding   = ((newhead>>9)&0x1);
    fr->extension = ((newhead>>8)&0x1);
    fr->mode      = ((newhead>>6)&0x3);
    fr->mode_ext  = ((newhead>>4)&0x3);
    fr->copyright = ((newhead>>3)&0x1);
    fr->original  = ((newhead>>2)&0x1);
    fr->emphasis  = newhead & 0x3;

    fr->stereo    = (fr->mode == MPG_MD_MONO) ? 1 : 2;

    oldhead = newhead;

    if(!fr->bitrate_index)
      return (0);

    switch(fr->lay)
    {  case 1:
	        fr->do_layer = do_layer1;
#ifdef VARMODESUPPORT
        if (varmode) {
          error("Sorry, layer-1 not supported in varmode."); 
          return (0);
        }
#endif
        fr->framesize  = (long) tabsel_123[fr->lsf][0][fr->bitrate_index] * 12000;
        fr->framesize /= freqs[fr->sampling_frequency];
        fr->framesize  = ((fr->framesize+fr->padding)<<2)-4;
        break;
      case 2:
	fr->do_layer = do_layer2;
#ifdef VARMODESUPPORT
        if (varmode) {
          error("Sorry, layer-2 not supported in varmode."); 
          return (0);
        }
#endif
        fr->framesize = (long) tabsel_123[fr->lsf][1][fr->bitrate_index] * 144000;
        fr->framesize /= freqs[fr->sampling_frequency];
        fr->framesize += fr->padding - 4;
        break;
      case 3:
        fr->do_layer = do_layer3;
        if(fr->lsf)
          ssize = (fr->stereo == 1) ? 9 : 17;
        else
          ssize = (fr->stereo == 1) ? 17 : 32;
        if(fr->error_protection)
          ssize += 2;
        fr->framesize  = (long) tabsel_123[fr->lsf][2][fr->bitrate_index] * 144000;
        fr->framesize /= freqs[fr->sampling_frequency]<<(fr->lsf);
        fr->framesize = fr->framesize + fr->padding - 4;
        break; 
      default:
        return (0);
    }
    if (fr->framesize > MAXFRAMESIZE)
      return (0);
    return 1;
}


int read_frame(struct reader *rd, struct frame *fr)
{	  unsigned long newhead;
    static unsigned char ssave[34];
	  //off_t framepos;
    fsizeold=fr->framesize;       /* for Layer3 */

read_again:
	
	  if(!rd->head_read(rd,&newhead))
			return FALSE;

    if(!decode_header(fr,newhead))
   { rd->strpos-=3;
      rd->stream-=3;
      rd->strremain+=3;
      goto read_again;
   };

#if 0	
  if(1 || oldhead != newhead || !oldhead)
  {

init_resync:

#ifdef SKIP_JUNK
	/* watch out for junk/tags on beginning of stream by invalid header */
	if(!firsthead && !head_check(newhead) && !free_format_header(newhead)) {
		int i;

		/* check for id3v2; first three bytes (of 4) are "ID3" */
		if((newhead & (unsigned long) 0xffffff00) == (unsigned long) 0x49443300)
		{
			int id3length = 0;
			id3length = parse_new_id3(newhead, rd);
			goto read_again;
		}
		else if(param.verbose > 1) fprintf(stderr,"Note: Junk at the beginning (0x%08lx)\n",newhead);

		/* I even saw RIFF headers at the beginning of MPEG streams ;( */
		if(newhead == ('R'<<24)+('I'<<16)+('F'<<8)+'F') {
			if(param.verbose > 1) fprintf(stderr, "Note: Looks like a RIFF header.\n");
			if(!rd->head_read(rd,&newhead))
				return 0;
			while(newhead != ('d'<<24)+('a'<<16)+('t'<<8)+'a') {
				if(!rd->head_shift(rd,&newhead))
					return 0;
			}
			if(!rd->head_read(rd,&newhead))
				return 0;
			if(param.verbose > 1) fprintf(stderr,"Note: Skipped RIFF header!\n");
			goto read_again;
		}
		/* unhandled junk... just continue search for a header */
		/* step in byte steps through next 64K */
		for(i=0;i<65536;i++) {
			if(!rd->head_shift(rd,&newhead))
				return 0;
			/* if(head_check(newhead)) */
			if(head_check(newhead) && decode_header(fr, newhead))
			break;
		}
		if(i == 65536) {
			if(!param.quiet) error("Giving up searching valid MPEG header after 64K of junk.");
			return 0;
		}
		/* 
		 * should we additionaly check, whether a new frame starts at
		 * the next expected position? (some kind of read ahead)
		 * We could implement this easily, at least for files.
		 */
	}
#endif

	/* first attempt of read ahead check to find the real first header; cannot believe what junk is out there! */
	/* for now, a spurious first free format header screws up here; need free format support for detecting false free format headers... */
	if(!firsthead && rd->flags & READER_SEEKABLE && head_check(newhead) && decode_header(fr, newhead))
	{
		unsigned long nexthead = 0;
		int hd = 0;
		off_t start = rd->tell(rd);
		debug1("doing ahead check with BPF %d", fr->framesize+4);
		/* step framesize bytes forward and read next possible header*/
		if(rd->back_bytes(rd, -fr->framesize))
		{
			error("cannot seek!");
			return 0;
		}
		hd = rd->head_read(rd,&nexthead);
		if(rd->back_bytes(rd, rd->tell(rd)-start))
		{
			error("cannot seek!");
			return 0;
		}
		if(!hd)
		{
			warning("cannot read next header, a one-frame stream? Duh...");
		}
		else
		{
			debug2("does next header 0x%08lx match first 0x%08lx?", nexthead, newhead);
			/* not allowing free format yet */
			if(!head_check(nexthead) || (nexthead & HDRCMPMASK) != (newhead & HDRCMPMASK))
			{
				debug("No, the header was not valid, start from beginning...");
				/* try next byte for valid header */
				if(rd->back_bytes(rd, 3))
				{
					error("cannot seek!");
					return 0;
				}
				goto read_again;
			}
		}
	}

    /* why has this head check been avoided here before? */
    if(!head_check(newhead))
    {
      if(!firsthead && free_format_header(newhead))
      {
        error1("Header 0x%08lx seems to indicate a free format stream; I do not handle that yet", newhead);
        goto read_again;
        return 0;
      }
    /* and those ugly ID3 tags */
      if((newhead & 0xffffff00) == ('T'<<24)+('A'<<16)+('G'<<8)) {
           rd->skip_bytes(rd,124);
	   if (param.verbose > 1) fprintf(stderr,"Note: Skipped ID3 Tag!\n");
           goto read_again;
      }
      /* duplicated code from above! */
      /* check for id3v2; first three bytes (of 4) are "ID3" */
      if((newhead & (unsigned long) 0xffffff00) == (unsigned long) 0x49443300)
      {
        int id3length = 0;
        id3length = parse_new_id3(newhead, rd);
        goto read_again;
      }
      else if (give_note)
      {
        fprintf(stderr,"Note: Illegal Audio-MPEG-Header 0x%08lx at offset 0x%lx.\n", newhead,rd->tell(rd)-4);
      }

      if(give_note && (newhead & 0xffffff00) == ('b'<<24)+('m'<<16)+('p'<<8)) fprintf(stderr,"Note: Could be a BMP album art.\n");
      if (param.tryresync || do_recover) {
        int try = 0;
        /* TODO: make this more robust, I'd like to cat two mp3 fragments together (in a dirty way) and still have mpg123 beign able to decode all it somehow. */
        if(give_note) fprintf(stderr, "Note: Trying to resync...\n");
            /* Read more bytes until we find something that looks
               reasonably like a valid header.  This is not a
               perfect strategy, but it should get us back on the
               track within a short time (and hopefully without
               too much distortion in the audio output).  */
        do {
          if(!rd->head_shift(rd,&newhead))
		return 0;
          /* debug2("resync try %i, got newhead 0x%08lx", try, newhead); */
          if (!oldhead)
          {
            debug("going to init_resync...");
            goto init_resync;       /* "considered harmful", eh? */
          }
         /* we should perhaps collect a list of valid headers that occured in file... there can be more */
         /* Michael's new resync routine seems to work better with the one frame readahead (and some input buffering?) */
         } while
         (
           ++try < RESYNC_LIMIT
           && (newhead & HDRCMPMASK) != (oldhead & HDRCMPMASK)
           && (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK)
         );
         /* too many false positives 
         }while (!(head_check(newhead) && decode_header(fr, newhead))); */
         if(try == RESYNC_LIMIT)
         {
           error("giving up resync - your stream is not nice... perhaps an improved routine could catch up");
           return 0;
         }

        if (give_note)
          fprintf (stderr, "Note: Skipped %d bytes in input.\n", try);
      }
      else
      {
        error("not attempting to resync...");
        return (0);
      }
    }

    if (!firsthead) {
      if(!decode_header(fr,newhead))
      {
         error("decode header failed before first valid one, going to read again");
         goto read_again;
      }
    }
    else
      if(!decode_header(fr,newhead))
      {
        error("decode header failed - goto resync");
        /* return 0; */
        goto init_resync;
      }
  }
  else
    fr->header_change = 0;
#endif

  bsbufold = bsbuf;
  bsbuf = bsspace[bsnum]+512;
  bsnum = (bsnum + 1) & 1;
	/* if filepos is invalid, so is framepos */
	//framepos = rd->filepos - 4;
  /* read main data into memory */
	/* 0 is error! */
	
	if(!rd->read_frame_body(rd,bsbuf,fr->framesize))
		return 0;
	
#if 0		
	if(!firsthead)
	{
		/* following stuff is actually layer3 specific (in practice, not in theory) */
		if(fr->lay == 3)
		{
			/*
				going to look for Xing or Info at some position after the header
				                                    MPEG 1  MPEG 2/2.5 (LSF)
				Stereo, Joint Stereo, Dual Channel  32      17
				Mono                                17       9
				
				Also, how to avoid false positives? I guess I should interpret more of the header to rule that out(?).
				I hope that ensuring all zeros until tag start is enough.
			*/
			size_t lame_offset = (fr->stereo == 2) ? (fr->lsf ? 17 : 32 ) : (fr->lsf ? 9 : 17);
			if(fr->framesize >= 120+lame_offset) /* traditional Xing header is 120 bytes */
			{
				size_t i;
				int lame_type = 0;
				/* only search for tag when all zero before it (apart from checksum) */
				for(i=2; i < lame_offset; ++i) if(bsbuf[i] != 0) break;
				if(i == lame_offset)
				{
					if
					(
					       (bsbuf[lame_offset] == 'I')
						&& (bsbuf[lame_offset+1] == 'n')
						&& (bsbuf[lame_offset+2] == 'f')
						&& (bsbuf[lame_offset+3] == 'o')
					)
					{
						lame_type = 1; /* We still have to see what there is */
					}
					else if
					(
					       (bsbuf[lame_offset] == 'X')
						&& (bsbuf[lame_offset+1] == 'i')
						&& (bsbuf[lame_offset+2] == 'n')
						&& (bsbuf[lame_offset+3] == 'g')
					)
					{
						lame_type = 2;
						vbr = VBR; /* Xing header means always VBR */
					}
					if(lame_type)
					{
						unsigned long xing_flags;
						
						/* we have one of these headers... */
						if(param.verbose > 1) fprintf(stderr, "Note: Xing/Lame/Info header detected\n");
						/* now interpret the Xing part, I have 120 bytes total for sure */
						/* there are 4 bytes for flags, but only the last byte contains known ones */
						lame_offset += 4; /* now first byte after Xing/Name */
						/* 4 bytes dword for flags */
						#define make_long(a, o) ((((unsigned long) a[o]) << 24) | (((unsigned long) a[o+1]) << 16) | (((unsigned long) a[o+2]) << 8) | ((unsigned long) a[o+3]))
						/* 16 bit */
						#define make_short(a,o) ((((unsigned short) a[o]) << 8) | ((unsigned short) a[o+1]))
						xing_flags = make_long(bsbuf, lame_offset);
						lame_offset += 4;
						debug1("Xing: flags 0x%08lx", xing_flags);
						if(xing_flags & 1) /* frames */
						{
							/*
								In theory, one should use that value for skipping...
								When I know the exact number of samples I could simply count in audio_flush,
								but that's problematic with seeking and such.
								I still miss the real solution for detecting the end.
							*/
							track_frames = make_long(bsbuf, lame_offset);
							if(track_frames > TRACK_MAX_FRAMES) track_frames = 0; /* endless stream? */
							#ifdef GAPLESS
							/* if no further info there, remove/add at least the decoder delay */
							if(param.gapless)
							{
								unsigned long length = track_frames * spf(fr);
								if(length > 1)
								layer3_gapless_init(DECODER_DELAY+GAP_SHIFT, length+DECODER_DELAY+GAP_SHIFT);
							}
							#endif
							debug1("Xing: %lu frames", track_frames);
							lame_offset += 4;
						}
						if(xing_flags & 0x2) /* bytes */
						{
							#ifdef DEBUG
							unsigned long xing_bytes = make_long(bsbuf, lame_offset);
							debug1("Xing: %lu bytes", xing_bytes);
							#endif
							lame_offset += 4;
						}
						if(xing_flags & 0x4) /* TOC */
						{
							lame_offset += 100; /* just skip */
						}
						if(xing_flags & 0x8) /* VBR quality */
						{
							#ifdef DEBUG
							unsigned long xing_quality = make_long(bsbuf, lame_offset);
							debug1("Xing: quality = %lu", xing_quality);
							#endif
							lame_offset += 4;
						}
						/* I guess that either 0 or LAME extra data follows */
						/* there may this crc16 be floating around... (?) */
						if(bsbuf[lame_offset] != 0)
						{
							unsigned char lame_vbr;
							float replay_gain[2] = {0,0};
							float peak = 0;
							float gain_offset = 0; /* going to be +6 for old lame that used 83dB */
							char nb[10];
							memcpy(nb, bsbuf+lame_offset, 9);
							nb[9] = 0;
							debug1("Info: Encoder: %s", nb);
							if(!strncmp("LAME", nb, 4))
							{
								gain_offset = 6;
								debug("TODO: finish lame detetcion...");
							}
							lame_offset += 9;
							/* the 4 big bits are tag revision, the small bits vbr method */
							lame_vbr = bsbuf[lame_offset] & 15;
							debug1("Info: rev %u", bsbuf[lame_offset] >> 4);
							debug1("Info: vbr mode %u", lame_vbr);
							lame_offset += 1;
							switch(lame_vbr)
							{
								/* from rev1 proposal... not sure if all good in practice */
								case 1:
								case 8: vbr = CBR; break;
								case 2:
								case 9: vbr = ABR; break;
								default: vbr = VBR; /* 00==unknown is taken as VBR */
							}
							/* skipping: lowpass filter value */
							lame_offset += 1;
							/* replaygain */
							/* 32bit float: peak amplitude -- why did I parse it as int before??*/
							/* Ah, yes, lame seems to store it as int since some day in 2003; I've only seen zeros anyway until now, bah! */
							if
							(
								   (bsbuf[lame_offset] != 0)
								|| (bsbuf[lame_offset+1] != 0)
								|| (bsbuf[lame_offset+2] != 0)
								|| (bsbuf[lame_offset+3] != 0)
							)
							{
								debug("Wow! Is there _really_ a non-zero peak value? Now is it stored as float or int - how should I know?");
								peak = *(float*) (bsbuf+lame_offset);
							}
							debug1("Info: peak = %f (I won't use this)", peak);
							peak = 0; /* until better times arrived */
							lame_offset += 4;
							/*
								ReplayGain values - lame only writes radio mode gain...
								16bit gain, 3 bits name, 3 bits originator, sign (1=-, 0=+), dB value*10 in 9 bits (fixed point)
								ignore the setting if name or originator == 000!
								radio 0 0 1 0 1 1 1 0 0 1 1 1 1 1 0 1
								audiophile 0 1 0 0 1 0 0 0 0 0 0 1 0 1 0 0
							*/
							
							for(i =0; i < 2; ++i)
							{
								unsigned char origin = (bsbuf[lame_offset] >> 2) & 0x7; /* the 3 bits after that... */
								if(origin != 0)
								{
									unsigned char gt = bsbuf[lame_offset] >> 5; /* only first 3 bits */
									if(gt == 1) gt = 0; /* radio */
									else if(gt == 2) gt = 1; /* audiophile */
									else continue;
									/* get the 9 bits into a number, divide by 10, multiply sign... happy bit banging */
									replay_gain[0] = ((bsbuf[lame_offset] & 0x2) ? -0.1 : 0.1) * (make_short(bsbuf, lame_offset) & 0x1f);
								}
								lame_offset += 2;
							}
							debug1("Info: Radio Gain = %03.1fdB", replay_gain[0]);
							debug1("Info: Audiophile Gain = %03.1fdB", replay_gain[1]);
							for(i=0; i < 2; ++i)
							{
								if(rva_level[i] <= 0)
								{
									rva_peak[i] = 0; /* at some time the parsed peak should be used */
									rva_gain[i] = replay_gain[i];
									rva_level[i] = 0;
								}
							}
							lame_offset += 1; /* skipping encoding flags byte */
							if(vbr == ABR)
							{
								abr_rate = bsbuf[lame_offset];
								debug1("Info: ABR rate = %u", abr_rate);
							}
							lame_offset += 1;
							/* encoder delay and padding, two 12 bit values... lame does write them from int ...*/
							#ifdef GAPLESS
							if(param.gapless)
							{
								/*
									Temporary hack that doesn't work with seeking and also is not waterproof but works most of the time;
									in future the lame delay/padding and frame number info should be passed to layer3.c and the junk samples avoided at the source.
								*/
								unsigned long length = track_frames * spf(fr);
								unsigned long skipbegin = DECODER_DELAY + ((((int) bsbuf[lame_offset]) << 4) | (((int) bsbuf[lame_offset+1]) >> 4));
								unsigned long skipend = -DECODER_DELAY + (((((int) bsbuf[lame_offset+1]) << 8) | (((int) bsbuf[lame_offset+2]))) & 0xfff);
								debug3("preparing gapless mode for layer3: length %lu, skipbegin %lu, skipend %lu", length, skipbegin, skipend);
								if(length > 1)
								layer3_gapless_init(skipbegin+GAP_SHIFT, (skipend < length) ? length-skipend+GAP_SHIFT : length+GAP_SHIFT);
							}
							#endif
						}
						/* switch buffer back ... */
						bsbuf = bsspace[bsnum]+512;
						bsnum = (bsnum + 1) & 1;
						goto read_again;
					}
				}
			}
		} /* end block for Xing/Lame/Info tag */
		firsthead = newhead; /* _now_ it's time to store it... the first real header */
		debug1("firsthead: %08lx", firsthead);
		/* now adjust volume */
		do_rva();
		/* and print id3 info */
		if(!param.quiet) print_id3_tag(rd->flags & READER_ID3TAG ? rd->id3buf : NULL);
	}
#endif
	
  bsi.bitindex = 0;
  bsi.wordpointer = (unsigned char *) bsbuf;
  set_synth_functions(fr);
  if (fr->error_protection) 
    getbits(16); 
  return 1;
}


#if 0

static int stream_back_bytes(struct reader *rds, off_t bytes)
{
  if(stream_lseek(rds,-bytes,SEEK_CUR) < 0)
    return -1;
	/* you sure you want the buffer to resync here? */
  if(param.usebuffer)
	  buffer_resync();
  return 0;
}


/* this function strangely is define to seek num frames _back_ (and is called with -offset - duh!) */
/* also... let that int be a long in future! */
static int stream_back_frame(struct reader *rds,struct frame *fr,long num)
{
	if(rds->flags & READER_SEEKABLE)
	{
		unsigned long newframe, preframe;
		if(num > 0) /* back! */
		{
			if(num > fr->num) newframe = 0;
			else newframe = fr->num-num;
		}
		else newframe = fr->num-num;
		
		/* two leading frames? hm, doesn't seem to be really needed... */
		/*if(newframe > 1) newframe -= 2;
		else newframe = 0;*/
		
		/* now seek to nearest leading index position and read from there until newframe is reached */
		if(stream_lseek(rds,frame_index_find(newframe, &preframe),SEEK_SET) < 0)
		return -1;
		
		debug2("going to %lu; just got %lu", newframe, preframe);
		
		fr->num = preframe;
		
		while(fr->num < newframe)
		{
			/* try to be non-fatal now... frameNum only gets advanced on success anyway */
			if(!read_frame(fr)) break;
		}

		/* this is not needed at last? */
		/*read_frame(fr);
		read_frame(fr);*/

		if(fr->lay == 3) {
			set_pointer(512);
		}

		debug1("arrived at %lu", fr->num);

		if(param.usebuffer)
			buffer_resync();

		return 0;

	}
	else return -1; /* invalid, no seek happened */
}

static int stream_head_read(struct reader *rds,unsigned long *newhead)
{
  unsigned char hbuf[4];

  if(fullread(rds,hbuf,4) != 4)
    return FALSE;
  
  *newhead = ((unsigned long) hbuf[0] << 24) |
    ((unsigned long) hbuf[1] << 16) |
    ((unsigned long) hbuf[2] << 8)  |
    (unsigned long) hbuf[3];
  
  return TRUE;
}

static int stream_head_shift(struct reader *rds,unsigned long *head)
{
  unsigned char hbuf;

  if(fullread(rds,&hbuf,1) != 1)
    return 0;
  *head <<= 8;
  *head |= hbuf;
  *head &= 0xffffffff;
  return 1;
}

static off_t stream_skip_bytes(struct reader *rds,off_t len)
{
  if (rds->filelen >= 0) {
    off_t ret = stream_lseek(rds, len, SEEK_CUR);
    if (param.usebuffer)
      buffer_resync();
    return ret;
  } else if (len >= 0) {
    unsigned char buf[1024]; /* ThOr: Compaq cxx complained and it makes sense to me... or should one do a cast? What for? */
    off_t ret;
    while (len > 0) {
      off_t num = len < sizeof(buf) ? len : sizeof(buf);
      ret = fullread(rds, buf, num);
      if (ret < 0)
	return ret;
      len -= ret;
    }
    return rds->filepos;
  } else
    return -1;
}

static int stream_read_frame_body(struct reader *rds,unsigned char *buf,
				  int size)
{
  long l;

  if( (l=fullread(rds,buf,size)) != size)
  {
    if(l <= 0)
      return 0;
    memset(buf+l,0,size-l);
  }

  return 1;
}

static off_t stream_tell(struct reader *rds)
{
  return rds->filepos;
}

static void stream_rewind(struct reader *rds)
{
  stream_lseek(rds,0,SEEK_SET);
  if(param.usebuffer) 
	  buffer_resync();
}

static off_t get_fileinfo(struct reader *rds,char *buf)
{
	off_t len;

        if((len=lseek(rds->filept,0,SEEK_END)) < 0) {
                return -1;
        }
        if(lseek(rds->filept,-128,SEEK_END) < 0)
                return -1;
        if(fullread(rds,(unsigned char *)buf,128) != 128) {
                return -1;
        }
        if(!strncmp(buf,"TAG",3)) {
                len -= 128;
        }
        if(lseek(rds->filept,0,SEEK_SET) < 0)
                return -1;
        if(len <= 0)
                return -1;
	return len;
}

#endif
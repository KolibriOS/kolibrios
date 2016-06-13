#define _TOKR_

#include "tok.h"

char useasm=FALSE;
char asmparam=FALSE;
#include "asmnemon.h"

void asmtwo1(int basecode); // used for ADD ADC SUB SBB CMP AND OR XOR.
void asmregmem(int out1,int out2); // used for LEA LDS LES LFS LGS LSS.
void Scanbit(int basecode);
void CheckBit(int code);
void asmone1(int basecode); 			// used for INC and DEC.
void asmone2(int basecode); 			// used for NEG NOT MUL IMUL DIV IDIV.
void asmshortjump(int shortcode,int nearcode);
void lar_lsl(int code);
unsigned char tabldeckr(int code);
void protectinstr(int code,int code2);
void doasmmov();		 // do MOV
void asmextend(int basecode); 	// procedure MOVSX and MOVZX
void movd();
void movq();
void packMMX(int code,int code1,int code2);
void asmshift(int basecode);			// used for ROL ROR RCL RCR SHL SAL SHR SAR.
void Shxd(int code);
void FpuType1(unsigned int addrm);
void FpuType2(unsigned int addrm,unsigned int addrm2);
void FpuType3(unsigned int opcode,unsigned int addrm);
void FpuType4(unsigned int opcode,unsigned int addrm);
void FpuType5(unsigned int opcode,unsigned int addrm);
void FpuType6(unsigned int opcode,unsigned int addrm);
void FpuType7(unsigned int addrm);
void FpuType8(unsigned int opcode,unsigned int addrm);
void addlocaljump(int callkind);
unsigned long SaveNumber(int type,int tok4,char *name);
void cmov(int num);
void mmxiii(int type);
void prefetch(int code, int type);
void pextrw();
void pinsrw();
void pshufw();
void xmminstr(int type,int sec,int op1=tk_xmmreg,int op2=tk_xmmreg);
void xmm3instr(int type,int sec);
void xmm2xmm(int code,int code2=0,int type=tk_xmmreg);
void movxmm(int code,int code2,int addc=1);
void movxmm2(int code,int code2=0);
void movxmm3(int code,int code2=0,int type=tk_xmmreg);
void movxmm4(int code,int code2);
void shiftxmm(int rm);	//rxmm,i8
void DDDW(int faradd);
void AADM(int code);

extern void shortjumperror();
extern void invalidfarjumpitem();
extern void invalidfarcallitem();
extern void bytedxexpected();
extern void axalexpected();
extern void clornumberexpected();
extern void reg32regexpected(int type);
extern void begexpected(int type);
extern void regexpected(int type);
extern void invalidoperand(int type);
extern void mmxregexpected(int type=0);
extern void mmxregordwordexpected(int type);
extern void mmxormem(int type);
extern void reg32orword(int type);
extern void xmmregorvarexpected(int type);
extern void xmmregexpected(int type);
extern void fpustakexpected(int type);
extern void fpuvarexpected(int type);

void doasm(int nexta)
{
unsigned char possiblecpu=0,next=1;
int htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
unsigned int i=0;
int razr=r16;
unsigned long hnumber;
int faradd=0;
	if(nexta==FALSE){
		useasm=TRUE;
		nexttok();
		useasm=FALSE;
	}
	if(tok==tk_idasm){
		htok=itok.rm;
		goto sw_asm;
	}
	if(tok==tk_ID||tok==tk_id||tok==tk_undefproc){
		if(tok2==tk_colon){	//метка
			if(tok==tk_undefproc)doanyundefproc();
			else doid((char)(tok==tk_ID?1:0),tk_void);
			return;
		}
		strupr((char *)string);
		htok=FastSearch((unsigned char *)asmMnem,ofsmnem,0,(char *)string);
sw_asm:
		asmparam=TRUE;
		switch(htok){
			case a_add: //ADD
			case a_or:	//OR
			case a_adc: //ADC
			case a_sbb:	//SBB
			case a_and:	//AND
			case a_sub:	//SUB
			case a_xor:	//XOR
			case a_cmp:	//CMP
				asmtwo1(htok*8);
				next=0;
				break;
			case a_not:	//NOT
			case a_neg:	//NEG
			case a_mul:	//MUL
			case a_div:	//DIV
			case a_idiv:	//IDIV
				asmone2((htok-a_not)*8+16);
				break;
			case a_rol:	//ROL
			case a_ror:	//ROR
			case a_rcl:	//RCL
			case a_rcr:	//RCR
			case a_shl:	//SHL
			case a_shr:	//SHR
			case a_sar:	//SAR
				asmshift((htok-a_rol)*8);
				next=0;
				break;
			case a_btc:	//BTC
			case a_btr: //BTR
			case a_bts: //BTS
			case a_bt:	//BT
				CheckBit((htok-a_bt)*8);
				next=0;
				break;
			case a_dec:	//DEC
			case a_inc:	//INC
				asmone1((htok-a_inc)*8);
				break;
			case a_shrd:	//SHRD
			case a_shld:	//SHLD
				Shxd((htok-a_shld)*8+0xa4);
				next=0;
				break;
			case a_aaa:
				ClearReg(AX);
				op(0x37);	//AAA
				break;
			case a_aad:
				ClearReg(AX);
				AADM(0xD5);	//AAD
				break;
			case a_aam:
				ClearReg(AX);
				AADM(0xD4);	//AAM
				break;
			case a_aas:
				ClearReg(AX);
				op(0x3F);	//AAS
				break;
			case a_adrsiz:
				op(0x67);
				possiblecpu=3;
				break;
			case a_arpl:	//ARPL
				nexttok();
				htok=tok;
				hstok=itok;
				hbuf=bufrm;
				bufrm=NULL;
				hstr=strinf;
				strinf.bufstr=NULL;
				nextexpecting2(tk_camma);
				if(tok!=tk_reg)regexpected(2);
				switch(htok){
					case tk_intvar:
					case tk_wordvar:
#ifdef OPTVARCONST
					ClearVarByNum(&hstok);
#endif
						CheckAllMassiv(hbuf,2,&hstr,&hstok);
						KillVar(hstok.name);
						outseg(&hstok,2);
						op(0x63);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
						break;
					case tk_reg:
						ClearReg(hstok.number);
 						op(0x63);
						op(192+(unsigned int)itok.number*8+hstok.number);
						break;
					default: wordvalexpected();
				}
				possiblecpu=2;
				break;
			case a_bound:	//BOUND
				asmregmem(0x62,0);
				possiblecpu=2;
				break;
			case a_lea:
				asmregmem(0x8D,0);	//LEA
				break;
			case a_lds:	//LDS
			case a_les:	//LES
				asmregmem(htok-a_les+0xC4,0);
				break;
			case a_lss:	//LSS
				asmregmem(0xF,0xB2);
				possiblecpu=3;
				break;
			case a_lgs:	//LGS
			case a_lfs:	//LFS
				asmregmem(0xF,htok-a_lfs+0xB4);
				possiblecpu=3;
				break;
			case a_bswap:	//BSWAP
				nexttok();
				ClearReg(itok.number);
				if(tok==tk_reg32||tok==tk_reg){
					op66(tok==tk_reg?r16:r32);
					op(0x0F);
					op(0xC8+(unsigned int)itok.number);
				}
				else preerror("Expecting 32 bit Register for BSWAP");
				possiblecpu=4;
				break;
			case a_bsf:	//BSF
			case a_bsr:	//BSR
				Scanbit(htok-a_bsf+0xbc);
				break;
			case a_call:	//CALL
				nexttok();
				htok=0;
				if(tok==tk_ID||tok==tk_id){
					if(stricmp("FAR",(char *)string)==0){
						nexttok();
						faradd=8;
						htok=1;
					}
					else if(stricmp("NEAR",(char *)string)==0){
						nexttok();
						htok=1;
					}
				}
				else if(tok==tk_far){
					nexttok();
					faradd=8;
					htok=1;
				}
//				printf("call %d\n",tok);
				switch(tok){
					case tk_proc:
						if(faradd==0){
							if(itok.segm==DYNAMIC){
								itok.segm=DYNAMIC_USED;
								updatetree();
							}
							int flag=itok.flag;
							if(itok.segm<NOT_DYNAMIC){	//динамическая процедура
								addacall(itok.number,(unsigned char)(am32!=FALSE?CALL_32:CALL_NEAR));
								callloc0();
							}
							else callloc(itok.number);
						}
						else invalidfarcallitem();
						break;
					case tk_number:
						asmparam=FALSE;
						hnumber=doconstdwordmath();
						if(faradd==0)callloc(hnumber);
						else{
							op(0x9A);
							expecting2(tk_colon);
							unsigned long tempi=doconstdwordmath();
							if(am32==FALSE)outword((unsigned int)tempi);
							else outdword(tempi);
							outword(hnumber);
						}
						next=0;
						break;	 /* CALL num */
					case tk_postnumber:
						if(faradd==0){
							itok.number=itok.number-outptr-3;
							op(0xE8);
							(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
							if(am32==FALSE)outword(itok.number);
							else outdword(itok.number-2);
/*							op(0xE8);
							(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
							if(am32==FALSE)outword((unsigned int)itok.number);
							else outdword(itok.number);*/
						}
						else invalidfarcallitem();
						break;
					case tk_reg32:
						op66(r32);
						goto callreg;
					case tk_reg:
						op66(r16);
callreg:
						if(faradd==0){
							op(0xFF);
							op(0xD0+(unsigned int)itok.number);
						} 	 /* CALL reg */
						else invalidfarcallitem();
						break;
					case tk_ID:
					case tk_id:
						if(faradd==0){
							tobedefined(am32==FALSE?CALL_NEAR:CALL_32,tk_void);
							callloc0();
						} 		 /* CALL NEAR */
						else invalidfarcallitem();
						break;
					case tk_declare:
						tok=tk_undefproc;
						updatetree();
					case tk_locallabel:
					case tk_undefproc:
						if(faradd==0){
							addacall((unsigned int)itok.number,
(unsigned char)((itok.flag&f_extern)!=0?CALL_EXT:(am32==FALSE?CALL_NEAR:CALL_32)));
							callloc0();
						} 			/* CALL NEAR */
						else invalidfarcallitem();
						break;
					case tk_dwordvar:
					case tk_longvar:
						i=2;
						if(am32==FALSE&&htok==0)faradd=8;
					case tk_intvar:
					case tk_wordvar:
						i+=2;
						CheckAllMassiv(bufrm,i,&strinf);
						outseg(&itok,2);
						op(0xFF); op(0x10+itok.rm+faradd);
						outaddress(&itok);
						break;
					default:
						preerror("Invalid item for CALL");
						break;
				}
#ifdef OPTVARCONST
				FreeGlobalConst();
#endif
				clearregstat();
				break;
			case a_cbw:
				ClearReg(AX);
				if(am32)op(0x66);
				op(0x98);	//CBW
				break;
			case a_cdq:
				ClearReg(DX);
				op66(r32);
				op(0x99);
				possiblecpu=3;	//CDQ
				break;
			case a_clc:
				op(0xF8);	//CLC
				break;
			case a_cld:
				op(0xFC);	//CLD
				break;
			case a_cli:
				op(0xFA);	//CLI
				break;
			case a_clts:	//CLTS
				outword(0x060F);
				possiblecpu=2;
				break;
			case a_cmc:
				op(0xF5);	//CMC
				break;
			case a_cmpsb:
				ClearReg(SI);
				ClearReg(DI);
				op(0xA6);
				break;
			case a_cmpsw:
				ClearReg(SI);
				ClearReg(DI);
				op66(r16);
				op(0xA7);
				break;
			case a_cmpsd:
				if(ScanTok3()==tk_camma&&tok2==tk_xmmreg){
					xmm3instr(0xC2,0xF2);
					possiblecpu=9;
					next=0;
				}
				else{
					ClearReg(SI);
					ClearReg(DI);
					op66(r32);
					op(0xA7);
					possiblecpu=3;
				}
				break;
			case a_cmpxchg:	//CMPXCHG
				nexttok();
				htok=tok;
				hstok=itok;
				hbuf=bufrm;
				bufrm=NULL;
				hstr=strinf;
				strinf.bufstr=NULL;
				nextexpecting2(tk_camma);
				i=r16;
				ClearReg(AX);
				switch(htok){
					case tk_reg32:
						i=r32;
					case tk_reg:
						ClearReg(hstok.number);
						switch(tok){
							case tk_reg32:
								if(i==r16)goto erreg;
							case tk_reg:
	 							op66(i);
								outword(0xB10F);
								op(128+64+(unsigned int)itok.number*8+hstok.number);
								break;
							case tk_longvar:
							case tk_dwordvar:
								if(i==r16)goto erreg;
							case tk_intvar:
							case tk_wordvar:
#ifdef OPTVARCONST
								ClearVarByNum(&itok);
#endif
								CheckAllMassiv(bufrm,i,&strinf);
	 							op66(i);
								outseg(&itok,3);
								outword(0xB10F);
								op(itok.rm+hstok.number*8);
								outaddress(&itok);
								break;
							default:
erreg:
								if(i==r16)wordvalexpected();
								else dwordvalexpected();
								break;
						}
						break;
					case tk_beg:
						ClearReg(hstok.number);
						switch(tok){
							case tk_beg:
								outword(0xB00F);
								op(128+64+(unsigned int)itok.number*8+hstok.number);
								break;
							case tk_charvar:
							case tk_bytevar:
								CheckAllMassiv(bufrm,1,&strinf);
								outseg(&itok,3);
								outword(0xB00F);
								op(itok.rm+hstok.number*8);
								outaddress(&itok);
								break;
							default: bytevalexpected(2);
								break;
						}
						break;
					case tk_charvar:
					case tk_bytevar:
#ifdef OPTVARCONST
						ClearVarByNum(&hstok);
#endif
						if(tok!=tk_beg)begexpected(2);
						CheckAllMassiv(hbuf,1,&hstr,&hstok);
						KillVar(hstok.name);
						outseg(&hstok,3);
						outword(0xB00F);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
						break;
					case tk_longvar:
					case tk_dwordvar:
						if(tok!=tk_reg32)reg32expected(2);
						i=r32;
						goto noint;
					case tk_intvar:
					case tk_wordvar:
						if(tok!=tk_reg)regexpected(2);
noint:
#ifdef OPTVARCONST
						ClearVarByNum(&hstok);
#endif
						CheckAllMassiv(hbuf,i,&hstr,&hstok);
						KillVar(hstok.name);
						op66(i);
						outseg(&hstok,3);
						outword(0xB10F);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
						break;
					default:
						varexpected(1);
						break;
				}
				possiblecpu=4;
				break;
			case a_cmpxchg8b:	//CMPXCHG8B
				nexttok();
				if(tok!=tk_qwordvar)qwordvalexpected();
#ifdef OPTVARCONST
				ClearVarByNum(&itok);
#endif
				CheckAllMassiv(bufrm,8,&strinf);
				KillVar(itok.name);
				outseg(&itok,3);
				outword(0xc70f);
				op(itok.rm+8);
				outaddress(&itok);
				possiblecpu=5;
				break;
			case a_cwd:
				ClearReg(DX);
				op66(r16);
				op(0x99);	//CWD
				break;
			case a_cwde:
				ClearReg(AX);
				if(!am32)op(0x66);
				op(0x98);
				possiblecpu=3;
				break;
			case a_cpuid:
				ClearReg(AX);
				ClearReg(BX);
				ClearReg(CX);
				ClearReg(DX);
				outword(0xa20f);	//CPUID
				possiblecpu=5;
				break;
			case a_daa:
				ClearReg(AX);
				op(0x27);	//DAA
				break;
			case a_das:
				ClearReg(AX);
				op(0x2F);	//DAS
				break;
			case a_db:	//DB
				dbgact++;
				asmparam=FALSE;
				do{
					i=1;
					nexttok();
					CheckIP();
					if(tok==tk_number){
						hnumber=doconstdwordmath();
						if(tok==tk_id&&strcmp((char *)string,"dup")==0){
							i=hnumber;
							nexttok();
							CheckMinusNum();
							if(tok==tk_number)hnumber=doconstdwordmath();
							else numexpected();
						}
						for(;i!=0;i--)op(hnumber);
					}
					else if(tok==tk_string){
						for(i=0;i<(unsigned int)itok.number;i++)opd(string[i]);
						switch(itok.flag&3){
							case zero_term:
								if(itok.flag&s_unicod)opd(0);
								opd(0);
								break;
							case dos_term:
								if(itok.flag&s_unicod)opd(0);
								opd('$');
								break;
						}
						nexttok();
					}
					else{
						numexpected();
						nexttok();
					}
				}while(tok==tk_camma);
				dbgact--;
				next=0;
				break;
			case a_dd:	//DD
				faradd++;
			case a_dw:	//DW
				DDDW(faradd);
				next=0;
				break;
			case a_enter:	//ENTER
				ClearReg(BP);
				op(0xC8);
				nexttok();
				if(tok==tk_number)outword((unsigned int)doconstlongmath());
				else{
					numexpected(1);
					nexttok();
				}
				expecting2(tk_camma);
				if(tok==tk_number)op((int)doconstlongmath());
				else{
					numexpected(2);
					nexttok();
				}
				next=0;
				possiblecpu=2;
				break;
			case a_emms:	//EMMS
				outword(0x770f);
				possiblecpu=6;
				break;
			case a_imul:	//IMUL
				nexttok();
				if(tok2!=tk_camma){
				ClearReg(AX);
					switch(tok){
						case tk_reg32:
							possiblecpu=3;
							razr=r32;
						case tk_reg:
							ClearReg(DX);
							op66(razr);
						  op(246+1);
							op(128+64+40+(unsigned int)itok.number);
							break;
						case tk_beg:
							op(246);
							op(128+64+40+(unsigned int)itok.number);
							break;
						case tk_charvar:
						case tk_bytevar:
							CheckAllMassiv(bufrm,1,&strinf);
							outseg(&itok,2);
							op(246);
							op(itok.rm+40);
							outaddress(&itok);
							break;
						case tk_longvar:
						case tk_dwordvar:
							CheckAllMassiv(bufrm,4,&strinf);
							possiblecpu=3;
							razr=r32;
						case tk_intvar:
						case tk_wordvar:
							CheckAllMassiv(bufrm,2,&strinf);
							ClearReg(DX);
							op66(razr);
							outseg(&itok,2);
							op(247);
							op(itok.rm+40);
							outaddress(&itok);
							break;
						default: varexpected(1);	break;
					}
				}
				else{
					htok=tok;
					hnumber=itok.number;
					ClearReg(hnumber);
					nextexpecting2(tk_camma);
					possiblecpu=2;
					if(tok2!=tk_camma){
						switch(tok){
							case tk_number:
								switch(htok){
									case tk_reg32:
										possiblecpu=3;
										razr=r32;
									case tk_reg:
										op66(razr);
										if(short_ok(itok.number,TRUE))i=2;	//короткая форма
										op(0x69+i);
										op(0xc0+(unsigned int)hnumber+(unsigned int)hnumber*8);
										if(i==2)op(itok.number);
										else{
											if(htok==tk_reg)outword(itok.number);
											else outdword(itok.number);
										}
										break;
									default: reg32regexpected(1);
								}
								break;
							case tk_reg32:
								razr=r32;
							case tk_reg:
								op66(razr);
								possiblecpu=3;
								if(htok!=tok)reg32regexpected(1);
								outword(0xaf0f);
								op(0xc0+(unsigned int)itok.number+(unsigned int)hnumber*8);
								break;
							case tk_longvar:
								tok++;
							case tk_dwordvar:
								if(htok!=tk_reg32)reg32expected(1);
								CheckAllMassiv(bufrm,4,&strinf);
								razr=r32;
								goto imul1;
							case tk_intvar:
								tok++;
							case tk_wordvar:
imul1:
								possiblecpu=3;
								if(htok!=tk_reg&&(tok==tk_intvar||tok==tk_wordvar))regexpected(1);
								CheckAllMassiv(bufrm,2,&strinf);
								op66(razr);
								outseg(&itok,3);
								outword(0xaf0f);
								op(itok.rm+hnumber*8);
								outaddress(&itok);
								break;
							default: varexpected(2);
						}
					}
					else{
						int htok2=tok;
						hstok=itok;
						hbuf=bufrm;
						hstr=strinf;
						strinf.bufstr=NULL;
						bufrm=NULL;
						nextexpecting2(tk_camma);
						if(tok!=tk_number)numexpected(3);
						switch(htok2){
							case tk_reg32:
								possiblecpu=3;
								razr=r32;
							case tk_reg:
								op66(razr);
								if(htok!=htok2)reg32regexpected(2);
								if(short_ok(itok.number,TRUE))i=2;	//короткая форма
								op(0x69+i);
								op(0xc0+(unsigned int)hstok.number+(unsigned int)hnumber*8);
								if(i==2)op(itok.number);
								else{
									if(htok==tk_reg)outword(itok.number);
									else outdword(itok.number);
								}
								break;
							case tk_longvar:
								tok++;
							case tk_dwordvar:
								possiblecpu=3;
								if(htok!=tk_reg32)reg32expected(2);
								CheckAllMassiv(hbuf,4,&hstr,&hstok);
								razr=r32;
								goto imul2;
							case tk_intvar:
								tok++;
							case tk_wordvar:
imul2:
								if(htok!=tk_reg&&(htok2==tk_intvar||htok2==tk_wordvar))regexpected(1);
								CheckAllMassiv(hbuf,2,&hstr,&hstok);
								op66(razr);
								if(short_ok(itok.number,TRUE))i=2;	//короткая форма
								outseg(&hstok,2);
								op(0x69+i);
								op(hstok.rm+hnumber*8);
								outaddress(&hstok);
								if(i==2)op(itok.number);
								else{
									if(htok==tk_reg)outword(itok.number);
									else outdword(itok.number);
								}
								break;
							default: varexpected(2);
						}
					}
				}
				break;
			case a_in:	//IN
				ClearReg(AX);
				nexttok();
				if((tok==tk_reg||tok==tk_reg32)&&(unsigned int)itok.number==AX){
					op66(tok==tk_reg?r16:r32);
					if(tok==tk_reg32)possiblecpu=3;
					nexttok();
					expecting(tk_camma);
					if(tok==tk_number){
						op(0xE5);
						asmparam=FALSE;
						if((hnumber=doconstdwordmath())>255)bytedxexpected();
						op(hnumber);
						next = 0;
					}
					else if(tok==tk_reg&&(unsigned int)itok.number==DX)op(0xED);
					else bytedxexpected();
				}
				else if(tok==tk_beg&&(unsigned int)itok.number==AL){
					nexttok();
					expecting(tk_camma);
					if(tok==tk_number){
						op(0xE4);
						asmparam=FALSE;
						if((hnumber=doconstdwordmath())>255)bytedxexpected();
						op(hnumber);
						next=0;
					}
					else if(tok==tk_reg&&(unsigned int)itok.number==DX)op(0xEC);
					else bytedxexpected();
				}
				else axalexpected();
				break;
			case a_insb:	//INSB
				ClearReg(AX);
				ClearReg(DI);
				op(0x6C);
				possiblecpu=2;
				break;
			case a_insw:	//INSW
				op66(r16);
				possiblecpu=2;
				goto insd;
				break;
			case a_insd:	//INSD
				op66(r32);
				possiblecpu=3;
insd:
				op(0x6D);
				ClearReg(AX);
				ClearReg(DI);
				break;
			case a_int:	//INT
				clearregstat();
				nexttok();
				if(tok==tk_number){
					asmparam=FALSE;
					htok=doconstlongmath();
					if(htok==3)op(0xCC);
					else{
						op(0xCD);
						op(htok);
					}
					next=0;
				}
				else numexpected();
				break;
			case a_into:
				op(0xCE);	//INTO
				break;
			case a_invd:	//INVD
				outword(0x080F);
				possiblecpu=4;
				break;
			case a_invlpg:	//INVLPG
				nexttok();
				switch(tok){
					case tk_longvar:
					case tk_dwordvar:
						CheckAllMassiv(bufrm,4,&strinf);
						goto invlgp;
					case tk_intvar:
					case tk_wordvar:
						CheckAllMassiv(bufrm,2,&strinf);
invlgp:
						outseg(&itok,3);
						outword(0x010F);
						op(itok.rm+0x38);
						outaddress(&itok);
						break;

					case tk_undefofs:
						strcpy(hstok.name,itok.name);
						tok=tk_number;
					case tk_number:
						hnumber=doconstdwordmath();
						outword(0x010f);
						op((am32==FALSE?rm_d16:rm_d32)+0x38);
		 				if(postnumflag&f_reloc)AddReloc();
					 	if(htok==tk_undefofs)AddUndefOff(2,hstok.name);
						if(am32)outdword(hnumber);
						else outword(hnumber);
						next=0;
						break;
					case tk_postnumber:
						outword(0x010F);
						op((am32==FALSE?rm_d16:rm_d32)+0x38);
						(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
						if(am32==FALSE)outword(itok.number);	//было 0
						outdword(itok.number);
						break;

					default:
						varexpected(0); break;
				}
				possiblecpu=4;
				break;
			case a_iret:
#ifdef OPTVARCONST
				ClearLVIC();
#endif
				RestoreStack();
				clearregstat();
				op66(r16);
				op(0xCF);	//IRET
				break;
			case a_iretd:
#ifdef OPTVARCONST
				ClearLVIC();
#endif
				RestoreStack();
				clearregstat();
				op66(r32);
				op(0xCF);	//IRETD
				break;
			case a_jo:
			case a_jno:
			case a_jc:
			case a_jnc:
			case a_jz:
			case a_jnz:
			case a_jna:
			case a_ja:
			case a_js:
			case a_jns:
			case a_jp:
			case a_jnp:
			case a_jl:
			case a_jnl:
			case a_jng:
			case a_jg:
				RestoreStack();
				asmshortjump((htok-a_jo)+0x70,(htok-a_jo)+0x80);
				next=0;
				break;
			case a_jecxz:	//JECXZ
				RestoreStack();
				op67(r32);
				asmshortjump(0xE3,0);
				next=0;
				break;
			case a_jcxz:	//JCXZ
				RestoreStack();
				op67(r16);
				asmshortjump(0xE3,0);
				next=0;
				break;
			case a_jmp:	//JMP
				RestoreStack();
				nexttok();
				faradd=0;
				lastcommand=tk_goto;
				if(stricmp("FAR",itok.name)==0){
					nexttok();
					faradd=8;
				}
				else if(stricmp("NEAR",itok.name)==0)nexttok();
				else if(stricmp("SHORT",itok.name)==0){	 // case insensitive
					nexttok();
					next=(unsigned char)GOTO();
					break;
				}
				next=gotol(faradd);
				break;
			case a_lahf:
				ClearReg(AX);
				op(0x9F);	//LAHF
				break;
			case a_lar:	//LAR
			case a_lsl:	//LSL
				lar_lsl(htok-a_lar+2);
				break;
			case a_leave:	//LEAVE
				RestoreStack();
				op(0xC9);
				ClearReg(BP);
				possiblecpu = 2;
				break;
			case a_lmsw:
				protectinstr(1,0x30);	//LMSW
				break;
			case a_loadall:	// LOADALL 80286 only
				outword(0x050F);
				possiblecpu=2;
				break;
			case a_lodsb:
				ClearReg(SI);
				ClearReg(AX);
				op(0xAC);	//LODSB
				break;
			case a_lodsw:
				op66(r16);
				ClearReg(SI);
				ClearReg(AX);
				op(0xAD);	//LODSW
				break;
			case a_lodsd:
				op66(r32);
				ClearReg(SI);
				ClearReg(AX);
				op(0xAD);
				possiblecpu =3;	//LODSD
				break;
			case a_lock:
				op(0xF0);	//LOCK
				break;
			case a_loop:
				ConstToReg(0,CX,(am32+1)*2);
				op67(r16);
				asmshortjump(0xE2,0);
				next=0;
				break;
			case a_loopd:
				ConstToReg(0,CX,r32);
				possiblecpu=3;
				op67(r32);
				asmshortjump(0xE2,0);
				next=0;
				break;
			case a_loopz:	//LOOPE LOOPZ
				ClearReg(CX);
				asmshortjump(0xE1,0);
				next=0;
				break;
			case a_loopnz:	//LOOPNZ LOOPNE
				ClearReg(CX);
				asmshortjump(0xE0,0);
				next=0;
				break;
			case a_ltr:
				protectinstr(0,0x18);	//LTR
				break;
			case a_str:
				protectinstr(0,0x08);	//STR
				break;
			case a_lgdt:
				next=tabldeckr(0x10);
				break;
			case a_lidt:	//LIDT
				next=tabldeckr(0x18);
				break;
			case a_lldt:
				protectinstr(0,0x10);	//LLDT
				break;
			case a_mov:
				doasmmov();
				next=0;
				break;
			case a_movsb:
				movsb();	//MOVSB
				break;
			case a_movsd:
				if(ScanTok3()==tk_camma&&(tok2==tk_xmmreg||(tok2>=tk_charvar&&tok2<tk_qwordvar))){
					movxmm(0x10,0xf2);
					possiblecpu=9;
				}
				else movsd();	//MOVSD
				break;
			case a_movsw:
				movsw();	//MOVSW
				break;
			case a_movzx:
			case a_movsx:
				asmextend((htok-a_movzx)*8+0xB6);
				break;
			case a_movd:
				movd();	//MOVD
				break;
			case a_movq:
				movq();	//MOVQ
				break;
			case a_nop:
				op(0x90);	//NOP
				break;
			case a_opsiz:	//OPSIZ OPSIZE
				op(0x66);
				possiblecpu=3;
				break;
			case a_out:	//OUT
				nexttok();
				if(tok==tk_number){
					if((hnumber=doconstdwordmath())>255)bytedxexpected();
					expecting2(tk_camma);
					if((tok==tk_reg||tok==tk_reg32)&&(unsigned int)itok.number==AX){
						op66(tok==tk_reg?r16:r32);
						if(tok==tk_reg32)possiblecpu=3;
						op(0xE7);	op(hnumber);
					}
					else if(tok==tk_beg&&(unsigned int)itok.number==AL){
						op(0xE6);	op(hnumber);
					}
					else axalexpected();
				}
				else if(tok==tk_reg&&(unsigned int)itok.number==DX){
					nextexpecting2(tk_camma);
					if((tok==tk_reg||tok==tk_reg32)&&(unsigned int)itok.number==AX){
						op66(tok==tk_reg?r16:r32);
						if(tok==tk_reg32)possiblecpu=3;
						op(0xEF);
					}
					else if(tok==tk_beg&&(unsigned int)itok.number==AL)op(0xEE);
					else axalexpected();
				}
				else bytedxexpected();
				break;
			case a_outsb:	//OUTSB
				ClearReg(SI);
				op(0x6E);
				possiblecpu=2;
				break;
			case a_outsw:	//OUTSW
				ClearReg(SI);
				op66(r16);
				op(0x6F);
				possiblecpu=2;
				break;
			case a_outsd:	//OUTSD
				ClearReg(SI);
				op66(r32);
				op(0x6F);
				possiblecpu=3;
				break;
			case a_pop:	//POP
				RestoreStack();
				do{
					possiblecpu=0;
					asmparam=TRUE;
					nexttok();
					razr=r16;
					switch(tok){
						case tk_reg32:
							possiblecpu=3;
							razr=r32;
						case tk_reg:
							ClearReg(itok.number);
							op66(razr);
						  op(0x58+(unsigned int)itok.number); break;
						case tk_dwordvar:
							tok--;
						case tk_longvar:
							CheckAllMassiv(bufrm,4,&strinf);
							possiblecpu=3;
							razr=r32;
							goto pop;
						case tk_wordvar:
							tok--;
						case tk_intvar:
							CheckAllMassiv(bufrm,2,&strinf);
pop:
#ifdef OPTVARCONST
							ClearVarByNum(&itok);
#endif
							KillVar(itok.name);
							op66(razr);
							outseg(&itok,2);
							op(0x8F);	op(itok.rm);
							outaddress(&itok);
							break;
						case tk_seg:
							if(itok.number!=CS){
								PopSeg((unsigned int)itok.number);
								break;
							}
						default:
							preerror("Invalid operand for POP");
							break;
					}
					if(cpu<possiblecpu)cpu=possiblecpu;
					asmparam=FALSE;
					addESP-=razr==r16?2:4;
					nexttok();
				}while(tok==tk_camma);
				next=0;
				break;
			case a_popa:	//POPA
				razr=r16;
				possiblecpu=2;
				goto POPAD;
			case a_popad:	//POPAD
				razr=r32;
				possiblecpu=3;
POPAD:
				op66(razr);
				addESP-=razr==r16?16:32;
				RestoreStack();
				clearregstat();
				op(0x61);
				break;
			case a_popf:
				razr=r16;
				goto POPFD;
			case a_popfd:	//POPFD
				razr=r32;
				possiblecpu = 3;
POPFD:
				op66(razr);
				addESP-=razr==r16?2:4;
				RestoreStack();
				op(0x9D);
				break;
			case a_push:	//PUSH
				RestoreStack();
				do{
					asmparam=TRUE;
					nexttok();
					if((razr=Push())==FALSE)preerror("Invalid operand for PUSH");
					asmparam=FALSE;
					addESP+=razr==r16?2:4;
				}while(tok==tk_camma);
				next=0;
				break;
			case a_pusha:	//PUSHA
				razr=r16;
				possiblecpu=2;
				goto PUSHAD;
			case a_pushad:	//PUSHAD
				razr=r32;
				possiblecpu=3;
PUSHAD:
				op66(razr);
				addESP+=razr==r16?16:32;
				RestoreStack();
				op(0x60);
				break;
			case a_pushf:
				razr=r16;
				goto PUSHFD;
			case a_pushfd:	//PUSHFD
				razr=r32;
				possiblecpu=3;
PUSHFD:
				op66(razr);
				RestoreStack();
				op(0x9C);
				addESP+=razr==r16?2:4;
				break;
			case a_pcmpeqd:
			case a_pcmpeqw:
			case a_pcmpeqb:
			case a_packssdw:  //+xmm
			case a_punpckhdq:
			case a_punpckhwd:
			case a_punpckhbw:
			case a_packuswb:    //+xmm
			case a_pcmpgtd:
			case a_pcmpgtw:
			case a_pcmpgtb:
			case a_packsswb:	//+xmm
			case a_punpckldq:
			case a_punpcklwd:
			case a_punpcklbw:
				mmxiii(htok-a_punpcklbw+0x60);
				break;
			case a_pmullw:
				mmxiii(0xD5);	//PMULLW
				break;
			case a_pmuludq:
				mmxiii(0xF4);	//PMULUDQ
				break;
			case a_psubusb:
				mmxiii(0xD8);	//PSUBUSB
				break;
			case a_psubusw:
				mmxiii(0xD9);//PSUBUSW
				break;
			case a_pand:    //+xmm
				mmxiii(0xDB);	//PAND
				break;
			case a_paddusb:	//+xmm
				mmxiii(0xDC);	//PADDUSB
				break;
			case a_paddusw: //+xmm
				mmxiii(0xDD);//PADDUSW
				break;
			case a_pandn:   //+xmm
				mmxiii(0xDF);//PANDN
				break;
			case a_pause:
				outword(0x90F3);
				possiblecpu=9;
				break;
			case a_pmulhw:
				mmxiii(0xE5);	//PMULHW
				break;
			case a_psubsb:
				mmxiii(0xE8);	//PSUBSB
				break;
			case a_psubsw:
				mmxiii(0xE9);//PSUBSW
				break;
			case a_por:
				mmxiii(0xEB);	//POR
				break;
			case a_paddsb:	//+xmm
				mmxiii(0xEC);	//PADDSB
				break;
			case a_paddsw:  //+xmm
				mmxiii(0xED);//PADDSW
				break;
			case a_pxor:
				mmxiii(0xEF);	//PXOR
				break;
			case a_pmaddwd:
				mmxiii(0xF5);	//PMADDWD
				break;
			case a_psubb:
				mmxiii(0xF8);	//PSUBB
				break;
			case a_psubw:
				mmxiii(0xF9);//PSUBW
				break;
			case a_psubd:
				mmxiii(0xFA);//PSUBD
				break;
			case a_psubq:
				mmxiii(0xFB);//PSUBQ
				possiblecpu=9;
				break;
			case a_paddb:   //+xmm
				mmxiii(0xFC);	//PADDB
				break;
			case a_paddw:   //+xmm
				mmxiii(0xFD);//PADDW
				break;
			case a_paddd:   //+xmm
				mmxiii(0xFE);//PADDD
				break;
			case a_paddq:   //+xmm
				mmxiii(0xD4);//PADDQ
				possiblecpu=9;
				break;

			case a_psrlw:
				packMMX(0xD1,0x71,0xd0);	//PSRLW
				next=0;
				break;
			case a_psrld:
				packMMX(0xD2,0x72,0xd0);//PSRLD
				next=0;
				break;
			case a_psrlq:
				packMMX(0xD3,0x73,0xd0);//PSRLQ
				next=0;
				break;
			case a_psraw:
				packMMX(0xE1,0x71,0xe0);	//PSRAW
				next=0;
				break;
			case a_psrad:
				packMMX(0xE2,0x72,0xe0);//PSRAD
				next=0;
				break;
			case a_psllw:
				packMMX(0xF1,0x71,0xf0);	//PSLLW
				next=0;
				break;
			case a_pslld:
				packMMX(0xF2,0x72,0xf0);//PSLLD
				next=0;
				break;
			case a_psllq:
				packMMX(0xF3,0x73,0xf0);//PSLLQ
				next=0;
				break;
			case a_pslldq:
				shiftxmm(7);
				next=0;
				possiblecpu=9;
				break;
			case a_psrldq:
				shiftxmm(3);
				next=0;
				possiblecpu=9;
				break;
			case a_rdmsr:
				ClearReg(AX);
				ClearReg(DX);
				outword(0X320F);
				possiblecpu=5;
				break;
			case a_rdtsc:
				outword(0X310F);
				possiblecpu=5;
				break;
			case a_rep:
	      op(0xF3);//REP REPE REPZ
				ClearReg(CX);
				break;
			case a_repnz:
	      op(0xF2);	//REPNE REPNZ
				ClearReg(CX);
				break;
			case a_ret:	//RET
#ifdef OPTVARCONST
				ClearLVIC();
#endif
				RestoreStack();
				clearregstat();
				next=0;
				if(tok2==tk_number){
//					usedirectiv=TRUE;
					nexttok();
					op(0xC2);
				 	asmparam=FALSE;
					outword((unsigned int)doconstlongmath());
//					usedirectiv=FALSE;
				}
				else{
					op(0xC3);
					nexttok();
				}
				break;
			case a_retf:
#ifdef OPTVARCONST
				ClearLVIC();
#endif
				RestoreStack();
				clearregstat();
				next=0;
				if(tok2==tk_number){
					nexttok();
					op(0xCA);
				 	asmparam=FALSE;
					outword((unsigned int)doconstlongmath());
				}
				else{
					op(0xCB);
					nexttok();
				}//RETF
				break;
			case a_rsm:	//RSM
				outword(0Xaa0F);
				possiblecpu=5;
				break;
			case a_sahf:
				op(0x9E);	//SAHF
				break;
			case a_scasb:
				op(0xAE);	//SCASB
				ClearReg(DI);
				break;
			case a_scasw:
				op66(r16);
				op(0xAF);  //SCASW
				ClearReg(DI);
				break;
			case a_scasd:	//SCASD
				op66(r32);
				op(0xAF);
				possiblecpu=3;
				ClearReg(DI);
				break;
			case a_smsw:
				protectinstr(1,0x20);	//SMSW
				break;
			case a_stc:
				op(0xF9);	//STC
				break;
			case a_std:
				op(0xFD);	//STD
				break;
			case a_sti:
				op(0xFB);	//STI
				break;
			case a_stosb:
				stosb();	//STOSB
				break;
			case a_stosw:
				stosw();	//STOSW
				break;
			case a_stosd:
				stosd();	//STOSD
				break;
			case a_seto:
			case a_setno:
			case a_setc:
			case a_setnc:
			case a_setz:
			case a_setnz:
			case a_setna:
			case a_seta:
			case a_sets:
			case a_setns:
			case a_setp:
			case a_setnp:
			case a_setl:
			case a_setnl:
			case a_setng:
			case a_setg:
				i=htok-a_seto;
				nexttok();
				switch(tok){
					case tk_beg:
						ClearReg(itok.number%4);
						op(0xf);
						op(0x90+i);
						op(0xc0+(unsigned int)itok.number);
						break;
					case tk_charvar:
					case tk_bytevar:
						CheckAllMassiv(bufrm,1,&strinf);
						KillVar(itok.name);
						outseg(&itok,3);
						op(0xf);
						op(0x90+i);
						op(itok.rm);
						outaddress(&itok);
						break;
					default: bytevalexpected(0);
				}
				possiblecpu=3;
				break;
			case a_sgdt:
				next=tabldeckr(0);
				break;
			case a_sidt:
				next=tabldeckr(0x8);
				break;
			case a_sldt:
				protectinstr(0,0);	//SLDT
				break;
			case a_fwait:	//FWAIT
			case a_wait:
				fwait();//WAIT
				break;
			case a_wbinvd:	//WBINVD
				outword(0x090F);
				possiblecpu=4;
				break;
			case a_wrmsr:	//WRMSR
				outword(0x300F);
				possiblecpu=5;
				break;
			case a_xadd:	//XADD
				nexttok();
				hstok=itok;
				htok=tok;
				hbuf=bufrm;
				bufrm=NULL;
				hstr=strinf;
				strinf.bufstr=NULL;
				nextexpecting2(tk_camma);
				switch(htok){
					case tk_reg32:
						razr=r32;
					case tk_reg:
						switch(tok){
							case tk_reg32:
								if(razr==r16)goto erxreg;
							case tk_reg:
								RegToReg(itok.number,hstok.number,razr);
								ClearReg(hstok.number);
								op66(razr);
								outword(0xC10F);
								op(128+64+(unsigned int)itok.number*8+hstok.number);
								break;
							case tk_longvar:
							case tk_dwordvar:
								if(razr==r16)goto erxreg;
							case tk_intvar:
							case tk_wordvar:
								KillVar(hstok.name);
								/*if(bufrm==0&&strinf.bufstr==NULL)*/AddRegVar(hstok.number,razr,&itok);
								CheckAllMassiv(bufrm,razr,&strinf);
								op66(razr);
								outseg(&itok,3);
								outword(0xC10F);
								op(itok.rm+hstok.number*8);
								outaddress(&itok);
								break;
							default:
erxreg:
								wordvalexpected(); break;
						}
						break;
					case tk_beg:
						switch(tok){
							case tk_beg:
								RegToReg(itok.number,hstok.number,r8);
								ClearReg(hstok.number%4);
								outword(0xC00F);
								op(128+64+(unsigned int)itok.number*8+hstok.number);
								break;
							case tk_charvar:
							case tk_bytevar:
								KillVar(hstok.name);
								/*if(bufrm==0&&strinf.bufstr==NULL)*/AddRegVar(hstok.number,r8,&itok);
								CheckAllMassiv(bufrm,1,&strinf);
								outseg(&itok,3);
								outword(0xC00F);
								op(itok.rm+hstok.number*8);
								outaddress(&itok);
								break;
							default: bytevalexpected(2); break;
						}
						break;
					case tk_charvar:
					case tk_bytevar:
						if(tok!=tk_beg)begexpected(2);
						CheckAllMassiv(hbuf,1,&hstr,&hstok);
						ClearReg(itok.number);
						KillVar(hstok.name);
						outseg(&hstok,3);
						outword(0xC00F);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
						break;
					case tk_longvar:
					case tk_dwordvar:
						if(tok!=tk_reg32)reg32expected(2);
						razr=r32;
						goto nointxadd;
					case tk_intvar:
					case tk_wordvar:
						if(tok!=tk_reg)regexpected(1);
nointxadd:
						CheckAllMassiv(hbuf,razr,&hstr,&hstok);
						ClearReg(itok.number);
						KillVar(hstok.name);
						op66(razr);
						outseg(&hstok,3);
						outword(0xC10F);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
						break;
					default: varexpected(1); break;
				}
				possiblecpu=4;
				break;
			case a_xchg:	//XCHG
				nexttok();
				htok=tok;
				hstok=itok;
				hbuf=bufrm;
				bufrm=NULL;
				hstr=strinf;
				strinf.bufstr=NULL;
				nextexpecting2(tk_camma);
				switch(htok){
					case tk_reg32:
						razr=r32;
						possiblecpu=3;
					case tk_reg:
						switch(tok){
							case tk_reg32:
								if(razr==r16)goto erregx;
							case tk_reg:
								op66(razr);
								RegSwapReg(hstok.number,itok.number,razr);
								if(hstok.number==AX)op(0x90+(unsigned int)itok.number);
								else if((unsigned int)itok.number==AX)op(0x90+hstok.number);
								else{
									op(0x87);
									op(128+64+(unsigned int)itok.number*8+hstok.number);
								}
								break;
							case tk_longvar:
							case tk_dwordvar:
								if(razr==r16)goto erregx;
							case tk_intvar:
							case tk_wordvar:
								CheckAllMassiv(bufrm,razr,&strinf);
								KillVar(itok.name);
								IDZToReg(itok.name,hstok.number,razr);
								op66(razr);
								outseg(&itok,2);
								op(0x87);
								op(itok.rm+hstok.number*8);
								outaddress(&itok);
								break;
							default:
erregx:
								wordvalexpected(); break;
						}
						break;
					case tk_beg:
						switch(tok){
							case tk_beg:
								RegSwapReg(hstok.number,itok.number,r8);
								op(0x86);
								op(128+64+(unsigned int)itok.number*8+hstok.number);
								break;
							case tk_charvar:
							case tk_bytevar:
								CheckAllMassiv(bufrm,1,&strinf);
								KillVar(itok.name);
								IDZToReg(itok.name,hstok.number,r8);
								outseg(&itok,2);
								op(0x86);
								op(itok.rm+hstok.number*8);
								outaddress(&itok);
								break;
							default: bytevalexpected(2); break;
						}
						break;
					case tk_charvar:
					case tk_bytevar:
						if(tok!=tk_beg)begexpected(2);
						CheckAllMassiv(hbuf,1,&hstr,&hstok);
						KillVar(hstok.name);
						IDZToReg(hstok.name,itok.number,r8);
						outseg(&hstok,2);
						op(0x86);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
						break;
					case tk_longvar:
					case tk_dwordvar:
						razr=r32;
						possiblecpu=3;
						if(tok!=tk_reg32)reg32expected(2);
						goto nointx;
					case tk_intvar:
					case tk_wordvar:
						if(tok!=tk_reg)regexpected(2);
nointx:
		 				CheckAllMassiv(hbuf,razr,&hstr,&hstok);
						KillVar(hstok.name);
						IDZToReg(hstok.name,itok.number,razr);
						op66(razr);
						outseg(&hstok,2);
						op(0x87);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
						break;
					default: varexpected(1); break;
				}
				break;
			case a_xlat:
				ClearReg(AX);
				op(0xD7);	//XLAT
				break;
			case a_hlt:
	      op(0xF4);	//HLT
				break;
			case a_verr:
				protectinstr(0,0x20);	//VERR
				break;
			case a_verw:
				protectinstr(0,0x28);	//VERW
				break;
			case a_test:	//TEST
				if(iTest()==FALSE)invalidoperand(0);
				next=0;
				break;
			case a_fcom:	//FCOM
				FpuType1(0x10);
				break;
			case a_fcomp:	//FCOMP
				FpuType1(0x18);
				break;
			case a_fadd:	//FADD
				FpuType2(0,0);
				break;
			case a_fdiv:	//FDIV
				FpuType2(0x38,0x30);
				break;
			case a_fdivr:	//FDIVR
				FpuType2(0x30,0x38);
				break;
			case a_fmul:	//FMUL
				FpuType2(0x8,0x8);
				break;
			case a_fsub:	//FSUB
				FpuType2(0x28,0x20);
				break;
			case a_fsubr:	//FSUBR
				FpuType2(0x20,0x28);
				break;
			case a_faddp:	//FADDP
				FpuType3(0xDE,0);
				break;
			case a_fdivp:	//FDIVP
				FpuType3(0xDE,0x38);
				break;
			case a_fdivrp:	//FDIVRP
				FpuType3(0xDE,0x30);
				break;
			case a_ffree:	//FFREE
				FpuType3(0xDD,0);
				break;
			case a_fmulp:	//FMULP
				FpuType3(0xDE,8);
				break;
			case a_fsubp:	//FSUBP
				FpuType3(0xDE,0x28);
				break;
			case a_fsubrp:	//FSUBRP
				FpuType3(0xDE,0x20);
				break;
			case a_fucom:	//FUCOM
				FpuType3(0xDD,0x20);
				break;
			case a_fucomp:	//FUCOMP
				FpuType3(0xDD,0x28);
				break;
			case a_fxch:	//FXCH
				FpuType3(0xD9,8);
				break;

			case a_fiadd:	//FIADD
				FpuType4(0,0);
				break;
			case a_ficom:	//FICOM
				FpuType4(0,0x10);
				break;
			case a_ficomp:	//FICOMP
				FpuType4(0,0x18);
				break;
			case a_fidiv:	//FIDIV
				FpuType4(0,0x30);
				break;
			case a_fidivr:	//FIDIVR
				FpuType4(0,0x38);
				break;
			case a_fild:	//FILD
				FpuType4(1,0);
				break;
			case a_fimul:	//FIMUL
				FpuType4(0,8);
				break;
			case a_fist:	//FIST
				FpuType4(1,0x10);
				break;
			case a_fistp:	//FISTP
				FpuType4(1,0x18);
				break;
			case a_fisub:	//FISUB
				FpuType4(0,0x20);
				break;
			case a_fisubr:	//FISUBR
				FpuType4(0,0x28);
				break;

			case a_fld:	//FLD
				FpuType5(0xD9,0);
				break;
			case a_fst:	//FST
				FpuType5(0xDD,0x10);
				break;
			case a_fstp:	//FSTP
				FpuType5(0xDD,0x18);
				break;

			case a_fbld:	//FBLD
				FpuType6(0xDF,0X20);
				break;
			case a_fbstp:	//FBSTP
				FpuType6(0xDF,0X30);
				break;
			case a_fildq:	//FILDQ
				FpuType6(0xDF,0x28);
				break;
			case a_fldenv:	//FLDENV БЕЗ РАЗМЕРНОСТИ
				FpuType6(0xD9,0x20);
				break;
			case a_frstor:	//FRSTOR БЕЗ РАЗМЕРНОСТИ
				FpuType6(0xDD,0x20);
				break;
			case a_fsave:	//FSAVE БЕЗ РАЗМЕРНОСТИ
				fwait();
			case a_fnsave:	//FNSAVE БЕЗ РАЗМЕРНОСТИ
				FpuType6(0xDD,0x30);
				break;
			case a_fstenv:	//FSTENV БЕЗ РАЗМЕРНОСТИ
				fwait();
			case a_fnstenv:	//FNSTENV БЕЗ РАЗМЕРНОСТИ
				FpuType6(0xD9,0x30);
				break;

			case a_fldcw:	//FLDCW
				FpuType7(0x28);
				break;
			case a_fstcw:	//FSTCW
				fwait();
			case a_fnstcw:	//FNSTCW
				FpuType7(0x38);
				break;

			case a_f2xm1:
				outword(0xf0d9);	//F2XM1
				possiblecpu=2;
				break;
			case a_fabs:
				outword(0xE1D9);	//FABS
				break;
			case a_fchs:
				outword(0xE0D9);	//FCHS
				break;
			case a_fclex:	//FCLEX
				fwait();
//				outword(0xE2DB);
//				break;
			case a_fnclex:
				outword(0xE2DB);	//FNCLEX
				break;
			case a_fcompp:
				outword(0xD9DE);	//FCOMPP
				break;
			case a_fcos:	//FCOS
				outword(0xFFD9);
				possiblecpu=3;
				break;
			case a_fdecstr:
				outword(0xF6D9);	//FDECSTP
				break;
			case a_fdisi:
				fwait();
			case a_fndisi:
				outword(0xE1DB);	//FDISI
				break;
			case a_feni:
				fwait();
			case a_fneni:
				outword(0xE0DB);	//FENI
				break;
			case a_fincstr:
				outword(0xF7D9);	//FINCSTP
				break;
			case a_finit:     	//FINIT
				fwait();
			case a_fninit:
				outword(0xE3DB);	//FNINIT
				break;
			case a_fldlg2:
				outword(0xECD9);	//FLDLG2
				break;
			case a_fldln2:
				outword(0xEDD9);	//FLDLN2
				break;
			case a_fldl2e:
				outword(0xEAD9);	//FLDL2E
				break;
			case a_fldl2t:
				outword(0xE9D9);	//FLDL2T
				break;
			case a_fldpi:
				outword(0xEBD9);	//FLDPI
				break;
			case a_fldz:
				outword(0xEED9);	//FLDZ
				break;
			case a_fld1:
				outword(0xE8D9);	//FLD1
				break;
			case a_fnop:
				outword(0xD0D9);	//FNOP
				break;
			case a_fpatan:
				outword(0xF3D9);	//FPATAN
				break;
			case a_fprem:
				outword(0xF8D9);	//FPREM
				break;
			case a_fprem1:
				outword(0xF5D9);	//FPREM1
				possiblecpu=3;
				break;
			case a_fptan:
				outword(0xF2D9);	//FPTAN
				break;
			case a_frndint:
				outword(0xFCD9);	//FRNDINT
				break;
			case a_fsetpm:
				fwait();
			case a_fnsetpm:
				outword(0xE4DB);	//FSETPM
				possiblecpu=2;
				break;
			case a_fscale:
				outword(0XFDD9);	//FSCALE
				break;
			case a_fsin:	//FSIN
				outword(0xFED9);
				possiblecpu=3;
				break;
			case a_fsincos:	//FSINCOS
				outword(0xFBD9);
				possiblecpu=3;
				break;
			case a_fsqrt:
				outword(0xFAD9);	//FSQRT
				break;
			case a_fstsw:	//FSTSW
				fwait();
			case a_fnstsw:	//FNSTSW
				nexttok();
				switch(tok){
					case tk_wordvar:
					case tk_intvar:
						CheckAllMassiv(bufrm,2,&strinf);
						KillVar(itok.name);
						outseg(&itok,2);
						op(0xDD);
						op(itok.rm+0x38);
						outaddress(&itok);
						break;
					case tk_reg:
						if(itok.number==0)outword(0xe0df);
						else preerror("Use only AX");
						ClearReg(AX);
						break;
					default: wordvalexpected();
				}
				break;
			case a_ftst:
				outword(0xE4D9);	//FTST
				break;
			case a_fucompp:	//FUCOMPP
				outword(0xE9DA);
				possiblecpu=3;
				break;
			case a_fxam:
				outword(0xE5D9);	//FXAM
				break;
			case a_fxtract:
				outword(0xF4D9);	//FXTRACT
				break;
			case a_fyl2x:
				outword(0xF1D9);	//FYL2X
				break;
			case a_fyl2xp1:
				outword(0xF9D9);	//FYL2XP1
				break;
			case a_ud2:
				outword(0x0B0F);	//UD2
				possiblecpu=7;
				break;
			case a_sysenter:
				outword(0x340F);	//SYSENTER
				possiblecpu=7;
				break;
			case a_sysexit:
				outword(0x350F);	//SYSEXIT
				possiblecpu=7;
				break;
			case a_rdpmc:
				outword(0x330F);	//RDPMC
				possiblecpu=7;
				break;
			case a_fcmovnu:
				i+=8;
			case a_fcmovnbe:
				i+=8;
			case a_fcmovne:
				i+=8;
			case a_fcmovnb:
				FpuType8(0xDB,i);
				break;
			case a_fcmovu:
				i+=8;
			case a_fcmovbe:
				i+=8;
			case a_fcmove:
				i+=8;
			case a_fcmovb:
				FpuType8(0xDA,i);
				break;
			case a_fcomi:	//FCOMI
				i+=8;
			case a_fucomi:
				FpuType8(0xDB,i+0x28);
				break;
			case a_fcomip:
				i+=8;
			case a_fucomip:
				FpuType8(0xDF,i+0x28);
				break;
			case a_cmovo:
			case a_cmovno:
			case a_cmovc:
			case a_cmovnc:
			case a_cmovz:
			case a_cmovnz:
			case a_cmovna:
			case a_cmova:
			case a_cmovs:
			case a_cmovns:
			case a_cmovp:
			case a_cmovnp:
			case a_cmovl:
			case a_cmovnl:
			case a_cmovng:
			case a_cmovg:
				cmov(htok-a_cmovo);
				possiblecpu=7;
				break;
			case a_lfence:	//LFENCE
				outword(0xAE0F);
				op(0xE8);
				possiblecpu=9;
				break;
			case a_mfence:	//MFENCE
				outword(0xAE0F);
				op(0xF0);
				possiblecpu=9;
				break;
			case a_sfence:	//SFENCE
				outword(0xAE0F);
				op(0xF8);
				possiblecpu=8;
				break;
			case a_maskmovq:	//MASKMOVQ
				nexttok();
				hnumber=itok.number;
				if(tok!=tk_mmxreg)mmxregexpected(1);
				nextexpecting2(tk_camma);
				if(tok!=tk_mmxreg)mmxregexpected(2);
				outword(0xF70F);
				op(0xC0+hnumber+itok.number*8);
				possiblecpu=8;
				break;
			case a_movntq:	//MOVNTQ
				movxmm3(0xE7,0,tk_mmxreg);
				possiblecpu=8;
				break;
			case a_pavgb:
				mmxiii(0xE0);
				break;
			case a_pavgw:
				mmxiii(0xE3);
				break;
			case a_pmaxub:
				mmxiii(0xDE);
				break;
			case a_pmaxsw:
				mmxiii(0xEE);
				break;
			case a_pminub:
				mmxiii(0xDA);
				break;
			case a_pminsw:
				mmxiii(0xEA);
				break;
			case a_pmulhuw:
				mmxiii(0xE4);
				break;
			case a_psadbw:
				mmxiii(0xF6);
				break;
			case a_prefetcht0:
				prefetch(0x18,1);
				possiblecpu=8;
				break;
			case a_prefetcht1:
				prefetch(0x18,2);
				possiblecpu=8;
				break;
			case a_prefetcht2:
				prefetch(0x18,3);
				possiblecpu=8;
				break;
			case a_prefetchnta:
				prefetch(0x18,0);
				possiblecpu=8;
				break;
			case a_pextrw:
				pextrw();
				next=0;
				break;
			case a_pinsrw:
				pinsrw();
				next=0;
				break;
			case a_pmovmskb:
				nexttok();
				if(tok!=tk_reg32)reg32expected(1);
				htok=itok.number;
				ClearReg(htok);
				nextexpecting2(tk_camma);
				if(tok==tk_xmmreg){
					op(0x66);
					possiblecpu=9;
				}
				else{
					possiblecpu=8;
					if(tok!=tk_mmxreg)mmxregexpected(2);
				}
				outword(0xD70F);
				op(rm_mod11+htok*8+itok.number);
				break;
			case a_pshufw:
				pshufw();
				next=0;
				possiblecpu=8;
				break;
			case a_pshufd:
				xmm3instr(0x70,0x66);
				possiblecpu=9;
				next=0;
				break;
			case a_pshufhw:
				xmm3instr(0x70,0xf3);
				possiblecpu=9;
				next=0;
				break;
			case a_pshuflw:
				xmm3instr(0x70,0xf2);
				possiblecpu=9;
				next=0;
				break;
			case a_addpd:
				xmminstr(0x58,0x66);
				possiblecpu=9;
				break;
			case a_addps:
				xmminstr(0x58,0);
				possiblecpu=8;
				break;
			case a_addsd:
				xmminstr(0x58,0xF2);
				possiblecpu=9;
				break;
			case a_addss:
				xmminstr(0x58,0xF3);
				possiblecpu=8;
				break;
			case a_addsubpd:
				xmminstr(0xD0,0x66);
				possiblecpu=9;
				break;
			case a_addsubps:
				xmminstr(0xD0,0xF2);
				possiblecpu=9;
				break;
			case a_andnpd:
				xmminstr(0x55,0x66);
				possiblecpu=9;
				break;
			case a_andnps:
				xmminstr(0x55,0);
				possiblecpu=8;
				break;
			case a_andpd:
				xmminstr(0x54,0x66);
				possiblecpu=9;
				break;
			case a_andps:
				xmminstr(0x54,0);
				possiblecpu=8;
				break;
			case a_comisd:
				xmminstr(0x2F,0x66);
				possiblecpu=9;
				break;
			case a_comiss:
				xmminstr(0x2F,0);
				possiblecpu=8;
				break;
			case a_divps:
				xmminstr(0x5E,0);
				possiblecpu=8;
				break;
			case a_divsd:
				xmminstr(0x5E,0xf2);
				possiblecpu=9;
				break;
			case a_divss:
				xmminstr(0x5E,0xF3);
				possiblecpu=8;
				break;
			case a_haddpd:
				xmminstr(0x7C,0x66);
				possiblecpu=9;
				break;
			case a_haddps:
				xmminstr(0x7C,0xF2);
				possiblecpu=9;
				break;
			case a_hsubpd:
				xmminstr(0x7D,0x66);
				possiblecpu=9;
				break;
			case a_hsubps:
				xmminstr(0x7D,0xF2);
				possiblecpu=9;
				break;
			case a_maskmovdqu:
				xmm2xmm(0xf7,0x66);
				possiblecpu=9;
				break;
			case a_maxpd:
				xmminstr(0x5f,0x66);
				possiblecpu=9;
				break;
			case a_maxps:
				xmminstr(0x5F,0);
				possiblecpu=8;
				break;
			case a_maxsd:
				xmminstr(0x5f,0xf2);
				possiblecpu=9;
				break;
			case a_maxss:
				xmminstr(0x5F,0xF3);
				possiblecpu=8;
				break;
			case a_minpd:
				xmminstr(0x5d,0x66);
				possiblecpu=9;
				break;
			case a_minps:
				xmminstr(0x5D,0);
				possiblecpu=8;
				break;
			case a_minsd:
				xmminstr(0x5d,0xf2);
				possiblecpu=9;
				break;
			case a_minss:
				xmminstr(0x5D,0xF3);
				possiblecpu=8;
				break;
			case a_mulpd:
				xmminstr(0x59,0x66);
				possiblecpu=9;
				break;
			case a_mulps:
				xmminstr(0x59,0);
				possiblecpu=8;
				break;
			case a_mulsd:
				xmminstr(0x59,0xF2);
				possiblecpu=9;
				break;
			case a_mulss:
				xmminstr(0x59,0xF3);
				possiblecpu=8;
				break;
			case a_orpd:
				xmminstr(0x56,0x66);
				possiblecpu=9;
				break;
			case a_orps:
				xmminstr(0x56,0);
				possiblecpu=8;
				break;
			case a_rcpps:
				xmminstr(0x53,0);
				possiblecpu=8;
				break;
			case a_rcpss:
				xmminstr(0x53,0xF3);
				possiblecpu=8;
				break;
			case a_rsqrtps:
				xmminstr(0x52,0);
				possiblecpu=8;
				break;
			case a_rsqrtss:
				xmminstr(0x52,0xF3);
				possiblecpu=8;
				break;
			case a_sqrtpd:
				xmminstr(0x51,0x66);
				possiblecpu=9;
				break;
			case a_sqrtps:
				xmminstr(0x51,0);
				possiblecpu=8;
				break;
			case a_sqrtsd:
				xmminstr(0x51,0xf2);
				possiblecpu=9;
				break;
			case a_sqrtss:
				xmminstr(0x51,0xF3);
				possiblecpu=8;
				break;
			case a_subpd:
				xmminstr(0x5C,0x66);
				possiblecpu=9;
				break;
			case a_subps:
				xmminstr(0x5C,0);
				possiblecpu=8;
				break;
			case a_subsd:
				xmminstr(0x5C,0xF2);
				possiblecpu=9;
				break;
			case a_subss:
				xmminstr(0x5C,0xF3);
				possiblecpu=8;
				break;
			case a_ucomisd:
				xmminstr(0x2E,0x66);
				possiblecpu=9;
				break;
			case a_ucomiss:
				xmminstr(0x2E,0);
				possiblecpu=8;
				break;
			case a_unpckhpd:
				xmminstr(0x15,0x66);
				possiblecpu=9;
				break;
			case a_unpckhps:
				xmminstr(0x15,0);
				possiblecpu=8;
				break;
			case a_unpcklpd:
				xmminstr(0x14,0x66);
				possiblecpu=9;
				break;
			case a_unpcklps:
				xmminstr(0x14,0);
				possiblecpu=8;
				break;
			case a_xorpd:
				xmminstr(0x57,0x66);
				possiblecpu=9;
				break;
			case a_xorps:
				xmminstr(0x57,0);
				possiblecpu=8;
				break;
			case a_cmppd:
				xmm3instr(0xC2,0x66);
				possiblecpu=9;
				next=0;
				break;
			case a_cmpeqpd:
			case a_cmpltpd:
			case a_cmplepd:
			case a_cmpunordpd:
			case a_cmpneqpd:
			case a_cmpnltpd:
			case a_cmpnlepd:
			case a_cmpordpd:
				xmminstr(0xC2,0x66);
				possiblecpu=9;
				op(htok-a_cmpeqpd);
				break;
			case a_cmpps:
				xmm3instr(0xC2,0);
				possiblecpu=8;
				next=0;
				break;
			case a_cmpeqps:
			case a_cmpltps:
			case a_cmpleps:
			case a_cmpunordps:
			case a_cmpneqps:
			case a_cmpnltps:
			case a_cmpnleps:
			case a_cmpordps:
				xmminstr(0xC2,0);
				possiblecpu=9;
				op(htok-a_cmpeqps);
				break;
			case a_cmpss:
				xmm3instr(0xC2,0xF3);
				possiblecpu=8;
				next=0;
				break;
			case a_cmpeqss:
			case a_cmpltss:
			case a_cmpless:
			case a_cmpunordss:
			case a_cmpneqss:
			case a_cmpnltss:
			case a_cmpnless:
			case a_cmpordss:
				xmminstr(0xC2,0xF3);
				possiblecpu=9;
				op(htok-a_cmpeqss);
				break;
			case a_cmpeqsd:
			case a_cmpltsd:
			case a_cmplesd:
			case a_cmpunordsd:
			case a_cmpneqsd:
			case a_cmpnltsd:
			case a_cmpnlesd:
			case a_cmpordsd:
				xmminstr(0xC2,0xF2);
				op(htok-a_cmpeqsd);
				possiblecpu=9;
				break;
			case a_shufpd:
				xmm3instr(0xC6,0x66);
				possiblecpu=9;
				next=0;
				break;
			case a_shufps:
				xmm3instr(0xC6,0);
				possiblecpu=8;
				next=0;
				break;
			case a_cvtdq2pd:
				xmminstr(0xE6,0xF3);
				possiblecpu=9;
				break;
			case a_cvtdq2ps:
				xmminstr(0x5B,0);
				possiblecpu=9;
				break;
			case a_cvtpd2dq:
				xmminstr(0xE6,0xF2);
				possiblecpu=9;
				break;
			case a_cvtpd2pi:
				xmminstr(0x2d,0x66,tk_mmxreg,tk_xmmreg);
				possiblecpu=9;
				break;
			case a_cvtpd2ps:
				xmminstr(0x5a,0x66);
				possiblecpu=9;
				break;
			case a_cvtpi2pd:
				xmminstr(0x2a,0x66,tk_xmmreg,tk_mmxreg);
				possiblecpu=9;
				break;
			case a_cvtpi2ps:
				xmminstr(0x2a,0,tk_xmmreg,tk_mmxreg);
				possiblecpu=8;
				break;
			case a_cvtps2dq:
				xmminstr(0x5b,0x66);
				possiblecpu=9;
				break;
			case a_cvtps2pd:
				xmminstr(0x5a,0);
				possiblecpu=9;
				break;
			case a_cvtsi2ss:
				xmminstr(0x2a,0xf3,tk_xmmreg,tk_reg32);
				possiblecpu=8;
				break;
			case a_cvtps2pi:
				xmminstr(0x2d,0,tk_mmxreg,tk_xmmreg);
				possiblecpu=9;
				break;
			case a_cvtsd2si:
				xmminstr(0x2d,0xf2,tk_reg32,tk_xmmreg);
				possiblecpu=9;
				break;
			case a_cvtsd2ss:
				xmminstr(0x5a,0xf2);
				possiblecpu=9;
				break;
			case a_cvtsi2sd:
				xmminstr(0x2a,0xf2,tk_xmmreg,tk_reg32);
				possiblecpu=9;
				break;
			case a_cvtss2sd:
				xmminstr(0x5a,0xf3);
				possiblecpu=9;
				break;
			case a_cvtss2si:
				xmminstr(0x2d,0xf3,tk_reg32,tk_xmmreg);
				possiblecpu=8;
				break;
			case a_cvttpd2pi:
				xmminstr(0x2c,0x66,tk_mmxreg,tk_xmmreg);
				possiblecpu=9;
				break;
			case a_cvttpd2dq:
				xmminstr(0xE6,0x66);
				possiblecpu=9;
				break;
			case a_cvttps2dq:
				xmminstr(0x5B,0xF3);
				possiblecpu=9;
				break;
			case a_cvttps2pi:
				xmminstr(0x2c,0,tk_mmxreg,tk_xmmreg);
				possiblecpu=8;
				break;
			case a_cvttsd2si:
				xmminstr(0x2C,0xF2,tk_reg32,tk_xmmreg);
				possiblecpu=9;
				break;
			case a_cvttss2si:
				xmminstr(0x2C,0xF3,tk_reg32,tk_xmmreg);
				possiblecpu=8;
				break;
			case a_divpd:
				xmminstr(0x5E,0x66);
				possiblecpu=9;
				break;
			case a_punpckhqdq:
				xmminstr(0x6D,0x66);
				possiblecpu=9;
				break;
			case a_punpcklqdq:
				xmminstr(0x6C,0x66);
				possiblecpu=9;
				break;
			case a_fxrstor:
				prefetch(0xAE,1);
				possiblecpu=8;
				break;
			case a_fxsave:
				prefetch(0xAE,0);
				possiblecpu=8;
				break;
			case a_ldmxcsr:
				prefetch(0xAE,2);
				possiblecpu=8;
				break;
			case a_stmxcsr:
				prefetch(0xAE,3);
				possiblecpu=8;
				break;
			case a_clflush:
				prefetch(0xAE,7);
				possiblecpu=9;
				break;
			case a_monitor:
				outword(0x010F);
				op(0xc8);
				possiblecpu=9;
				break;
			case a_mwait:
				outword(0x010F);
				op(0xc9);
				possiblecpu=9;
				break;
			case a_lddqu:
				movxmm4(0xF0,0xF2);
				possiblecpu=9;
				break;
			case a_movhlps:
				xmm2xmm(0x12);
				possiblecpu=8;
				break;
			case a_movlhps:
				xmm2xmm(0x16);
				possiblecpu=8;
				break;
			case a_movmskps:
				xmm2xmm(0x50,0,tk_reg32);
				possiblecpu=8;
				break;
			case a_movntdq:
				movxmm3(0xE7,0x66);
				possiblecpu=9;
				break;
			case a_movntpd:
				movxmm3(0x2B,0x66);
				possiblecpu=9;
				break;
			case a_movntps:
				movxmm3(0x2b,0);
				possiblecpu=8;
				break;
			case a_movapd:
				movxmm(0x28,0x66);
				possiblecpu=9;
				break;
			case a_movaps:
				movxmm(0x28,0);
				possiblecpu=8;
				break;
			case a_movdqa:
				movxmm(0x6f,0x66,0x10);
				possiblecpu=9;
				break;
			case a_movddup:
				xmminstr(0x12,0xF2);
				possiblecpu=9;
				break;
			case a_movshdup:
				xmminstr(0x16,0xF3);
				possiblecpu=9;
				break;
			case a_movsldup:
				xmminstr(0x12,0xF3);
				possiblecpu=9;
				break;
			case a_movdqu:
				movxmm(0x6f,0xf3,0x10);
				possiblecpu=9;
				break;
			case a_movdq2q:
				xmm2xmm(0xd6,0xf2,tk_mmxreg);
				possiblecpu=9;
				break;
			case a_movhpd:
				movxmm2(0x16,0x66);
				possiblecpu=9;
				break;
			case a_movlpd:
				movxmm2(0x12,0x66);
				possiblecpu=9;
				break;
			case a_movmskpd:
				xmm2xmm(0x50,0x66,tk_reg32);
				possiblecpu=9;
				break;
			case a_movnti:	//MOVNTI
				movxmm3(0xC3,0,tk_reg32);
				possiblecpu=9;
				break;
			case a_movq2dq:	//MOVQ2DQ
				nexttok();
				hnumber=itok.number;
				if(tok!=tk_xmmreg)xmmregexpected(1);
				nextexpecting2(tk_camma);
				if(tok!=tk_mmxreg)mmxregexpected(2);
				op(0xF3);
				outword(0xD60F);
				op(0xC0+hnumber+itok.number*8);
				possiblecpu=9;
				break;
			case a_movupd:
				movxmm(0x10,0x66);
				possiblecpu=9;
				break;
			case a_movups:
				movxmm(0x10,0);
				possiblecpu=8;
				break;
			case a_movss:
				movxmm(0x10,0xF3);
				possiblecpu=8;
				break;
			case a_movhps:
				movxmm2(0x16);
				possiblecpu=8;
				break;
			case a_movlps:
				movxmm2(0x12);
				possiblecpu=8;
				break;

			case -1: codeexpected(); break;
			default:
				preerror("sorry, this instruction is not supported");
				break;
		}
		asmparam=FALSE;
		if(cpu<possiblecpu)cpu=possiblecpu;
		if(next)nexttok();
	}
	else if(tok==tk_seg){
		switch((unsigned int)itok.number){
			case ES: op(0x26); break;
			case SS: op(0x36); break;
			case CS: op(0x2E); break;
			case DS: op(0x3E); break;
			case FS: op(0x64);
				if(cpu<3)cpu=3;
				break;
			case GS: op(0x65);
				if(cpu<3)cpu=3;
				break;
			default: beep(); break;
		}
		nextexpecting2(tk_colon);
	}
	else if(tok==tk_locallabel)define_locallabel();
	else if(tok==tk_at&&ScanTok3()==tk_colon){
		nexttok();
		LLabel();
	}
	else codeexpected();
	if(tok==tk_semicolon)nexttok();
}

void cmov(int num)
{
int type=r32;
int reg;
	nexttok();
	if(tok==tk_reg)type=r16;
	else if(tok!=tk_reg32)reg32regexpected(1);
	reg=itok.number;
	ClearReg(reg);
	nextexpecting2(tk_camma);
	op66(type);
	switch(tok){
		case tk_reg:
			if(type==r32)reg32expected(2);
			goto regs;
		case tk_reg32:
			if(type==r16)regexpected(2);
regs:
			op(0x0F);
			op(0x40+num);
			op(0xC0+reg*8+itok.number);
			break;
		case tk_wordvar:
		case tk_intvar:
			if(type==r32)dwordvalexpected();
			CheckAllMassiv(bufrm,2,&strinf);
			goto dwords;
		case tk_dwordvar:
		case tk_longvar:
			if(type==r16)wordvalexpected();
			CheckAllMassiv(bufrm,4,&strinf);
dwords:
			outseg(&itok,3);
			op(0x0F);
			op(0x40+num);
			op(itok.rm+reg*8);
			outaddress(&itok);
			break;
		default:
			varexpected(2);
			break;
	}
}

#ifdef OPTVARCONST
int GetOperand(int code)
{
	switch(code){
		case a_inc:	//INC
		case a_add: return tk_plus;
		case a_or: return tk_or;
		case a_and:	return tk_and;
		case a_dec:	//DEC
		case a_sub:	return tk_minus;
		case a_xor:	return tk_xor;
		case a_not:	return tk_not;
		case a_neg:	return tk_numsign;
		case a_shl:	return tk_ll;
		case a_shr:	return tk_rr;
	}
	return tokens;
}
#endif

void  asmtwo1(int basecode) // used for ADD ADC SUB SBB CMP AND OR XOR.
{
unsigned long holdnumber2;
int htok,typet=r16;
long longholdnumber;
char *hbuf;
ITOK hstok;
unsigned char next=1;
SINFO hstr;
#ifdef OPTVARCONST
int initconst=FALSE;
int operand=GetOperand(basecode/8);
#endif
	nexttok();
	hstok=itok;
	htok=tok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
int i=tok;
	switch(htok){
		case tk_reg:
			if((basecode/8)!=a_cmp)ClearReg(hstok.number);
		  switch(tok){
				case tk_undefofs:
					strcpy(hstok.name,itok.name);
					tok=tk_number;
					goto wnum;
				case tk_minus:
					if(tok2!=tk_number){
						wordvalexpected();
						break;
					}
				case tk_number:
wnum:
					asmparam=FALSE;
					holdnumber2=doconstlongmath();
					next=0;
					op66(r16);
					if(hstok.number==AX){
						op(4+1+basecode);
						if((postnumflag&f_reloc)!=0)AddReloc();
						if(i==tk_undefofs)AddUndefOff(2,hstok.name);
						outword(holdnumber2);
					}
					else{
						if((postnumflag&f_reloc)==0&&short_ok(holdnumber2)){
							op(128+2+1);
							op(128+64+hstok.number+basecode);
							op(holdnumber2);
						}
						else{
							op(128+1);
							op(128+64+hstok.number+basecode);
							if((postnumflag&f_reloc)!=0)AddReloc();
							if(i==tk_undefofs)AddUndefOff(2,hstok.name);
							outword(holdnumber2);
						}
					}
					break;
				case tk_postnumber:
					op66(r16);
					if(hstok.number==AX)op(4+1+basecode);
					else{
						op(128+1);
						op(128+64+hstok.number+basecode);
					}
					(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
					outword((unsigned int)itok.number);
					break;
				case tk_reg:
					op66(r16);
					op(2+1+basecode);
					op(128+64+(unsigned int)itok.number+hstok.number*8);
					break;
				case tk_wordvar:
				case tk_intvar:
					CheckAllMassiv(bufrm,2,&strinf);
					op66(r16);
					outseg(&itok,2);
					op(2+1+basecode);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					break;
				default: wordvalexpected(); break;
			}
			break;
		case tk_reg32:
			if((basecode/8)!=a_cmp)ClearReg(hstok.number);
			switch(tok){
				case tk_undefofs:
					strcpy(hstok.name,itok.name);
					tok=tk_number;
					goto dnum;
				case tk_minus:
					i=tk_number;
					if(tok2!=tk_number){
						dwordvalexpected();
						break;
					}
				case tk_number:
dnum:
					op66(r32);
					asmparam=FALSE;
					longholdnumber=doconstdwordmath();
					next=0;
					if(i==tk_number&&(postnumflag&f_reloc)==0&&short_ok(longholdnumber,TRUE)){
						op(128+2+1);
						op(128+64+hstok.number+basecode);
						op((int)longholdnumber);
					}
					else{
						if(hstok.number==EAX)op(4+1+basecode);
						else{
							op(128+1);
							op(128+64+hstok.number+basecode);
						}
						if((postnumflag&f_reloc)!=0)AddReloc();
						if(i==tk_undefofs)AddUndefOff(2,hstok.name);
						outdword(longholdnumber);
					}
					break;
				case tk_postnumber:
					op66(r32);
					if(hstok.number==EAX)op(4+1+basecode);
					else{
						op(128+1);
						op(128+64+hstok.number+basecode);
					}
					(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
					outdword(itok.number);
					break;
				case tk_reg32:
					op66(r32);
					op(2+1+basecode);
					op(128+64+(unsigned int)itok.number+hstok.number*8);
					break;
				case tk_dwordvar:
				case tk_longvar:
					CheckAllMassiv(bufrm,4,&strinf);
					op66(r32);
					outseg(&itok,2);
					op(2+1+basecode);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					break;
				default: dwordvalexpected(); break;
			}
			if(cpu<3)cpu=3;
			break;
		case tk_beg:
			if((basecode/8)!=a_cmp)ClearReg(hstok.number>3?hstok.number-4:hstok.number);
			switch(tok){
				case tk_minus:
					if(tok2!=tk_number){
						bytevalexpected(2);
						break;
					}
				case tk_number:
					asmparam=FALSE;
					holdnumber2=doconstlongmath();
					next=0;
					if(hstok.number==AL)op(4+basecode);
					else{
						op(128);
						op(128+64+hstok.number+basecode);
					}
					op(holdnumber2);
					break;
				case tk_beg:
					op(2+basecode);
					op(128+64+(unsigned int)itok.number+hstok.number*8);
					break;
				case tk_bytevar:
				case tk_charvar:
					CheckAllMassiv(bufrm,1,&strinf);
					outseg(&itok,2);
					op(2+basecode);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					break;
				default: bytevalexpected(2); break;
			}
			break;
		case tk_dwordvar:
		case tk_longvar:
			typet=r32;
			if(cpu<3)cpu=3;
		case tk_intvar:
		case tk_wordvar:
			if((basecode/8)!=a_cmp)KillVar(hstok.name);
			CheckAllMassiv(hbuf,typet,&hstr,&hstok);
			op66(typet);
			outseg(&hstok,2);
			switch(tok){
				case tk_undefofs:
					strcpy(hstok.name,itok.name);
					tok=tk_number;
					goto vnum;
				case tk_minus:
					if(tok2!=tk_number)goto erval;
				case tk_number:
vnum:
					asmparam=FALSE;
					holdnumber2=doconstlongmath();
#ifdef OPTVARCONST
					if(i==tk_number&&(postnumflag&f_reloc)==0){
						initconst=UpdVarConst(&hstok,holdnumber2,tk_dword,operand);
					}
#endif
					next=0;
					if(i!=tk_undefofs&&(postnumflag&f_reloc)==0&&short_ok(holdnumber2,typet/2-1)){
						op(128+2+1);
						op(hstok.rm+basecode);
						outaddress(&hstok);
						op(holdnumber2);
					}
					else{
						op(128+1);
						op(hstok.rm+basecode);
						outaddress(&hstok);
						if((postnumflag&f_reloc)!=0)AddReloc();
						if(i==tk_undefofs)AddUndefOff(2,hstok.name);
						if(typet==r16)outword(holdnumber2);
						else outdword(holdnumber2);
					}
					break;
				case tk_postnumber:
					if(typet==r32)goto erval;
					op(128+1);
					op(hstok.rm+basecode);
					outaddress(&hstok);
					(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
					if(typet==r16)outword(itok.number);
					else outdword(itok.number);
					break;
				case tk_reg32:
					if(typet==r16)goto erval;
				case tk_reg:
					op(1+basecode);
					op(hstok.rm+(unsigned int)itok.number*8);
					outaddress(&hstok);
					break;
				default:
erval:
					if(typet==r16)wordvalexpected();
					else dwordvalexpected();
					break;
			}
#ifdef OPTVARCONST
			if(initconst==FALSE)ClearVarByNum(&hstok);
#endif

			break;
		case tk_bytevar:
		case tk_charvar:
			if((basecode/8)!=a_cmp)KillVar(hstok.name);
			CheckAllMassiv(hbuf,1,&hstr,&hstok);
			switch(tok){
				case tk_minus:
					if(tok2!=tk_number){
						bytevalexpected(2);
						break;
					}
				case tk_number:
					asmparam=FALSE;
					holdnumber2=doconstlongmath();
#ifdef OPTVARCONST
					if((postnumflag&f_reloc)==0){
						initconst=UpdVarConst(&hstok,holdnumber2,tk_byte,operand);
					}
#endif
					next=0;
					outseg(&hstok,2);
					op(128);
					op(hstok.rm+basecode);
					outaddress(&hstok);
					op(holdnumber2);
					break;
				case tk_beg:
					outseg(&hstok,2);
					op(basecode);
					op(hstok.rm+(unsigned int)itok.number*8);
					outaddress(&hstok);
					break;
				default: bytevalexpected(2); break;
			}
#ifdef OPTVARCONST
			if(initconst==FALSE)ClearVarByNum(&hstok);
#endif
			break;
		default: varexpected(1);	break;
	}
	if(next)nexttok();
}

int GOTO()
{
unsigned char next=1;
	CheckIP();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	clearregstat();
	switch(tok){
		case tk_proc:
		case tk_interruptproc:
			if(itok.segm!=NOT_DYNAMIC){
				idrec *ptr=itok.rec;
				itok.segm=ptr->recsegm=DYNAMIC_USED;
				itok.rm=tok=tk_undefofs;	//смещение еще не известной метки
				goto undefproc;
			}
			jumploc(itok.number);	//на процедуры опр ранеее
			break;
		case tk_number:
			asmparam=FALSE;
			jumploc(doconstlongmath());//на конкретный адрес
			next=0;
			break;
		case tk_ID:
			addlocaljump(CALL_SHORT); //возможную локальную метку
			outword(0x00EB); 	// JMP SHORT
			break;
		case tk_id:
			tobedefined(CALL_SHORT,tk_void);//возможно обьявленую метку
			outword(0x00EB);	// JMP SHORT
			break;
		case tk_declare:
			tok=tk_undefproc;
			updatetree();
		case tk_locallabel:
		case tk_undefproc:		//необъявленую проц
undefproc:
			addacall((unsigned int)itok.number,(unsigned char)((itok.flag&f_extern)!=0?CALL_EXT:CALL_SHORT));
			outword(0x00EB);	// JMP SHORT
			break;
		default: shortjumperror(); break;
	}
	return next;
}

unsigned char gotol(int faradd)
{
unsigned char next=1;
unsigned long hnumber;
unsigned int i=0;
	CheckIP();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	clearregstat();
	switch(tok){
		case tk_declare:
			if(tok2!=tk_openbracket){
				tok=tk_undefproc;
				updatetree();
			}
		case tk_locallabel:
		case tk_undefproc:
			if(tok2==tk_openbracket){
				asmparam=FALSE;
				if(doanyundefproc(TRUE)==tokens)next=0;
				break;
			}
			if(faradd==0){
				addacall((unsigned int)itok.number,(unsigned char)((itok.flag&f_extern)!=0?CALL_EXT:(am32==FALSE?JMP_NEAR:JMP_32)));
				jumploc0();
			} 			/* JMP num */
			else invalidfarjumpitem();
			break;
		case tk_proc:
			if(tok2==tk_openbracket){
				doanyproc(TRUE);
				break;
			}
			if(itok.segm!=NOT_DYNAMIC){
				idrec *ptr=itok.rec;
				itok.segm=ptr->recsegm=DYNAMIC_USED;
				itok.rm=tok=tk_undefofs;	//смещение еще не известной метки
		 		if(faradd==0){
					addacall((unsigned int)itok.number,(unsigned char)((itok.flag&f_extern)!=0?CALL_EXT:(am32==FALSE?JMP_NEAR:JMP_32)));
					jumploc0();
					break;
				} 			/* JMP num */
			}
		case tk_interruptproc:
			if(faradd==0)jumploc(itok.number);
			else invalidfarjumpitem();
			break;
		case tk_apiproc:
			asmparam=FALSE;
			if(tok2==tk_openbracket){
				if(doanyundefproc(TRUE)==tokens)next=0;
			}
			else{
				if(FastCallApi==TRUE){
					outword(0x25ff);
					AddApiToPost(itok.number);
				}
				else{
					addacall(itok.number,(unsigned char)CALL_32);
					jumploc0();
				}
			}
			break;
		case tk_minus:
			if(tok2!=tk_number)goto err;
		case tk_number:
			asmparam=FALSE;
			hnumber=doconstdwordmath();
			if(faradd==0)jumploc(hnumber);
			else{
				op(0xEA);
				expecting2(tk_colon);
				int htok=tok;
				char name[IDLENGTH];
				if(tok==tk_undefofs){
					tok=tk_number;
					strcpy(name,itok.name);
				}
				unsigned long tempi=doconstdwordmath();
				if(postnumflag&f_reloc)AddReloc();
				if(htok==tk_undefofs)AddUndefOff(2,name);
				if(am32==FALSE)outword((unsigned int)tempi);
				else outdword(tempi);
				outword((unsigned int)hnumber);
			}
			next=0;
			break;	 /* JMP num */
		case tk_postnumber:
			if(faradd==0){
				op(0xE9);
				(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
				if(am32==FALSE)outword((unsigned int)itok.number);
				else outdword(itok.number);
			}
			else invalidfarjumpitem();
			break;
		case tk_reg32:
			op66(r32);
			goto jmpreg;
		case tk_reg:
			op66(r16);
jmpreg:
			if(faradd==0){
				op(0xFF);
				op(0xE0+(unsigned int)itok.number);
			} 	 /* JMP reg */
			else invalidfarjumpitem();
			break;
		case tk_ID:
			if(faradd==0){
				addlocaljump(am32==FALSE?JMP_NEAR:JMP_32);
				jumploc0();
			} 		 /* JMP num */
			else invalidfarjumpitem();
			break;
		case tk_id:
			if(faradd==0){
				tobedefined(am32==FALSE?JMP_NEAR:JMP_32,tk_void);
				jumploc0();
			} 		 /* JMP num */
			else invalidfarjumpitem();
			break;
		case tk_dwordvar:
		case tk_longvar:
			i=2;
		case tk_intvar:
		case tk_wordvar:
			i+=2;
			CheckAllMassiv(bufrm,i,&strinf);
			outseg(&itok,2);
			op(0xFF);  op(0x20+itok.rm+(faradd==0?(i==4&&am32==0?8:0):8));
			outaddress(&itok);
			break;
		default:
err:
			preerror("Invalid item for JMP");
			break;
	}
	return next;
}

void  asmregmem(int out1,int out2) // used for LEA LDS LES LFS LGS LSS.
{
int holdreg;
	nexttok();
	op66(tok==tk_reg?r16:r32);
	if(tok!=tk_reg32&&tok!=tk_reg)reg32regexpected(1);
	holdreg=(unsigned int)itok.number;
	ClearReg(itok.number);
	nextexpecting2(tk_camma);
	switch(tok){
		case tk_intvar:
		case tk_wordvar:
		case tk_charvar:
		case tk_bytevar:
		case tk_longvar:
		case tk_dwordvar:
			break;
		default: varexpected(2);
	}
	CheckAllMassiv(bufrm,2,&strinf);
	if(out2==0)outseg(&itok,2);
	else outseg(&itok,3);
	op(out1);
	if(out2!=0)op(out2);
	op(itok.rm+holdreg*8);
	outaddress(&itok);
}

void Scanbit(int basecode)
{
unsigned long hnumber;
int htok,typet=r16;
	nexttok();
	hnumber=itok.number;
	ClearReg(itok.number);
	htok=tok;
	nextexpecting2(tk_camma);
	switch(htok){
		case tk_reg32:
			typet=r32;
			if(tok==tk_reg||tok==tk_wordvar||tok==tk_intvar)goto erval;
		case tk_reg:
		  switch(tok){
				case tk_reg32:
					if(typet==r16)goto erval;
				case tk_reg:
					op66(typet);
					op(0xf);
					op(basecode);
					op(0xc0+(unsigned int)itok.number+hnumber*8);
					break;
				case tk_dwordvar:
				case tk_longvar:
					if(typet==r16)goto erval;
				case tk_wordvar:
				case tk_intvar:
					CheckAllMassiv(bufrm,typet,&strinf);
					op66(typet);
					outseg(&itok,3);
					op(0xf);
					op(basecode);
					op(itok.rm+hnumber*8);
					outaddress(&itok);
					break;
				default:
erval:
					if(typet==r16)wordvalexpected();
					else dwordvalexpected();
					break;
			}
			break;
		default: reg32regexpected(1);
	}
	if(cpu<3)cpu=3;
}

void CheckBit(int code)
{
unsigned long holdnumber2;
int htok;
ITOK hstok;
unsigned char next=1;
char *hbuf;
SINFO hstr;
	nexttok();
	hstok=itok;
	htok=tok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	switch(htok){
		case tk_reg32:
		case tk_reg:
			op66(htok==tk_reg?r16:r32);
			if(code)ClearReg(hstok.number);
		  switch(tok){
				case tk_number:
					asmparam=FALSE;
					holdnumber2=doconstlongmath();
					next=0;
					outword(0xba0f);
					op(0xe0+code+hstok.number);
					op(holdnumber2);
					break;
				case tk_reg32:
					if(tok!=htok){
						dwordvalexpected();
						break;
					}
				case tk_reg:
					if(tok!=htok)wordvalexpected();
					op(0xf);
					op(0xa3+code);
					op(0xc0+hstok.number+(unsigned int)itok.number*8);
					break;
				default: wordvalexpected(); break;
			}
			break;
		case tk_intvar:
		case tk_wordvar:
			CheckAllMassiv(hbuf,2,&hstr,&hstok);
			if(code)KillVar(hstok.name);
			op66(r16);
			switch(tok){
				case tk_number:
varc:
					asmparam=FALSE;
					holdnumber2=doconstlongmath();
					next=0;
					outseg(&hstok,3);
					outword(0xba0f);
					op(hstok.rm+code+0x20);
					outaddress(&hstok);
					op(holdnumber2);
					break;
				case tk_reg:
varreg:
					outseg(&hstok,3);
					op(0xf);
					op(0xa3+code);
					op(hstok.rm+(unsigned int)itok.number*8);
					outaddress(&hstok);
					break;
				default: wordvalexpected(); break;
			}
#ifdef OPTVARCONST
			if(code)ClearVarByNum(&hstok);
#endif
			break;
		case tk_dwordvar:
		case tk_longvar:
			CheckAllMassiv(hbuf,4,&hstr,&hstok);
			if(code)KillVar(hstok.name);
			op66(r32);
			if(tok==tk_number)goto varc;
			if(tok==tk_reg32)goto varreg;
			dwordvalexpected();
			break;
		default: varexpected(1);
	}
	if(cpu<3)cpu=3;
	if(next)nexttok();
}

void asmone1(int basecode)			 // used for INC and DEC.
{
int razr=r16;
#ifdef OPTVARCONST
int operand=GetOperand(basecode/8+a_inc);
#endif
	nexttok();
	switch(tok){
		case tk_reg32:
			if(cpu<3)cpu=3;
			razr=r32;
		case tk_reg:
			ClearReg(itok.number);
			op66(razr);
		  op(64+basecode+(unsigned int)itok.number);
			break;
		case tk_beg:
			ClearReg(itok.number);
			op(254); op(128+64+basecode+(unsigned int)itok.number);
			break;
		case tk_charvar:
		case tk_bytevar:
#ifdef OPTVARCONST
			UpdVarConst(&itok,1,tk_byte,operand);
#endif
			CheckAllMassiv(bufrm,1,&strinf);
			KillVar(itok.name);
			outseg(&itok,2);
			op(254);
			op(itok.rm+basecode);
			outaddress(&itok);
			break;
		case tk_longvar:
		case tk_dwordvar:
			CheckAllMassiv(bufrm,4,&strinf);
			if(cpu<3)cpu=3;
			razr=r32;
			goto dec;
		case tk_intvar:
		case tk_wordvar:
			CheckAllMassiv(bufrm,2,&strinf);
dec:
#ifdef OPTVARCONST
			UpdVarConst(&itok,1,tk_dword,operand);
#endif
			KillVar(itok.name);
			op66(razr);
			outseg(&itok,2);
			op(255);
			op(itok.rm+basecode);
			outaddress(&itok);
			break;
		default: varexpected(0);	break;
	}
}

void asmone2(int basecode)			 // used for NEG NOT MUL IMUL DIV IDIV.
{
int razr=r16;
#ifdef OPTVARCONST
int operand=GetOperand((basecode-16)/8+a_not);
#endif
	nexttok();
	switch(tok){
		case tk_reg32:
			if(cpu<3)cpu=3;
			razr=r32;
		case tk_reg:
			ClearReg(itok.number);
			op66(razr);
		  op(246+1); op(128+64+basecode+(unsigned int)itok.number); break;
		case tk_beg:
			ClearReg(itok.number);
			op(246); op(128+64+basecode+(unsigned int)itok.number); break;
			razr=r8;
		case tk_charvar:
		case tk_bytevar:
#ifdef OPTVARCONST
			UpdVarConst(&itok,0,tk_dword,operand);
#endif
			CheckAllMassiv(bufrm,1,&strinf);
			KillVar(itok.name);
			outseg(&itok,2);
			op(246);
			op(itok.rm+basecode);
			outaddress(&itok);
			razr=r8;
			break;
		case tk_longvar:
		case tk_dwordvar:
			CheckAllMassiv(bufrm,4,&strinf);
			if(cpu<3)cpu=3;
			razr=r32;
			goto neg;
		case tk_intvar:
		case tk_wordvar:
			CheckAllMassiv(bufrm,2,&strinf);
neg:
#ifdef OPTVARCONST
			UpdVarConst(&itok,0,tk_dword,operand);
#endif
			KillVar(itok.name);
			op66(razr);
			outseg(&itok,2);
			op(247);
			op(itok.rm+basecode);
			outaddress(&itok);
			break;
		default: varexpected(0);	break;
	}
	if(basecode!=16&&basecode!=24){
		ClearReg(AX);
		if(razr!=r8)ClearReg(DX);
	}
}

void  asmshortjump(int shortcode,int nearcode)
{
unsigned char next=1,shortjump=1;
unsigned int address;
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	nexttok();
	if(stricmp("FAR",itok.name)==0){ 	 // case insensitive
		preerror("FAR jump not available for this instruction");
		nexttok();
	}
	else if(stricmp("NEAR",itok.name)==0){  // case insensitive
		shortjump=0;
		nexttok();
	}
	else if(stricmp("SHORT",itok.name)==0)nexttok();  // case insensitive
	if(shortjump){
		CheckIP();
		switch(tok){
			case tk_proc:
				*(unsigned int *)&itok.number-=outptr+2;
				if(short_ok(itok.number)){
					op(shortcode);
					op((unsigned int)itok.number);
				}
				else shortjumptoolarge();
				break;
			case tk_number:
				asmparam=FALSE;
				address=doconstdwordmath()-(outptr+2);
				if(short_ok(address)){
					op(shortcode);
					op(address);
				}
				else shortjumptoolarge();
				next=0;
				break;
			case tk_ID:
				addlocaljump(CALL_SHORT);
				op(shortcode); op(0x00);	 /* JXX SHORT */
				break;
			case tk_id:
				tobedefined(CALL_SHORT,tk_void);
				op(shortcode); op(0x00);	 /* JXX SHORT */
				break;
			case tk_declare:
				tok=tk_undefproc;
				updatetree();
			case tk_locallabel:
			case tk_undefproc:
				addacall((unsigned int)itok.number,(unsigned char)((itok.flag&f_extern)!=0?CALL_EXT:CALL_SHORT));
				op(shortcode); op(0x00);		/* JXX SHORT */
				break;
			default: shortjumperror(); break;
		}
	}
	else if(nearcode!=0){
unsigned long numlong;
		asmparam=FALSE;
		switch(tok){
			case tk_proc:
				op(0xF); op(nearcode);
				numlong=itok.number-outptr-2;
				if(am32==FALSE)outword((unsigned int)numlong);
				else outdword(numlong-2);
				break;
			case tk_number:
				op(0xF); op(nearcode);
				numlong=doconstdwordmath()-outptr-2;
				if(am32==FALSE)outword((unsigned int)numlong);
				else outdword(numlong-2);
				next=0;
				break;
			case tk_undefofs:	// ???? 32 битный режим
				op(0xF); op(nearcode);
				AddUndefOff(2,itok.name);
				outword(-(int)(outptr-2));
				if(am32!=FALSE)outword(0x0000);
				break;
			case tk_postnumber:
				op(0xF); op(nearcode);
				(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
				numlong=itok.number-outptr-2;
				if(am32==FALSE)outword((unsigned int)numlong);
				else outdword(numlong-2);
				break;
			case tk_ID:
				op(0xF);	 // must go before tobedefined()
				addlocaljump(am32==FALSE?CALL_NEAR:CALL_32);
				op(nearcode); outword(0x0000);			/* JXX NEAR */
				if(am32!=FALSE)outword(0x0000);
				break;
			case tk_id:
				op(0xF);	 // must go before tobedefined()
				tobedefined(am32==FALSE?CALL_NEAR:CALL_32,tk_void);
				op(nearcode); outword(0x0000);			/* JXX NEAR */
				if(am32!=FALSE)outword(0x0000);
				break;
			case tk_declare:
				tok=tk_undefproc;
				updatetree();
			case tk_locallabel:
			case tk_undefproc:
				op(0xF);	 // must go before addacall()
				addacall((unsigned int)itok.number,(unsigned char)((itok.flag&f_extern)!=0?CALL_EXT:(am32==FALSE?CALL_NEAR:CALL_32)));
				op(nearcode); outword(0x0000);  /* JXX NEAR */
				if(am32!=FALSE)outword(0x0000);
				break;
			default: preerror("Invalid operand for NEAR jump"); break;
		}
		if(cpu<3)cpu=3;
	}
	else preerror("NEAR jump not available for this instruction");
	if(next)nexttok();
}

void lar_lsl(int code)
{
unsigned char possiblecpu=0;
int razr=r16;
	nexttok();
	if(tok!=tk_reg&&tok!=tk_reg32)reg32regexpected(1);
	int htok=tok;
	int hnumber=(unsigned int)itok.number;
	ClearReg(itok.number);
	nextexpecting2(tk_camma);
	switch(tok){
		case tk_reg32:
			if(htok==tk_reg)reg32expected(1);
			possiblecpu=3;
			razr=r32;
			goto lar;
		case tk_reg:
			if(htok==tk_reg32)regexpected(1);
			possiblecpu=2;
lar:
			op66(razr);
			op(0x0f);
			op(code);
			op(0xc0+(unsigned int)itok.number+hnumber*8);
			break;
		case tk_longvar:
		case tk_dwordvar:
			if(htok==tk_reg)reg32expected(1);
			CheckAllMassiv(bufrm,4,&strinf);
			possiblecpu=3;
			razr=r32;
			goto lar1;
		case tk_intvar:
		case tk_wordvar:
			if(htok==tk_reg32)regexpected(1);
			possiblecpu=2;
			CheckAllMassiv(bufrm,2,&strinf);
lar1:
			op66(razr);
			outseg(&itok,3);
			op(0x0f);
			op(code);
			op(itok.rm+hnumber*8);
			outaddress(&itok);
			break;
		default: invalidoperand(2); break;
	}
	if(possiblecpu>cpu)cpu=possiblecpu;
}

unsigned char tabldeckr(int code)
{
unsigned int i=0;
int htok;
unsigned char next=1;
char name[IDLENGTH];
	nexttok();
	htok=tok;
	KillVar(itok.name);
	switch(tok){
		case tk_longvar:
		case tk_dwordvar:
			i=2;
		case tk_intvar:
		case tk_wordvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			i++;
			CheckAllMassiv(bufrm,i,&strinf);
			outseg(&itok,3);
			outword(0x010f);
			op(itok.rm+code);
			outaddress(&itok);
			break;
		case tk_undefofs:
			strcpy(name,itok.name);
			tok=tk_number;
		case tk_number:
			i=doconstdwordmath();
			outword(0x010f);
			op((am32==FALSE?rm_d16:rm_d32)+code);
		 	if(postnumflag&f_reloc)AddReloc();
		 	if(htok==tk_undefofs)AddUndefOff(2,name);
			if(am32)outdword(i);
			else outword(i);
			next=0;
			break;
		default: varexpected(0);
	}
	if(cpu<2)cpu=2;
	return next;
}

void  protectinstr(int code,int code2)
{
int i=0;
	nexttok();
	switch(tok){
		case tk_longvar:
		case tk_dwordvar:
			i=2;
		case tk_intvar:
		case tk_wordvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			i++;
#ifdef OPTVARCONST
			ClearVarByNum(&itok);
#endif
			CheckAllMassiv(bufrm,i,&strinf);
			KillVar(itok.name);
			outseg(&itok,3);
			op(0x0f);
			op(code);
			op(itok.rm+code2);
			outaddress(&itok);
			break;
		case tk_reg32:
		case tk_reg:
			ClearReg(itok.number);
			op(0x0f);
			op(code);
			op(0xc0+code2+(unsigned int)itok.number);
			break;
		default: wordvalexpected();
	}
	if(cpu<2)cpu=2;
}

void doasmmov() 		// do MOV
{
unsigned char next=1,possiblecpu=0;
int htok,typet;
ITOK hstok;
char *hbuf;
SINFO hstr;
#ifdef OPTVARCONST
int initconst=FALSE;
#endif
	nexttok();
	hstok=itok;
	htok=tok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	typet=r16;
int i=tok;
	switch(htok){
		case tk_reg32:
			possiblecpu=3;
			typet=r32;
		case tk_reg:
			switch(tok){
				case tk_debugreg:
					ClearReg(hstok.number);
					if(typet==r16)goto erreg;
					outword(0x210F);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					possiblecpu=4;
					if((unsigned int)itok.number<=DR3||(unsigned int)itok.number==DR6
					   ||(unsigned int)itok.number==DR7)possiblecpu=3;
					break;
				case tk_controlreg:
					ClearReg(hstok.number);
					if(typet==r16)goto erreg;
					outword(0x200F);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					possiblecpu=4;
					if((unsigned int)itok.number<=CR3)possiblecpu=3;
					break;
				case tk_testreg:
					ClearReg(hstok.number);
					if(typet==r16)goto erreg;
					outword(0x240F);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					possiblecpu=4;
					if((unsigned int)itok.number==TR6||(unsigned int)itok.number==TR7)possiblecpu=3;
					break;
				case tk_reg32:
					if(typet==r16)goto erreg;
				case tk_reg:
					RegToReg(hstok.number,itok.number,typet);
					op66(typet);
					op(0x89);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					break;
				case tk_undefofs:
					strcpy(hstok.name,itok.name);
					tok=tk_number;
					goto wnum;
				case tk_minus:
					if(tok2!=tk_number){
						wordvalexpected();
						break;
					}
				case tk_number:
wnum:
					op66(typet);
					op(0xB8+hstok.number);
					ClearReg(hstok.number);
					asmparam=FALSE;
					SaveNumber(typet,i,hstok.name);
					next=0;
					break;
				case tk_postnumber:
					ClearReg(hstok.number);
					op66(typet);
					op(0xB8+hstok.number);
					(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
					if(typet==r32)outdword(itok.number);
					else outword((unsigned int)itok.number);
					break;
				case tk_dwordvar:
				case tk_longvar:
					if(typet==r16)goto erreg;
				case tk_wordvar:
				case tk_intvar:
					CheckAllMassiv(bufrm,typet,&strinf);
					IDZToReg(itok.name,hstok.number,typet);
					op66(typet);
					if(hstok.number==0&&((itok.rm==rm_d16&&itok.sib==CODE16)||(itok.rm==rm_d32&&(itok.sib==CODE32||itok.sib==0)))){
						outseg(&itok,1);
						op(0xA1);
					}
					else{
						outseg(&itok,2);
						op(0x8B);
						op(itok.rm+hstok.number*8);
					}
					outaddress(&itok);
					break;
				case tk_seg:
					if(typet==r32)goto erreg;
					IDZToReg(itok.name,hstok.number,typet);
					op66(r16);
					op(0x8C);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					if((unsigned int)itok.number==FS||(unsigned int)itok.number==GS)possiblecpu=3;
					break;
				default:
erreg:
					invalidoperand(2); break;
			}
			break;
		case tk_beg:
			ClearReg(hstok.number%4);
			switch(tok){
				case tk_beg:
						op(0x88);
						op(128+64+(unsigned int)itok.number*8+hstok.number);
					break;
				case tk_number:
					op(0xB0+hstok.number);
					asmparam=FALSE;
					op((int)doconstlongmath());
					next=0;
					break;
				case tk_bytevar:
				case tk_charvar:
					CheckAllMassiv(bufrm,1,&strinf);
					if(hstok.number==0&&((itok.rm==rm_d16&&itok.sib==CODE16)||(itok.rm==rm_d32&&(itok.sib==CODE32||itok.sib==0)))){
						outseg(&itok,1);
						op(0xA0);
					}
					else{
						outseg(&itok,2);
						op(0x8A);
						op(itok.rm+hstok.number*8);
					}
					outaddress(&itok);
					break;
				default: invalidoperand(2); break;
			}
			break;
		case tk_seg:
			if(hstok.number==CS){
				invalidoperand(1); break;
			}
			switch(tok){
				case tk_reg:
					op66(r16);
				  op(0x8E);
					op(128+64+hstok.number*8+(unsigned int)itok.number);
					if(hstok.number==FS||hstok.number==GS)possiblecpu=3;
					break;
				case tk_wordvar:
				case tk_intvar:
					CheckAllMassiv(bufrm,2,&strinf);
					op66(r16);
					outseg(&itok,2);
					op(0x8E);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					if(hstok.number==FS||hstok.number==GS)possiblecpu=3;
					break;
				default: invalidoperand(2); break;
			}
			break;
		case tk_dwordvar:
		case tk_longvar:
			possiblecpu=3;
			typet=r32;
		case tk_wordvar:
		case tk_intvar:
			KillVar(itok.name);
			CheckAllMassiv(hbuf,typet,&hstr,&hstok);
			op66(typet);
			switch(tok){
				case tk_reg32:
					if(typet==r16)goto ervar;
				case tk_reg:
			/*if((tok==tk_reg||tok==tk_reg32)&&hbuf==NULL&&hstr.bufstr==NULL)*/
					AddRegVar(itok.number,typet,&hstok);
					if(itok.number==0&&((hstok.rm==rm_d16&&hstok.sib==CODE16)||(hstok.rm==rm_d32&&(hstok.sib==CODE32||hstok.sib==0)))){
						outseg(&hstok,1);
						op(0xA3);
						outaddress(&hstok);
					}
					else{
						outseg(&hstok,2);
						op(0x89);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
					}
					break;
				case tk_undefofs:
					strcpy(hstok.name,itok.name);
					tok=tk_number;
					goto vnum;
				case tk_minus:
					if(tok2!=tk_number){
						wordvalexpected();
						break;
					}
				case tk_number:
vnum:
					outseg(&hstok,2);
					op(0xC7);
					op(hstok.rm);
					outaddress(&hstok);
					asmparam=FALSE;
#ifdef OPTVARCONST
					unsigned long t;
					t=SaveNumber(typet,i,hstok.name);
					if(typet==r16)t&=0xffff;
					else t&=0xffffffff;
					if((i==tk_number||i==tk_minus)&&(postnumflag&f_reloc)){
						Const2Var(&hstok,t,tk_dword);
						initconst=TRUE;
					}
#else
					SaveNumber(typet,i,hstok.name);
#endif
					next=0;
					break;
				case tk_postnumber:
					outseg(&hstok,2);
					op(0xC7);
					op(hstok.rm);
					outaddress(&hstok);
					(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
					if(typet==r16)outword((unsigned int)itok.number);
					else outdword(itok.number);
					break;
				case tk_seg:
					outseg(&hstok,2);
					if(typet==r32)goto ervar;
					op(0x8C);
					op(hstok.rm+(unsigned int)itok.number*8);
					outaddress(&hstok);
					if(hstok.number==FS||hstok.number==GS)possiblecpu=3;
					break;
				default:
ervar:
					invalidoperand(2); break;
			}
#ifdef OPTVARCONST
			if(initconst==FALSE)ClearVarByNum(&hstok);
#endif
			break;
		case tk_charvar:
		case tk_bytevar:
			KillVar(itok.name);
			CheckAllMassiv(hbuf,1,&hstr,&hstok);
			switch(tok){
				case tk_beg:
			/*if(tok==tk_beg&&hbuf==NULL&&hstr.bufstr==NULL)*/
					AddRegVar(itok.number,r8,&hstok);
					if(itok.number==0&&((hstok.rm==rm_d16&&hstok.sib==CODE16)||(hstok.rm==rm_d32&&(hstok.sib==CODE32||hstok.sib==0)))){
						outseg(&hstok,1);
						op(0xA2);
						outaddress(&hstok);
					}
					else{
						outseg(&hstok,2);
						op(0x88);
						op(hstok.rm+(unsigned int)itok.number*8);
						outaddress(&hstok);
					}
					break;
				case tk_number:
					outseg(&hstok,2);
					op(0xC6);
					op(hstok.rm);
					outaddress(&hstok);
					asmparam=FALSE;
#ifdef OPTVARCONST
					long t;
					t=doconstlongmath()&0xff;
					op(t);
					if((postnumflag&f_reloc)){
						initconst=TRUE;
						Const2Var(&hstok,t,tk_byte);
					}
#else
					op((int)doconstlongmath());
#endif
					next=0;
					break;
				default: invalidoperand(2); break;
			}
#ifdef OPTVARCONST
			if(initconst==FALSE)ClearVarByNum(&hstok);
#endif
			break;
		case tk_debugreg:
			if(tok==tk_reg32){
				outword(0x230F);
				op(128+64+hstok.number*8+(unsigned int)itok.number);
				possiblecpu=4;
				if(hstok.number<=DR3||hstok.number==DR6||hstok.number==DR7)possiblecpu=3;
			}
			else invalidoperand(2);
			break;
		case tk_controlreg:
			if(tok==tk_reg32){
				outword(0x220F);
				op(128+64+hstok.number*8+(unsigned int)itok.number);
				possiblecpu=4;
				if(hstok.number<=CR3)possiblecpu=3;
			}
			else invalidoperand(2);
			break;
		case tk_testreg:
			if(tok==tk_reg32){
				outword(0x260F);
				op(128+64+hstok.number*8+(unsigned int)itok.number);
				possiblecpu=4;
				if(hstok.number==TR6||hstok.number==TR7)possiblecpu=3;
			}
			else invalidoperand(2);
			break;
		default: invalidoperand(1); break;
	}
	asmparam=FALSE;
	if(possiblecpu>cpu)cpu=possiblecpu;
	if(next)nexttok();
}

void asmextend(int basecode)	 // procedure MOVSX and MOVZX
{
int regnum;
int razr=r16;
	nexttok();
	if(tok==tk_reg32)razr=r32;
	if(tok!=tk_reg32&&tok!=tk_reg)reg32regexpected(1);
	regnum=(unsigned int)itok.number*8;
	ClearReg(itok.number);
	nextexpecting2(tk_camma);
	switch(tok){
		case tk_reg:
			op66(razr);
			op(0xF); op(basecode|1);
			op(128+64+regnum+(unsigned int)itok.number);
			break;
		case tk_beg:
			op66(razr);
			op(0xF); op(basecode);
			op(128+64+regnum+(unsigned int)itok.number);
			break;
		case tk_wordvar:
		case tk_intvar:
			CheckAllMassiv(bufrm,2,&strinf);
			op66(razr);
			outseg(&itok,3);
			op(0xF); op(basecode|1);
			op(itok.rm+regnum);
			outaddress(&itok);
			break;
		case tk_bytevar:
		case tk_charvar:
			CheckAllMassiv(bufrm,1,&strinf);
			op66(razr);
			outseg(&itok,3);
			op(0xF); op(basecode);
			op(itok.rm+regnum);
			outaddress(&itok);
			break;
		default: varexpected(2); break;
	}
	if(cpu<3)cpu=3;
}

void movd()
{
ITOK hstok;
int htok;
char *hbuf;
SINFO hstr;
int i=0;
	nexttok();
	hstok=itok;
	htok=tok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	if(tok==tk_xmmreg)i++;
	switch(htok){
		case tk_reg32:
			ClearReg(hstok.number);
			if(tok==tk_mmxreg||i){	//MOVD EAX,MM0
				if(i)op(0x66);
				outword(0x7E0F);
				op(0xc0+hstok.number+(unsigned int)itok.number*8);
			}
			else mmxregexpected(2);
			break;
		case tk_dwordvar:
		case tk_longvar:
			if(tok==tk_mmxreg||i){	//MOVD mem,MM0
#ifdef OPTVARCONST
				ClearVarByNum(&hstok);
#endif
				CheckAllMassiv(hbuf,4,&hstr,&hstok);
				KillVar(hstok.name);
				outseg(&hstok,i==0?3:4);
				if(i)op(0x66);
				outword(0x7E0F);
				op(hstok.rm+(unsigned int)itok.number*8);
				outaddress(&hstok);
			}
			else mmxregexpected(2);
			break;
		case tk_xmmreg:
			i++;
		case tk_mmxreg:
			switch(tok){
				case tk_reg32:	//MOVD MM0,EAX
					if(i)op(0x66);
					outword(0x6E0F);
					op(0xc0+hstok.number*8+(unsigned int)itok.number);
					break;
				case tk_dwordvar:	//MOVD MMO,mem
				case tk_longvar:
					CheckAllMassiv(bufrm,4,&strinf);
					outseg(&itok,i==0?3:4);
					if(i)op(0x66);
					outword(0x6E0F);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					break;
				default: dwordvalexpected();
			}
			break;
		default: mmxregordwordexpected(1);
	}
	if(cpu<6)cpu=6;
	if(i&&cpu<9)cpu=9;
}

void movq()
{
int htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
int i=1,xmm=0;
	nexttok();
	hstok=itok;
	htok=tok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	if(tok==tk_xmmreg)xmm++;
	switch(htok){
		case tk_qwordvar:
		case tk_doublevar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			if(tok==tk_mmxreg||xmm){	//MOVQ mem,MM0
#ifdef OPTVARCONST
				ClearVarByNum(&hstok);
#endif
				KillVar(hstok.name);
				CheckAllMassiv(hbuf,i,&hstr,&hstok);
				outseg(&hstok,xmm==0?3:4);
				if(xmm){
					op(0x66);
					outword(0xD60f);
				}
				else outword(0x7F0F);
				op(hstok.rm+(unsigned int)itok.number*8);
				outaddress(&hstok);
			}
			else mmxregexpected(2);
			break;
		case tk_mmxreg:
			switch(tok){
				case tk_mmxreg:	//MOVQ MM0,MM1
					outword(0x6F0F);
					op(0xc0+hstok.number*8+(unsigned int)itok.number);
					break;
				case tk_qwordvar:
				case tk_doublevar:
					i+=4;
				case tk_longvar:
				case tk_dwordvar:
				case tk_floatvar:
					i+=2;
				case tk_wordvar:
				case tk_intvar:
					i++;
				case tk_charvar:
				case tk_bytevar:
					CheckAllMassiv(bufrm,i,&strinf);
					outseg(&itok,3);
					outword(0x6F0F);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					break;
				default: mmxregordwordexpected(2);
			}
			break;
		case tk_xmmreg:
			switch(tok){
				case tk_xmmreg:
					op(0xF3);
					outword(0x7e0F);
					op(0xc0+hstok.number*8+(unsigned int)itok.number);
					break;
				case tk_qwordvar:
				case tk_doublevar:
					i+=4;
				case tk_longvar:
				case tk_dwordvar:
				case tk_floatvar:
					i+=2;
				case tk_wordvar:
				case tk_intvar:
					i++;
				case tk_charvar:
				case tk_bytevar:
					CheckAllMassiv(bufrm,i,&strinf);
					outseg(&itok,4);
					op(0xF3);
					outword(0x7E0F);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					xmm++;
					break;
				default: xmmregorvarexpected(2);
			}
			break;
		default: mmxregordwordexpected(1);
	}
	if(cpu<6)cpu=6;
	if(xmm&&cpu<9)cpu=9;
}

void packMMX(int code,int code1,int code2)
{
unsigned int hnumber;
int htok,next=TRUE;
int i=1;
int xmm=FALSE;
	nexttok();
	hnumber=(unsigned int)itok.number;
	htok=tok;
	nextexpecting2(tk_camma);
	if(htok==tk_xmmreg)xmm=TRUE;
	else if(htok!=tk_mmxreg)mmxregexpected(1);
	switch(tok){
		case tk_mmxreg:
			if(xmm)xmmregexpected(2);
			op(0x0f);
			op(code);
			op(0xc0+hnumber*8+(unsigned int)itok.number);
			break;
		case tk_xmmreg:
			if(!xmm)mmxregexpected(2);
			else op(0x66);
			op(0x0f);
			op(code);
			op(0xc0+hnumber*8+(unsigned int)itok.number);
			break;
		case tk_number:
			if(xmm)op(0x66);
			op(0x0f);
			op(code1);
			op(code2+hnumber);
		 	asmparam=FALSE;
			op((unsigned int)doconstlongmath());
			next=FALSE;
			break;
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(bufrm,i,&strinf);
			outseg(&itok,xmm==FALSE?3:4);
			if(xmm)op(0x66);
			op(0x0f);
			op(code);
			op(itok.rm+hnumber*8);
			outaddress(&itok);
			break;
		default:
			mmxregordwordexpected(2);
			break;
	}
	if(cpu<6)cpu=6;
	if(xmm&&cpu<9)cpu=9;
	if(next==TRUE)nexttok();
}

void asmshift(int basecode) 		 // used for ROL ROR RCL RCR SHL SAL SHR SAR.
{
int htok,precode;
unsigned char holdbyte;
int usenumsh=TRUE;
ITOK hstok;
char *hbuf;
char next=1;
SINFO hstr;
int razr=r16;
#ifdef OPTVARCONST
int operand=GetOperand(basecode/8+a_rol);
#endif
	nexttok();
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	if(tok==tk_beg&&(unsigned int)itok.number==CL){
		precode=0xD2;
		usenumsh=FALSE;
	}
	else if(tok==tk_number){
		asmparam=FALSE;
		holdbyte=(unsigned char)doconstlongmath();
		if(holdbyte==1){
			precode=0xD0;
			usenumsh=FALSE;
		}
		else /*if(holdbyte!=0)*/{
			precode=0xC0;
			if(cpu<2)cpu=2;
		}
		next=0;
	}
	else clornumberexpected();
//	if(precode!=0){
		switch(htok){
			case tk_reg32:
				if(cpu<3)cpu=3;
				razr=r32;
			case tk_reg:
				ClearReg(hstok.number);
				op66(razr);
			  op(precode+1); op(128+64+basecode+hstok.number);
				break;
			case tk_beg:
				ClearReg(hstok.number);
				op(precode); op(128+64+basecode+hstok.number);
				break;
			case tk_charvar:
			case tk_bytevar:
#ifdef OPTVARCONST
				if(precode==0xD0||precode==0xc0)UpdVarConst(&hstok,holdbyte,tk_byte,operand);
				else ClearVarByNum(&hstok);
#endif
				CheckAllMassiv(hbuf,1,&hstr,&hstok);
				KillVar(hstok.name);
				outseg(&hstok,2);
				op(precode);
				op(hstok.rm+basecode);
				outaddress(&hstok);
				break;
			case tk_dwordvar:
			case tk_longvar:
				CheckAllMassiv(hbuf,4,&hstr,&hstok);
				if(cpu<3)cpu=3;
				razr=r32;
				goto rol;
			case tk_wordvar:
			case tk_intvar:
				CheckAllMassiv(hbuf,2,&hstr,&hstok);
rol:
#ifdef OPTVARCONST
				if(precode==0xD0||precode==0xc0)UpdVarConst(&hstok,holdbyte,tk_byte,operand);
				else ClearVarByNum(&hstok);
#endif
				KillVar(hstok.name);
				op66(razr);
				outseg(&hstok,2);
				op(precode+1);
				op(hstok.rm+basecode);
				outaddress(&hstok);
				break;
			default: varexpected(1); break;
		}
		if(usenumsh)op(holdbyte);
	if(next)nexttok();
}

void CheckCl(int code)
{
	op(0xf);
	if(tok==tk_beg){
		if(itok.number==CL)code++;
		else clornumberexpected();
	}
	else if(tok!=tk_number)clornumberexpected();
	op(code);
}

void Shxd(int code)
{
unsigned int h2number;
int htok,h2tok;
ITOK hstok;
char *hbuf;
unsigned char next=1;
SINFO hstr;
	nexttok();
	hstok=itok;
	htok=tok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	h2number=(unsigned int)itok.number;
	ClearReg(itok.number);
	h2tok=tok;
	nextexpecting2(tk_camma);
	switch(htok){
		case tk_reg:
			if(h2tok==tk_reg){
				op66(r16);
regreg:
				ClearReg(hstok.number);
				CheckCl(code);
				op(0xc0+hstok.number+(unsigned int)h2number*8);
				if(tok==tk_number){
					asmparam=FALSE;
					op(doconstlongmath());
					next=0;
				}
			}
			else regexpected(2);
			break;
		case tk_reg32:
			if(h2tok==tk_reg32){
				op66(r32);
				goto regreg;
			}
			else reg32expected(2);
			break;
		case tk_intvar:
		case tk_wordvar:
			if(h2tok==tk_reg){
				CheckAllMassiv(hbuf,2,&hstr,&hstok);
				op66(r16);
varreg:
#ifdef OPTVARCONST
			ClearVarByNum(&hstok);
#endif

			KillVar(hstok.name);
				outseg(&hstok,3);
				CheckCl(code);
				op(hstok.rm+h2number*8);
				outaddress(&hstok);
				if(tok==tk_number){
					asmparam=FALSE;
					op(doconstlongmath());
					next=0;
				}
			}
			else regexpected(2);
			break;
		case tk_dwordvar:
		case tk_longvar:
			if(h2tok==tk_reg32){
				CheckAllMassiv(hbuf,4,&hstr,&hstok);
				op66(r32);
				goto varreg;
			}
			else reg32expected(2);
			break;
		default: valueexpected();
	}
	if(cpu<3)cpu=3;
	if(next)nexttok();
}

void FpuType1(unsigned int addrm)
{
int opcode=0xd8;
int oscan=scanlexmode;
	scanlexmode=ASMLEX;
	nexttok();
	retoldscanmode(oscan);
	if(tok==tk_endline||tok==tk_semicolon){
		op(0xD8);
		op(0xC1+addrm);
		return;
	}
	if(tok==tk_double){
		opcode=0xdc;
		nexttok();
	}
	switch(tok){
		case tk_fpust:
			op(0xD8);
			op(0xC0+(unsigned int)itok.number+addrm);
			break;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			CheckAllMassiv(bufrm,4,&strinf);
			outseg(&itok,2);
			op(opcode);
			op(itok.rm+addrm);
			outaddress(&itok);
			break;
		case tk_doublevar:
			CheckAllMassiv(bufrm,8,&strinf);
			outseg(&itok,2);
			op(0xDC);
			op(itok.rm+addrm);
			outaddress(&itok);
			break;
		default: fpuvarexpected(0);
	}
}

void FpuType2(unsigned int addrm,unsigned int addrm2)
{
long hnum;
int opcode=0xd8;
int oscan=scanlexmode;
	scanlexmode=ASMLEX;
	nexttok();
	retoldscanmode(oscan);
	if(tok==tk_endline||tok==tk_semicolon){
		op(0xDE);
		op(0xC1+addrm);
		return;
	}
	if(tok==tk_double){
		opcode=0xdc;
		nexttok();
	}
	switch(tok){
		case tk_fpust:
			hnum=itok.number;
			nextexpecting2(tk_camma);
			if(hnum==0){
				if(tok==tk_fpust){
					op(0xD8);
					op(0xC0+addrm2+(unsigned int)itok.number);
				}
				else fpustakexpected(2);
			}
			else{
				if(tok==tk_fpust){
					if(itok.number!=0)fpu0expected();
					op(0xDC);
					op(0xC0+(unsigned int)hnum+addrm);
				}
				else fpustakexpected(2);
			}
			break;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			CheckAllMassiv(bufrm,4,&strinf);
			outseg(&itok,2);
			op(opcode);
			op(addrm2+itok.rm);
			outaddress(&itok);
			break;
		case tk_doublevar:
			CheckAllMassiv(bufrm,8,&strinf);
			outseg(&itok,2);
			op(0xDC);
			op(addrm2+itok.rm);
			outaddress(&itok);
			break;
		default: fpuvarexpected(1);
	}
}

void FpuType3(unsigned int opcode,unsigned int addrm)
{
int oscan=scanlexmode;

	scanlexmode=ASMLEX;
	nexttok();
//	scanlexmode=oscan;
	retoldscanmode(oscan);
	if((tok==tk_endline||tok==tk_semicolon)&&opcode!=0xDD){
		op(opcode);
		op(0xC1+addrm);
	}
	else if(tok==tk_fpust){
		op(opcode);
		op(0xC0+(unsigned int)itok.number+addrm);
	}
	else fpustakexpected(1);
	if(opcode==0xDE&&tok2==tk_camma){
		nexttok();
		nexttok();
		if(tok!=tk_fpust||itok.number!=0)fpu0expected();

	}
}

void FpuType4(unsigned int opcode,unsigned int addrm)
{
	nexttok();
	if(opcode==1&&(addrm==0x18||addrm==0)&&tok==tk_qwordvar){
		if(addrm==0)addrm=0x28;
		else addrm=0x38;
		CheckAllMassiv(bufrm,8,&strinf);
		KillVar(itok.name);
		outseg(&itok,2);
		op(0xDF);
		op(itok.rm+addrm);
		outaddress(&itok);
		return;
	}
	switch(tok){
		case tk_wordvar:
		case tk_intvar:
			CheckAllMassiv(bufrm,2,&strinf);
			KillVar(itok.name);
			outseg(&itok,2);
			op(0xDE + opcode);
			op(itok.rm+addrm);
			outaddress(&itok);
			break;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			CheckAllMassiv(bufrm,4,&strinf);
			KillVar(itok.name);
			outseg(&itok,2);
			op(0xDA+opcode);
			op(itok.rm+addrm);
			outaddress(&itok);
			break;
		default: fpuvarexpected(0);
	}
}

void  FpuType5(unsigned int opcode,unsigned int addrm)
{
int opc=0xd9;
int i=4;
	nexttok();
	if(tok==tk_qword||tok==tk_double){
		nexttok();
		tok=tk_qwordvar;
	}
	else if((strcmp(itok.name,"tbyte")==0||strcmp(itok.name,"ldouble")==0)&&addrm!=0x10){
		opc=0xdb;
		i=10;
		if(addrm==0)addrm=40;
		else addrm=56;
		nexttok();
		if(tok>=tk_charvar&&tok<=tk_wordvar)tok=tk_dwordvar;
	}
	switch(tok){
		case tk_fpust:
			op(opcode);
			op(0xC0+(unsigned int)itok.number+addrm);
			break;
		case tk_qwordvar:
		case tk_doublevar:
			opc=0xdd;
			i=8;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			CheckAllMassiv(bufrm,i,&strinf);
			if(opcode!=0xD9)KillVar(itok.name);
			outseg(&itok,2);
			op(opc);
			op(itok.rm+addrm);
			outaddress(&itok);
			break;
		default: fpuvarexpected(0);
	}
}

void FpuType6(unsigned int opcode,unsigned int addrm)
{
int i=0;
	nexttok();
	if(opcode==0xDF){
		if(tok==tk_qword){
			nexttok();
			tok=tk_qwordvar;
		}
		if(strcmp(itok.name,"tbyte")==0){
			i=2;
			nexttok();
			tok=tk_qwordvar;
		}
	}
	switch(tok){
		case tk_qwordvar:
		if(opcode!=0xDF)wordvalexpected();
			i=+4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i+=2;
			CheckAllMassiv(bufrm,i,&strinf);
			KillVar(itok.name);
			outseg(&itok,2);
			op(opcode);
			op(itok.rm+addrm);
			outaddress(&itok);
			break;
		default: wordvalexpected();
	}
}

void FpuType7(unsigned int addrm)
{
	nexttok();
	switch(tok){
		case tk_wordvar:
		case tk_intvar:
			CheckAllMassiv(bufrm,2,&strinf);
			if(addrm!=0x28)KillVar(itok.name);
			outseg(&itok,2);
			op(0xD9);
			op(itok.rm+addrm);
			outaddress(&itok);
			break;
		default: wordvalexpected();
	}
}

void FpuType8(unsigned int opcode,unsigned int addrm)
{
	nexttok();
	if(tok==tk_fpust&&itok.number==0&&tok2==tk_camma){
		nexttok();
		nexttok();
	}
	if(tok==tk_fpust){
		op(opcode);
		op(0xC0+(unsigned int)itok.number+addrm);
		if(cpu<7)cpu=7;
	}
	else fpustakexpected(1);
}

/* ***************** start of some quick codes ****************** */

int  short_ok(long thenumber,int reg32)
{
	if(reg32==TRUE){
		if(thenumber<=SHORTMAX&&thenumber>=SHORTMIN)return(1);
	}
	else{
		if((short)thenumber<=SHORTMAX&&(short)thenumber>=SHORTMIN)return(1);
	}
	return(0);
}

void cbw()
{
	if(optimizespeed&&(chip==5||chip==6)){
		outdword(0xFCC0C488);	//mov ah,al	sar AH,7
		op(7);
	}
	else{
		op66(r16);
		op(0x98);
	}
	ClearReg(AX);
}

void stosb()
{
	op(0xAA);
	ClearReg(DI);
}

void stosw()
{
	op66(r16);
	op(0xAB);
	ClearReg(DI);
}

void stosd()
{
	op66(r32);
	op(0xAB);
	if(cpu<3)cpu=3;
	ClearReg(DI);
}

void movsb()
{
	op(0xA4);
	ClearReg(DI);
	ClearReg(SI);
}

void movsw()
{
	op66(r16);
	op(0xA5);
	ClearReg(DI);
	ClearReg(SI);
}

void movsd()
{
	op66(r32);
	op(0xA5);
	if(cpu<3)cpu=3;
	ClearReg(DI);
	ClearReg(SI);
}

void pushds()  /* produce PUSH DS */
{
	RestoreStack();
	op(0x1E);
}

void pushss()
{
	RestoreStack();
	op(0x16);			/* PUSH SS */
}

void popes()	 /* produce POP ES */
{
	RestoreStack();
	op(0x07);
}

void ret()		/* produce RET */
{
	RestoreStack();
	op(0xC3);
}

void retf() 	/* produce RETF */
{
	RestoreStack();
	op(0xCB);
}

void jumploc(long loc)		 /* produce JUMP # */
{

	loc=loc-outptr-3;
	if(loc>-130&&loc<127){
		loc++;
		op(0xEB);
		op(loc);
		if(loc==0)notunreach=TRUE;
	}
	else{
		if(am32==FALSE){
			op(0xE9);
			outword(loc);
		}
		else{
			if(!optimizespeed&&(loc>126&&loc<65533)){
				outword(0xE966);
				outword(loc-1);
			}
			else{
				op(0xE9);
				outdword(loc-2);
			}
		}
	}
}

void callloc(long loc)	 /* produce CALL # */
{
	loc=loc-outptr-3;
	op(0xE8);
	if(am32==FALSE)	outword(loc);
	else outdword(loc-2);
}

void xorAHAH()	 /* produce XOR AH,AH */
{
	outword(0xE430);
	ConstToReg(0,AH,r8);
}

void xorAXAX()	 /* produce XOR AX,AX */
{
	op66(r16);
	outword(0xC031);
	ConstToReg(0,AX,r16);
}

void xorEAXEAX()	 /* produce XOR EAX,EAX */
{
	op66(r32);
	outword(0xC031);
	if(cpu<3)cpu=3;
	ConstToReg(0,AX,r32);
}

void ZeroReg(int reg, int razr)
{
	op66(razr);
	op(0x31);
	op(0xc0+9*reg);	//xor reg,reg
	ConstToReg(0,reg,razr);
}

void fwait()
{
	op(0x9B);
}

void cwdq(int razr)
{
	op66(razr);
	if(optimizespeed&&(chip==5||chip==6)){
		outword(0xC289);	//mov dx,ax
		op66(razr);
		outword(0xFAC1);	//sar dx,15
		op(razr==r16?15:31);
	}
	else op(0x99);
	ClearReg(DX);
}

void  nextexpecting2(int want)
{
	nexttok();
	expecting2(want);
}

void expecting2(int want)
{
	if(want!=tok)SwTok(want);
	nexttok();
}

void CheckIP()
{
	if(tok==tk_dollar){
		tok=tk_number;
		itok.number=outptr;
	}
}

void jumploc0()
{
	op(0xE9);
	outword(0); 	/* the large jump */
	if(am32!=FALSE)outword(0);
}

void callloc0()
{
	op(0xE8);
	outword(0);
	if(am32!=FALSE)outword(0);
}

void Leave()
{
	if((optimizespeed&&chip>3&&chip<7)||chip==0){
		outword(0xEC89);	// MOV SP,BP
		op(0x5D);
	}
	else op(0xC9);
}

void  tobedefined(int callkind,int expectedreturn)
{
//	strcpy(itok.name,(char *)string);
	string[0]=0;
	itok.flag=(unsigned char)(tok==tk_ID?tp_fastcall:(comfile==file_w32?tp_stdcall:tp_pascal));
	tok=tk_undefproc;
	itok.number=secondcallnum;
	itok.segm=NOT_DYNAMIC;
	itok.rm=expectedreturn;
	itok.post=0;
	addtotree(itok.name);
	addacall(secondcallnum++,(unsigned char)callkind);
}

void  addlocaljump(int callkind)
{
	addlocalvar((char *)string,tk_locallabel,secondcallnum,TRUE);
	addacall(secondcallnum++,(char)callkind);
}

unsigned long SaveNumber(int type,int tok4,char *name)
{
	unsigned long t=doconstdwordmath();
	if(tok4==tk_undefofs)AddUndefOff(0,name);
	else if((postnumflag&f_reloc)!=0)AddReloc();
	if(type==r16)outword((unsigned int)t);
	else outdword(t);
	return t;
}

void Swap2tok(int *tok4, ITOK *itok4, char **buf4, SINFO *strinf4, int *tok6,
	 ITOK *itok6, char **buf6, SINFO *strinf6)
{
int htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
	htok=*tok4;
	*tok4=*tok6;
	*tok6=htok;
	hstok=*itok4;
	*itok4=*itok6;
	*itok6=hstok;
	hbuf=*buf4;
	*buf4=*buf6;
	*buf6=hbuf;
	hstr=*strinf4;
	*strinf4=*strinf6;
	*strinf6=hstr;
}

int iTest(int mode)
{
int htok,i;
ITOK hstok;
char *hbuf;
SINFO hstr;
unsigned char possiblecpu=0,next=1;
unsigned long num;
 	asmparam=TRUE;
	nexttok();
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	if(tok==tk_number)hstok.number=doconstlongmath();
	else nexttok();
	expecting2(tk_camma);
	i=r16;
//	printf("tok=%d itok.number=%u bufrm=%s htok=%d hstok.number=%u nbuf=%s\n",
//	 tok,itok.number,bufrm,htok,hstok.number,hbuf);
	if(htok==tk_number||htok==tk_postnumber)Swap2tok(&tok,&itok,&bufrm,&strinf,
			&htok,&hstok,&hbuf,&hstr);
//	printf("tok=%d itok.number=%u bufrm=%s htok=%d hstok.number=%u nbuf=%s\n",
//	 tok,itok.number,bufrm,htok,hstok.number,hbuf);
	switch(htok){
		case tk_reg32:
			i=r32;
			possiblecpu=3;
		case tk_reg:
			switch(tok){
				case tk_reg32:
					if(i==r16){
						reg32expected(1);
						return FALSE;
					}
					op66(i);
					op(0x85);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					break;
				case tk_reg:
					if(i==r32){
						reg32expected(2);
						return FALSE;
					}
					op66(i);
					op(0x85);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					break;
				case tk_number:
				 	asmparam=FALSE;
					num=doconstdwordmath();
					if(mode){
						if(num<256&&hstok.number<4)goto testal;
						if(num<65536)i=r16;
					}
					op66(i);
					if(hstok.number==AX)op(0xA9);
					else{
						op(0xF7);
						op(128+64+hstok.number);
					}
					if(i==r16)outword((unsigned int)num);
					else outdword(num);
					next=0;
					break;
				case tk_postnumber:
					if(i==r32)return FALSE;
					op66(r16);
					if(hstok.number==AX)op(0xA9);
					else{
						op(0xF7);
						op(128+64+hstok.number);
					}
					(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
					outword(itok.number);	//было 0
					break;
				case tk_dwordvar:
				case tk_longvar:
					if(i==r16){
						reg32expected(1);
						return FALSE;
					}
					CheckAllMassiv(bufrm,i,&strinf);
					op66(i);
					outseg(&itok,2);
					op(0x85);
					op(hstok.number*8+itok.rm);
					outaddress(&itok);
					break;
				case tk_wordvar:
				case tk_intvar:
					if(i==r32){
						regexpected(1);
						return FALSE;
					}
					CheckAllMassiv(bufrm,i,&strinf);
					op66(i);
					outseg(&itok,2);
					op(0x85);
					op(hstok.number*8+itok.rm);
					outaddress(&itok);
					break;
				default: return FALSE;
			}
			break;
		case tk_beg:
			switch(tok){
				case tk_beg:
					op(0x84);
					op(128+64+(unsigned int)itok.number*8+hstok.number);
					break;
				case tk_number:
				 	asmparam=FALSE;
					num=doconstdwordmath();
testal:
					if(hstok.number==AL)op(0xA8);
					else{
						op(0xF6);
						op(128+64+hstok.number);
					}
					op(num);
					next=0;
					break;
				case tk_charvar:
				case tk_bytevar:
					CheckAllMassiv(bufrm,1,&strinf);
					outseg(&itok,2);
					op(0x84);
					op(hstok.number*8+itok.rm);
					outaddress(&itok);
					break;
				default: return FALSE;
			}
			break;
		case tk_dwordvar:
		case tk_longvar:
			i=r32;
			possiblecpu=3;
		case tk_wordvar:
		case tk_intvar:
			CheckAllMassiv(hbuf,i,&hstr,&hstok);
			switch(tok){
				case tk_reg32:
					if(i==r16){
						regexpected(2);
						return FALSE;
					}
					op66(i);
					outseg(&hstok,2);
					op(0x85);
					op((unsigned int)itok.number*8+hstok.rm);
					outaddress(&hstok);
					break;
				case tk_reg:
					if(i==r32){
						reg32expected(2);
						return FALSE;
					}
					op66(i);
					outseg(&hstok,2);
					op(0x85);
					op((unsigned int)itok.number*8+hstok.rm);
					outaddress(&hstok);
					break;
				case tk_number:
				 	asmparam=FALSE;
					num=doconstdwordmath();
					if(mode){
						if(num<256)goto testbyte;
						if(num<65536)i=r16;
					}
					op66(i);
					outseg(&hstok,2);
					op(0xF7);
					op(hstok.rm);
					outaddress(&hstok);
					if(i==r32)outdword(num);
					else outword((unsigned int)num);
					next=0;
					break;
				case tk_postnumber:
					op66(i);
					outseg(&hstok,2);
					if(i==r32)return FALSE;
					op(0xF7);
					op(hstok.rm);
					outaddress(&hstok);
					(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
					outword((unsigned int)itok.number);
					break;
				default: return FALSE;
			}
			break;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(hbuf,1,&hstr,&hstok);
			switch(tok){
				case tk_beg:
					outseg(&hstok,2);
					op(0x84);
					op((unsigned int)itok.number*8+hstok.rm);
					outaddress(&hstok);
					break;
				case tk_number:
				 	asmparam=FALSE;
					num=doconstdwordmath();
testbyte:
					outseg(&hstok,2);
					op(0xF6);
					op(hstok.rm);
					outaddress(&hstok);
					op(num);
					next=0;
					break;
				default: return FALSE;
			}
			break;
		default: return FALSE;
	}
	if(cpu<possiblecpu)cpu=possiblecpu;
	if(next)nexttok();
	return TRUE;
}

void mmxiii(int type)
{
int num1,i=1;
int xmm=FALSE;
	nexttok();
	if(tok==tk_xmmreg)xmm=TRUE;
	else if(tok!=tk_mmxreg)mmxregexpected(1);
	num1=itok.number;
	nextexpecting2(tk_camma);
	switch(tok){
		case tk_mmxreg:
			if(xmm)xmmregorvarexpected(2);
			op(0x0F);
			op(type);
			op(rm_mod11+itok.number+num1*8);
			break;
		case tk_xmmreg:
			if(xmm==FALSE)mmxormem(2);
			outword(0x0F66);
			op(type);
			op(rm_mod11+itok.number+num1*8);
			break;
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(bufrm,i,&strinf);
			outseg(&itok,xmm==FALSE?3:4);
			if(xmm)op(0x66);
			op(0x0F);
			op(type);
			op(itok.rm+num1*8);
			outaddress(&itok);
			break;
		default:
			mmxormem(2);
			break;
	}
	if(cpu<8)cpu=8;
	if(xmm&&cpu<9)cpu=9;
}

void prefetch(int code,int type)
{
int i=1;
	nexttok();
	switch(tok){
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(bufrm,i,&strinf);
			KillVar(itok.name);
			outseg(&itok,3);
			op(0x0f);
			op(code);
			op(itok.rm+type*8);
			outaddress(&itok);
			break;
		default:
			datatype_expected(1);
			break;
	}
}

void pextrw()
{
int num1,num2;
int xmm=FALSE;
	nexttok();
	if(tok!=tk_reg32)reg32expected(1);
	num1=itok.number;
	ClearReg(num1);
	nextexpecting2(tk_camma);
	if(tok==tk_xmmreg)xmm=TRUE;
	else if(tok!=tk_mmxreg)mmxregexpected(2);
	num2=itok.number;
	nextexpecting2(tk_camma);
	if(tok!=tk_number)numexpected(3);
	if(xmm)op(0x66);
	outword(0xC50F);
	op(rm_mod11+num1+num2*8);
	op(doconstdwordmath());
	if(cpu<8)cpu=8;
	if(xmm&&cpu<9)cpu=9;
}

void pinsrw()
{
int num1,htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
int xmm=FALSE;
	nexttok();
	if(tok==tk_xmmreg)xmm=TRUE;
	else if(tok!=tk_mmxreg)mmxregexpected(1);
	num1=itok.number;
	nextexpecting2(tk_camma);
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	if(tok!=tk_number)numexpected(3);
	switch(htok){
		case tk_reg32:
			if(xmm)op(0x66);
			outword(0xC40F);
			op(rm_mod11+num1+hstok.number*8);
			break;
		case tk_wordvar:
		case tk_intvar:
			CheckAllMassiv(hbuf,2,&hstr);
			outseg(&hstok,xmm==FALSE?3:4);
			if(xmm)op(0x66);
			outword(0xC40F);
			op(hstok.rm+num1*8);
			outaddress(&hstok);
			break;
		default:
			reg32orword(2);
			break;
	}
	op(doconstdwordmath());
	if(cpu<8)cpu=8;
	if(xmm&&cpu<9)cpu=9;
}

void pshufw()
{
int num1,htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
int i=1;
	nexttok();
	if(tok!=tk_mmxreg)mmxregexpected(1);
	num1=itok.number;
	nextexpecting2(tk_camma);
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	if(tok!=tk_number)numexpected(3);
	switch(htok){
		case tk_mmxreg:
			outword(0x700F);
			op(rm_mod11+num1+hstok.number*8);
			break;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(hbuf,i,&hstr);
			KillVar(hstok.name);
			outseg(&hstok,3);
			outword(0x700F);
			op(hstok.rm+num1*8);
			outaddress(&hstok);
			break;
		default:
			mmxregordwordexpected(2);
			break;
	}
	op(doconstdwordmath());
}

void xmminstr(int type,int sec,int op1,int op2)
{
int num1,i=1;
	nexttok();
	if(tok!=op1){
		if(op1==tk_mmxreg)mmxregexpected(1);
		else if(op1==tk_reg32)reg32expected(1);
		else xmmregexpected(1);
	}
	if(tok==tk_reg32)ClearReg(itok.number);
	num1=itok.number;
	nextexpecting2(tk_camma);
	switch ( tok ) {
		case tk_reg32:
		case tk_mmxreg:
		case tk_xmmreg:
			if(tok!=op2){
				if(op2==tk_mmxreg)mmxregexpected(2);
				else if(op2==tk_reg32)reg32expected(2);
				else xmmregexpected(2);
			}
			if(sec)op(sec);
			op(0x0F);
			op(type);
			op(rm_mod11+itok.number+num1*8);
			break;
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(bufrm,i,&strinf);
			outseg(&itok,sec==0?3:4);
			if(sec)op(sec);
			op(0x0F);
			op(type);
			op(itok.rm+num1*8);
			outaddress(&itok);
			break;
		default:
			if(op2==tk_mmxreg)mmxregordwordexpected(2);
			else if(op2==tk_reg32)reg32orword(2);
			else xmmregorvarexpected(2);
			break;
	}
}

void xmm3instr(int type,int sec)	//xmm,xmm/mem,i8
{
int num1,i=1,htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
	nexttok();
	if(tok!=tk_xmmreg)xmmregexpected(1);
	num1=itok.number;
	nextexpecting2(tk_camma);
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	if(tok!=tk_number)numexpected(3);
	switch(htok){
		case tk_xmmreg:
			if(sec)op(sec);
			op(0x0F);
			op(type);
			op(rm_mod11+hstok.number+num1*8);
			break;
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(hbuf,i,&hstr);
			outseg(&hstok,sec==0?3:4);
			if(sec)op(sec);
			op(0x0F);
			op(type);
			op(hstok.rm+num1*8);
			outaddress(&hstok);
			break;
		default:
			xmmregorvarexpected(2);
			break;
	}
	op(doconstdwordmath());
}

void xmm2xmm(int code,int code2,int type)
{
int num;
	nexttok();
	if(tok!=type){
		if(type==tk_mmxreg)mmxregexpected(1);
		else if(type==tk_reg32)reg32expected(1);
		else xmmregexpected(1);
	}
	if(tok==tk_reg32)ClearReg(itok.number);
	num=itok.number;
	nextexpecting2(tk_camma);
	if(tok!=tk_xmmreg)xmmregexpected(2);
	if(code2)op(code2);
	op(0x0F);
	op(code);
	op(rm_mod11+itok.number+num*8);
}

void movxmm3(int code,int code2,int type)
{
int i=1,htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
	nexttok();
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	if(tok!=type){
		if(type==tk_mmxreg)mmxregexpected(2);
		else if(type==tk_xmmreg)xmmregexpected(2);
		else reg32expected(2);
	}
	switch(htok){
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i+=1;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(hbuf,i,&hstr);
			KillVar(hstok.name);
			outseg(&hstok,code2==0?3:4);
			if(code2)op(code2);
			op(0x0F);
			op(code);
			op(hstok.rm+itok.number*8);
			outaddress(&hstok);
			break;
		default: varexpected(1);
	}
}

void movxmm4(int code,int code2)
{
int i=1;
int num;
	nexttok();
	num=itok.number;
	if(tok!=tk_xmmreg)xmmregexpected(1);
	nextexpecting2(tk_camma);
	switch(tok){
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i+=1;
		case tk_charvar:
		case tk_bytevar:
			CheckAllMassiv(bufrm,i,&strinf);
			outseg(&itok,(code2==0?3:4));
			if(code2)op(code2);
			op(0x0F);
			op(code);
			op(itok.rm+num*8);
			outaddress(&itok);
			break;
		default: varexpected(2);
	}
}

void movxmm(int code,int code2,int addc)
{
int i=1,htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
	nexttok();
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	switch(htok){
		case tk_xmmreg:
			switch(tok){
				case tk_xmmreg:
					if(code2)op(code2);
					op(0x0F);
					op(code);
					op(rm_mod11+itok.number+hstok.number*8);
					break;
				case tk_qwordvar:
					i+=4;
				case tk_longvar:
				case tk_dwordvar:
				case tk_floatvar:
					i+=2;
				case tk_wordvar:
				case tk_intvar:
					i++;
				case tk_charvar:
				case tk_bytevar:
					CheckAllMassiv(bufrm,i,&strinf);
					outseg(&itok,(code2==0?3:4));
					if(code2)op(code2);
					op(0x0F);
					op(code);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					break;
				default:
					xmmregorvarexpected(2);
					break;
			}
			break;
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			if(tok!=tk_xmmreg)xmmregexpected(2);
			CheckAllMassiv(hbuf,i,&hstr);
			KillVar(hstok.name);
			outseg(&hstok,code2==0?3:4);
			if(code2)op(code2);
			op(0x0F);
			op(code+addc);
			op(hstok.rm+itok.number*8);
			outaddress(&hstok);
			break;
		default:
			xmmregorvarexpected(1);
			break;
	}
}

void movxmm2(int code,int code2)
{
int i=1,htok;
ITOK hstok;
char *hbuf;
SINFO hstr;
	nexttok();
	htok=tok;
	hstok=itok;
	hbuf=bufrm;
	bufrm=NULL;
	hstr=strinf;
	strinf.bufstr=NULL;
	nextexpecting2(tk_camma);
	switch(htok){
		case tk_xmmreg:
			switch(tok){
				case tk_qwordvar:
					i+=4;
				case tk_longvar:
				case tk_dwordvar:
				case tk_floatvar:
					i+=2;
				case tk_wordvar:
				case tk_intvar:
					i++;
				case tk_charvar:
				case tk_bytevar:
					CheckAllMassiv(bufrm,i,&strinf);
					outseg(&itok,(code2==0?3:4));
					if(code2)op(code2);
					op(0x0F);
					op(code);
					op(itok.rm+hstok.number*8);
					outaddress(&itok);
					break;
				default:
					datatype_expected(2);
					break;
			}
			break;
		case tk_qwordvar:
			i+=4;
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
			i+=2;
		case tk_wordvar:
		case tk_intvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			if(tok!=tk_xmmreg)xmmregexpected(2);
			CheckAllMassiv(hbuf,i,&hstr);
			KillVar(hstok.name);
			outseg(&hstok,code2==0?3:4);
			if(code2)op(code2);
			op(0x0F);
			op(code+1);
			op(hstok.rm+itok.number*8);
			outaddress(&hstok);
			break;
		default:
			xmmregorvarexpected(1);
			break;
	}
}

void shiftxmm(int rm)	//rxmm,i8
{
int num;
	nexttok();
	if(tok!=tk_xmmreg)xmmregexpected(1);
	num=itok.number;
	nextexpecting2(tk_camma);
	if(tok!=tk_number)numexpected(2);
	op(0x66);
	op(0x0F);
	op(0x73);
	op(rm*8+num+rm_mod11);
	op(doconstdwordmath());
}

void DDDW(int faradd)
{
int htok,i;
char name[IDLENGTH];
unsigned long hnumber;
	if(dbg&2)AddDataNullLine(faradd==0?2:4);
	dbgact++;
	asmparam=FALSE;
	do{
		i=1;
		nexttok();
		CheckIP();
		htok=tok;
		switch(tok){
			case tk_undefofs:
				tok=tk_number;
				strcpy(name,itok.name);
			case tk_number:
				hnumber=doconstdwordmath();
				if(tok==tk_id&&strcmp((char *)string,"dup")==0){
					i=hnumber;
					nexttok();
					CheckMinusNum();
					htok=tok;
					if(tok==tk_undefofs){
						tok=tk_number;
						strcpy(name,itok.name);
					}
					if(tok==tk_number)hnumber=doconstdwordmath();
					else numexpected();
				}
				for(;i!=0;i--){
				 	if(postnumflag&f_reloc)AddReloc();
				 	if(htok==tk_undefofs)AddUndefOff(2,name);
					if(faradd)outdword(hnumber);
					else outword(hnumber);
				}
				break;
			case tk_postnumber:
				setwordpost(&itok);
				if(faradd)outdword(itok.number);
				else outword(itok.number);
				nexttok();
				break;
			default:
				numexpected();
				nexttok();
				break;
		}
	}while(tok==tk_camma);
	dbgact--;
}

void AADM(int code)
{
	op(code);
	itok.number=10;
	if(tok2==tk_number)nexttok();
	op(itok.number);	//AAD
	ClearReg(AX);
}

int Push(ITOK *wtok)
{
int i;
int razr;
ITOK hstok;
unsigned long hnumber;
int possiblecpu=0;
int next=1;
	i=(am32+1)*2;
	razr=r16;
	switch(tok){
		case tk_id:
		case tk_ID:
			if((stricmp("dword",(char *)string)==0)||(stricmp("long",(char *)string)==0))i=r32;
			else if((stricmp("word",(char *)string)==0)||(stricmp("int",(char *)string)==0))i=r16;
			else return FALSE;
			goto swpushpar;
		case tk_dword:
		case tk_long:
			i=r32;
			goto swpushpar;
		case tk_int:
		case tk_word:
			i=r16;
swpushpar:
			nexttok();
			CheckMinusNum();
			if(tok==tk_number)goto pushnum;
			if(tok==tk_undefofs)goto pushundef;
			return FALSE;
		case tk_reg32:
			possiblecpu=3;
		case tk_reg:
			op66(tok==tk_reg?r16:r32);
			op(0x50+(unsigned int)itok.number);
			break;
		case tk_seg:
			PushSeg((unsigned int)itok.number);
			break;
		case tk_undefofs:
pushundef:
			hstok=itok;
			tok=tk_number;
			hnumber=doconstlongmath();
			op66(i);	////
			op(0x68);
		 	if(postnumflag&f_reloc)AddReloc();
		 	AddUndefOff(2,hstok.name);
			if(i==r16)outword(hnumber);
			else outdword(hnumber);
			possiblecpu=(unsigned char)(i==r16?2:3);
			next=0;
			break;
		case tk_minus:
			if(tok2!=tk_number)return FALSE;
		case tk_number:
pushnum:
			hnumber=doconstlongmath();
			if(i==r16)hnumber&=0xffff;
			else hnumber&=0xffffffff;
			if(wtok&&(postnumflag&f_reloc)==0)Const2Var(wtok,hnumber,i==r16?tk_word:tk_dword);
			op66(i);
			if((postnumflag&f_reloc)==0&&short_ok(hnumber,i/2-1)){
				op(0x6A);
				op(hnumber);
			}
			else{
				op(0x68);
				if((postnumflag&f_reloc)!=0)AddReloc();
				if(i==r16)outword(hnumber);
				else outdword(hnumber);
			}
			possiblecpu=(unsigned char)(i==r16?2:3);
			next=0;
			break;
		case tk_dwordvar:
		case tk_longvar:
			CheckAllMassiv(bufrm,4,&strinf);
			possiblecpu=3;
			razr=r32;
			goto push;
		case tk_wordvar:
		case tk_intvar:
			CheckAllMassiv(bufrm,2,&strinf);
push:
			op66(razr);
			outseg(&itok,2);
			op(0xFF);	op(0x30+itok.rm);
			outaddress(&itok);
			break;
		case tk_postnumber:
			op(0x68);
			(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
			if(am32==FALSE)outword((unsigned int)itok.number);
			else outdword(itok.number);
			break;
		case tk_string:
			op66(i);
			op(0x68);
			if(am32==FALSE)outword(addpoststring());
			else outdword(addpoststring());
			break;
		default:
			return FALSE;
	}
	if(cpu<possiblecpu)cpu=possiblecpu;
	asmparam=FALSE;
	if(next)nexttok();
	return i;
}
/* end of TOKR.C */


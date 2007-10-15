/*
        function for format output to the string
*/

#include <kolibrisys.h>
#include <string.h>
#include <stdio.h>
//#include <ctype.h>
#include <math.h>

int formatted_double_to_string(long double number,int format1,int format2,char *s)
{
        double	n;
        double	nbefor;
        double	nafter;
        double	v,v2;
        long	intdigit;
        long	beforpointdigit;
        long	div;
        int             i;
        int             pos;
        int             size;
        int             fmt1;
        int             fmt2;
        long    mul;
        char            buf[200];

        size=(int)s;
        n=(double)number;
        if (n<0) {*s='-';s++;n=-n;}

        fmt1=format1;
        fmt2=format2;
        if (fmt2>18) {fmt2=18;} //maximum of size long long type

        //clear array befor output
        for(i=0;i<=200;i++) {buf[i]=0;}

        if ((fmt1>=0) && (n<1))
        {       //formatted output if 0<=n<1
                mul=1;
                for(i=0;i<fmt2;i++)
                {n=n*10;mul=mul*10;}

                n=n*10;
                n=ceil(n);
                intdigit=floor(n);
                //intdigit=n;
                intdigit=(intdigit/10);

                pos=0;
                mul=mul/10;
                for(i=0;i<fmt2-1;i++)
                {
                        div=intdigit/mul;
                        buf[pos]=(char)div;
                        pos++;
                        intdigit=intdigit-div*mul;
                        mul=mul/10;
                        if (mul==1) break;
                }
                buf[pos]=(char)intdigit;
                *s='0';s++;
                *s='.';s++;
                for(i=0;i<format2;i++)
                {
                        if ((buf[i]>=0) && (buf[i]<=9)) {*s='0'+buf[i];}
                        else {*s='0';}
                        s++;
                }
        }
        else
        {       //if n>=1
                //v=floorf(n+0.00000000000001);
                beforpointdigit=floor(n+0.00000000000001);
		//beforpointdigit=n;
                nbefor=beforpointdigit;
                nafter=n-nbefor;

                //print part of number befor point
                mul=1;
                for(i=0;i<200-2;i++)
                {
                        mul=mul*10;
                        if ((beforpointdigit/mul)==0) {fmt1=i+1;break;}
                }

                pos=0;
                mul=mul/10;
                for(i=0;i<fmt1-1;i++)
                {
                        div=beforpointdigit/mul;
                        buf[pos]=(char)div;
                        pos++;
                        beforpointdigit=beforpointdigit-div*mul;
                        mul=mul/10;
                        if (mul==1) break;
                }
                buf[pos]=(char)beforpointdigit;

                for(i=0;i<fmt1;i++)
                {
                        if ((buf[i]>=0) && (buf[i]<=9)) {*s='0'+buf[i];}
                        s++;
                }

                //print part of number after point
                mul=1;
                for(i=0;i<fmt2;i++)
                {nafter=nafter*10;mul=mul*10;}

                nafter=nafter*10;
                nafter=ceil(nafter);
                intdigit=floor(nafter);
                //intdigit=nafter;
                intdigit=intdigit/10;

                pos=0;
                mul=mul/10;
                for(i=0;i<fmt2-1;i++)
                {
                        div=intdigit/mul;
                        buf[pos]=(char)div;
                        pos++;
                        intdigit=intdigit-div*mul;
                        mul=mul/10;
                        if (mul==1) break;
                }
                buf[pos]=(char)intdigit;
                *s='.';s++;
                for(i=0;i<format2;i++)
                {
                        if ((buf[i]>=0) && (buf[i]<=9)) {*s='0'+buf[i];}
                        else {*s='0';}
                        s++;
                }

        }
        size=(int)s-size;
        return(size);
}

int formatted_long_to_string(long long number,int fmt1,char *s)
{
        int             i;
        int             pos;
        int             fmt;
        int             size;
        int             difference_pos;
        long    digit;
        long    mul;
        long    div;
        char            buf[200];

        //clear array befor output
        for(i=0;i<200;i++) {buf[i]=0;}
        digit=number;

        size=(int)s;
        if (digit<0) {*s='-';s++;digit=-digit;}
        if (digit==0) {*s='0';s++;goto end;}

        mul=1;
                for(i=0;i<200-2;i++)
                {
                        mul=mul*10;
                        if ((digit/mul)==0) {fmt=i+1;break;}
                }

                difference_pos=i+1;

                pos=0;
                mul=mul/10;
                for(i=0;i<fmt-1;i++)
                {
                        div=digit/mul;
                        buf[pos]=(char)div;
                        pos++;
                        digit=digit-div*mul;
                        mul=mul/10;
                        if (mul==1) break;
                }
                buf[pos]=(char)digit;

                if (fmt1>=difference_pos) fmt=fmt1;
                else
                  fmt=difference_pos;

                for(i=0;i<fmt;i++)
                {
                        if (i<difference_pos)
                        {
                                if ((buf[i]>=0) && (buf[i]<=9)) {*s='0'+buf[i];}
                        }
                        else
                        {
                                *s=' ';
                        }
                        s++;
                }
        end:
        size=(int)s-size;
        return(size);
}

int formatted_hex_to_string(long long number,int fmt1,char flag_register,char *s)
{
        long    n;
        int             i,pos;
        int             fmt;
        long    size;
        int             difference_pos;
        char            xdigs_lower[16]="0123456789abcdef";
        char            xdigs_upper[16]="0123456789ABCDEF";
        char            buf[200];

        n=(long)number;
        size=(int)s;
        if (n<0) {*s='-';s++;n=-n;}

        if (n==0) {*s='0';s++;goto end;}
        for(i=0;i<200;i++) {buf[i]=0;}

        i=0;
        if (flag_register==0)
        {
                while (n>0)
                {
                        buf[i]=xdigs_lower[n & 15];
                        n=n>>4;
                        i++;
                }
        }
        else
        {
                while (n>0)
                {
                        buf[i]=xdigs_upper[n & 15];
                        n=n>>4;
                        i++;
                }
        }

        pos=i;
        difference_pos=i;

        for(i=pos-1;i>=0;i--)
        {
                *s=buf[i];
                s++;
        }

        if (fmt1-difference_pos>0)
        {
                for(i=difference_pos+1;i<=fmt1;i++)
                {
                        *s=' ';
                        s++;
                }
        }
        end:size=(int)s-size;
        return(size);
}

int formatted_octa_to_string(long long number,int fmt1,char flag_register,char *s)
{
        long    n;
        int             i,pos;
        int             fmt;
        long   	size;
        int             difference_pos;
        char            xdigs_lower[16]="012345678";
        char            buf[200];

        n=number;
        size=(int)s;
        if (n<0) {*s='-';s++;n=-n;}

        if (n==0) {*s='0';s++;goto end;}
        for(i=0;i<200;i++) {buf[i]=0;}

        i=0;
        if (flag_register==0)
        {
                while (n>0)
                {
                        buf[i]=xdigs_lower[n & 7];
                        n=n>>3;
                        i++;
                }
        }

        pos=i;
        difference_pos=i;

        for(i=pos-1;i>=0;i--)
        {
                *s=buf[i];
                s++;
        }

        if (fmt1-difference_pos>0)
        {
                for(i=difference_pos+1;i<=fmt1;i++)
                {
                        *s=' ';
                        s++;
                }
        }
        end:size=(int)s-size;
        return(size);
}

int format_print(char *dest, size_t maxlen,const char *fmt0, va_list argp)
{
        int                     i,j,k;
        int                     length;
        int                     fmt1,fmt2,stepen;
        size_t                  pos,posc;
        long	long	        intdigit;
        long	double          doubledigit;
        float                   floatdigit;
        const   char            *fmt,*fmtc;
        char                    *s;
        char                    *str;
        char                    buffmt1[30];
        char                    buffmt2[30];
        char                    buf[1024];
        char                    format_flag;
        char                    flag_point;
        char                    flag_noformat;
        char                    flag_long;
        char                    flag_unsigned;
        char                    flag_register;
        char                    flag_plus;

        fmt=fmt0;
        s=dest;
        pos=0;
        while(pos<maxlen)
        {
                if (*fmt=='%')
                {

                        if (*(fmt+1)=='%')
                        {
                                *s='%';
                                s++;
                                fmt=fmt+2;
                                pos++;
                                goto exit_check;
                        }
                        //checking to containg format in the string
                        fmtc=fmt;
                        posc=pos;
                        format_flag=0;
                        flag_long=0;
                        flag_unsigned=0;
                        flag_register=0;
                        flag_plus=0;
                        while((*fmtc!='\0') || (*fmtc!=0))
                        {
                                fmtc++;
                                posc++;
                                switch(*fmtc)
                                {
                                        case 'c':
                                        case 'C':
                                        format_flag=1;
                                        break;
                                        case 'd':
                                        case 'D':
                                        case 'i':
                                        case 'I':
                                        format_flag=1;
                                        break;
                                        case 'e':
                                        format_flag=1;
                                        break;
                                        case 'E':
                                        format_flag=1;
                                        flag_long=1;
                                        break;
                                        case 'f':
                                        format_flag=1;
                                        break;
                                        case 'F':
                                        format_flag=1;
                                        flag_long=1;
                                        break;
                                        case 'g':
                                        format_flag=1;
                                        break;
                                        case 'G':
                                        format_flag=1;
                                        flag_long=1;
                                        break;
                                        case 'l':
                                        flag_long=1;
                                        break;
                                        case 'L':
                                        flag_long=2;
                                        break;
                                        case 'o':
                                        format_flag=1;
                                        break;
                                        case 's':
                                        case 'S':
                                        format_flag=1;
                                        break;
                                        case 'u':
                                        case 'U':
                                        format_flag=1;
                                        flag_unsigned=1;
                                        break;
                                        case 'x':
                                        format_flag=1;
                                        break;
                                        case 'X':
                                        flag_register=1;
                                        format_flag=1;
                                        break;
                                        case 'z':
                                        case 'Z':
                                        format_flag=1;
                                        flag_unsigned=1;
                                        break;
                                        case '+':
                                        flag_plus=1;
                                        break;

                                        default:;
                                }
                                if ((*fmtc=='%') || (*fmtc==' ')) break;
                                if (format_flag==1) break;
                        }

                        if (format_flag==0)
                        {
                                *s=*fmt;
                                fmt++;
                                s++;
                                pos++;
                        }
                        else
                        {
                                if ((posc-pos)==1)
                                {//simbols % and format simbol near tothere(for example %c )
                                        fmt=fmtc+1;
                                        switch(*fmtc)
                                        {
                                                case 'c':
                                                case 'C':
                                                if ((pos+1)<maxlen)
                                                {
                                                        //*s=(int)va_arg(argp,char*);
                                                        *s=*((char *)argp);
                                                        argp=argp+4;
                                                        *s++;pos++;
                                                }
                                                break;
                                                case 's':
                                                case 'S':
                                                str=va_arg(argp,char*);
                                                length=strlen(str);
                                                if ((pos+length)<maxlen)
                                                {
                                                        memcpy(s,str,length);
                                                        s=s+length;pos=pos+length;
                                                }
                                                break;
                                                case 'd':
                                                case 'D':
                                                case 'i':
                                                case 'I':
                                                if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                if (flag_long==1) {intdigit=va_arg(argp,long);}
                                                if (flag_long==2) {intdigit=va_arg(argp,long long);}
						//intdigit=*((long*)argp);
                                                //argp=argp+4;
                                                if ((intdigit>0) && (flag_plus==1) && (pos+1<maxlen))
                                                {
                                                        *s='+';
                                                        s++;
                                                        pos++;
                                                }
                                                length=formatted_long_to_string(intdigit,0,buf);
                                                if ((pos+length)<maxlen)
                                                {
                                                        memcpy(s,buf,length);
                                                        s=s+length;pos=pos+length;
                                                }
						break;
                                                case 'o':
                                                if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                if (flag_long==1) {intdigit=va_arg(argp,long);}
                                                if (flag_long==2) {intdigit=va_arg(argp,long long);}
                                                //intdigit=*((long int *)argp);
                                                //argp=argp+4;

                                                length=formatted_octa_to_string(intdigit,0,flag_register,buf);
                                                if ((pos+length)<maxlen)
                                                {
                                                        memcpy(s,buf,length);
                                                        s=s+length;pos=pos+length;
                                                }
                                                break;
                                                case 'u':
                                                case 'U':
                                                if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                if (flag_long==1) {intdigit=va_arg(argp,long int);}
                                                if (flag_long==2) {intdigit=va_arg(argp,long long);}

                                                if (flag_unsigned==1) {
                                                        if (intdigit<0) {intdigit=-intdigit;}
                                                }

                                                length=formatted_long_to_string(intdigit,0,buf);
                                                if ((pos+length)<maxlen)
                                                {
                                                        memcpy(s,buf,length);
                                                        s=s+length;pos=pos+length;
                                                }
                                                break;
                                                case 'x':
                                                case 'X':
                                                if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                if (flag_long==1) {intdigit=va_arg(argp,long);}
                                                if (flag_long==2) {intdigit=va_arg(argp,long long);}
                                                //intdigit=*((long int *)argp);
                                                //argp=argp+4;

                                                length=formatted_hex_to_string(intdigit,0,flag_register,buf);
                                                if ((pos+length)<maxlen)
                                                {
                                                        memcpy(s,buf,length);
                                                        s=s+length;pos=pos+length;
                                                }
                                                break;
                                                case 'z':
                                                case 'Z':
                                                intdigit=va_arg(argp,size_t);

                                                if (flag_unsigned==1) {
                                                        if (intdigit<0) {intdigit=-intdigit;}
                                                }

                                                length=formatted_long_to_string(intdigit,0,buf);
                                                if ((pos+length)<maxlen)
                                                {
                                                        memcpy(s,buf,length);
                                                        s=s+length;pos=pos+length;
                                                }
                                                break;
                                                default:;

                                        }
                                }
                                else
                                {
                                        fmt++;
                                        flag_point=0;
                                        flag_noformat=0;
                                        fmt1=0;
                                        fmt2=0;
                                        j=0;
                                        k=0;
                                        for(i=pos+1;i<posc;i++)
                                        {
                                                switch(*fmt)
                                                {
                                                        case '0':
                                                        case '1':
                                                        case '2':
                                                        case '3':
                                                        case '4':
                                                        case '5':
                                                        case '6':
                                                        case '7':
                                                        case '8':
                                                        case '9':
                                                        if (flag_point==0)
                                                        {
                                                                buffmt1[j]=*fmt-'0';
                                                                j++;
                                                        }
                                                        else
                                                        {
                                                                buffmt2[k]=*fmt-'0';
                                                                k++;
                                                        }
                                                        break;
                                                        case '.':
                                                        flag_point=1;
                                                        break;
                                                        case 'l':
                                                        case 'L':
                                                        break;
                                                        case '+':
                                                        break;
                                                        default:flag_noformat=1;
                                                }
                                                if (flag_noformat==1) break;
                                                fmt++;
                                        }
                                        if (flag_noformat==0)
                                        {
                                                stepen=1;
                                                for(i=j-1;i>=0;i--)
                                                {
                                                        fmt1=fmt1+buffmt1[i]*stepen;
                                                        stepen=stepen*10;
                                                }
                                                stepen=1;
                                                for(i=k-1;i>=0;i--)
                                                {
                                                        fmt2=fmt2+buffmt2[i]*stepen;
                                                        stepen=stepen*10;
                                                }
                                                switch(*fmtc)
                                                {
                                                        case 'f':
                                                        case 'F':
                                                        if (flag_long==0) {doubledigit=va_arg(argp,double);}
                                                        if (flag_long>=1) {doubledigit=va_arg(argp,long double);}
                                                        //doubledigit=*((double *)argp);
                                                        //sargp=argp+8;
                                                        length=formatted_double_to_string(doubledigit,fmt1,fmt2,buf);
                                                        if ((pos+length)<maxlen)
                                                        {
                                                                memcpy(s,buf,length);
                                                                s=s+length;pos=pos+length;
                                                        }
                                                        break;
                                                        case 'd':
                                                        case 'D':
                                                        case 'i':
                                                        case 'I':
                                                        if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                        if (flag_long==1) {intdigit=va_arg(argp,long);}
                                                        if (flag_long==2) {intdigit=va_arg(argp,long long);}

                                                        if ((intdigit>0) && (flag_plus==1) && (pos+1<maxlen))
                                                        {
                                                                *s='+';
                                                                s++;
                                                                pos++;
                                                        }
                                                        length=formatted_long_to_string(intdigit,fmt1,buf);
                                                        if ((pos+length)<maxlen)
                                                        {
                                                                memcpy(s,buf,length);
                                                                s=s+length;pos=pos+length;
                                                        }
                                                        break;
                                                        case 'o':
                                                        if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                        if (flag_long==1) {intdigit=va_arg(argp,long);}
                                                        if (flag_long==2) {intdigit=va_arg(argp,long long);}
                                                        length=formatted_octa_to_string(intdigit,fmt1,flag_register,buf);
                                                        if ((pos+length)<maxlen)
                                                        {
                                                                memcpy(s,buf,length);
                                                                s=s+length;pos=pos+length;
                                                        }
                                                        break;
                                                        case 'u':
                                                        case 'U':
                                                        if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                        if (flag_long==1) {intdigit=va_arg(argp,long int);}
                                                        if (flag_long==2) {intdigit=va_arg(argp,long long);}

                                                        if (flag_unsigned==1) {
                                                                if (intdigit<0) {intdigit=-intdigit;}
                                                        }

                                                        length=formatted_long_to_string(intdigit,fmt1,buf);
                                                        if ((pos+length)<maxlen)
                                                        {
                                                                memcpy(s,buf,length);
                                                                s=s+length;pos=pos+length;
                                                        }
                                                        break;
                                                        case 'x':
                                                        case 'X':
                                                        if (flag_long==0) {intdigit=va_arg(argp,int);}
                                                        if (flag_long==1) {intdigit=va_arg(argp,long int);}
                                                        if (flag_long==2) {intdigit=va_arg(argp,long long);}
                                                        length=formatted_hex_to_string(intdigit,fmt1,flag_register,buf);
                                                        if ((pos+length)<maxlen)
                                                        {
                                                                memcpy(s,buf,length);
                                                                s=s+length;pos=pos+length;
                                                        }
                                                        break;
                                                        case 'z':
                                                        case 'Z':
                                                        intdigit=va_arg(argp,size_t);

                                                        if (flag_unsigned==1) {
                                                                if (intdigit<0) {intdigit=-intdigit;}
                                                        }

                                                        length=formatted_long_to_string(intdigit,fmt1,buf);
                                                        if ((pos+length)<maxlen)
                                                        {
                                                                memcpy(s,buf,length);
                                                                s=s+length;pos=pos+length;
                                                        }
                                                        break;
                                                        default:;
                                                }
                                        }
                                        fmt=fmtc+1;
                                }
                        }
                }
                else
                {
                        if (*fmt=='\0') {break;}
                        *s=*fmt;
                        fmt++;
                        s++;
                        pos++;
                }
                exit_check:;
        }
        return(pos);
}

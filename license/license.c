//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef SECRET_HASH
#define SECRET_HASH "TestHash"
#endif

/** Used to convert from binary to 32 base (I, L and O removed).
 */
static char Base[34]="0123456789ABCDEFGHJKMNPQRSTUVWXYZ";

/* File included in TOra binary to get this hash with the following define set
 */

#ifndef TO_LICENSE
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdllib.h>

/* MD5 stuff from RSA
 */

#include "global.h"
#include "md5.h"

int main(int argc, char *argv[])
{
   MD5_CTX MD5;
   unsigned char output[16];
   int i;

   char buffer[1024];
   char username[256];
   int userlen=0;

   int licenses;

   if (argc!=3) {
     printf("Usage:\n\n  licgen {Name} {Number of licenses}\n\n");
     exit(2);
   }

   /** Just some garbage to make licenses different if same name.
    */

   time_t tim=time(NULL);

   /* Assume licenses in argv[2]
    */

   if (argc==3) {
     licenses=atoi(argv[2]);
     if (licenses<0)
       exit(2);
     else if (licenses>=200)
       licenses=1000; /* Unlimited licenses */
   } else if (argc==2)
     licenses=0;
   else
     exit(3);

   /* Strip everything not alphanumeric in name. Don't use isalnum to avoid possible LANG
    * problems.
    */

   for(i=0;i<255&&argv[1][i];i++) {
     char c=toupper(argv[1][i]);
     if ((c>='A'&&c<='Z')||(c>='0'&&c<='9')) {
       username[userlen]=c;
       userlen++;
     }
   }

   /* Contains some information like the version this license was issued for, part of a
    * a timestamp so that different keys are generated for same name and license number.
    * Number of licenses. Is prepended the MD5 checksum in the license.
    */
   sprintf(buffer,
	   "12%c%c%c%c",
	   Base[tim&31],
	   Base[(tim>>5)&31],
	   Base[(licenses>>5)&31],
	   Base[licenses&31]);

   /* Calculate hash sum value of secret value and username in strip argument concatenated.
    */
   MD5Init(&MD5);
   MD5Update(&MD5,SECRET_HASH,strlen(SECRET_HASH));
   MD5Update(&MD5,buffer,strlen(buffer));
   MD5Update(&MD5,username,userlen);
   MD5Final(output,&MD5);
   
   printf("12%c%c-%c%c",
	  Base[tim&31],
	  Base[(tim>>5)&31],
	  Base[(licenses>>5)&31],
	  Base[licenses&31]);

   printf("%c%c",
	  Base[output[15]&31],
	  Base[(output[15]>>3)&31]);

   /* Output MD5 checksum in base 32 with a few characters left out.
    */
   for(i=0;i<3;i++) {
     printf("-%c%c%c%c-%c%c%c%c",
	    Base[output[i*5]&31],
	    Base[((output[i*5+0]>>5)+(output[i*5+1]<<3))&31],
	    Base[(output[i*5+1]>>2)&31],
	    Base[((output[i*5+1]>>7)+(output[i*5+2]<<1))&31],
	    Base[((output[i*5+2]>>4)+(output[i*5+3]<<4))&31],
	    Base[(output[i*5+3]>>1)&31],
	    Base[((output[i*5+3]>>6)+(output[i*5+4]<<2))&31],
	    Base[(output[i*5+4]>>3)&31]);
   }
   printf("\n");

   return 0;
};

#endif

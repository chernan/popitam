/*
 * This file is part of the Popitam software
 * Copyright (C) 2009 Swiss Institute of Bioinformatics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#define USE_POPIPARSER
#include "use.h"




int ReadInt(char* szBuffer)
{
    int ptSzBuffer = 0;
    if(szBuffer==NULL || strlen(szBuffer)==0) return 0;
    if(strchr(szBuffer, ':')==NULL) return 0;
    for( ;szBuffer[ptSzBuffer]!=':' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
    if(szBuffer[ptSzBuffer] !='\0') {
        ptSzBuffer+=2;//arrive après le : (il reste des espaces mais atoi ne s'en occupe pas)
        return atoi(szBuffer+ptSzBuffer);
    }
    return 0;
}

double ReadDouble(char* szBuffer)
{
    int ptSzBuffer = 0;
    if(szBuffer==NULL || strlen(szBuffer)==0) return 0.0;
    if(strchr(szBuffer, ':')==NULL) return 0.0;
    for( ;szBuffer[ptSzBuffer]!=':' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
    if(szBuffer[ptSzBuffer] !='\0') {
        ptSzBuffer+=2;//arrive après le : (il reste des espaces mais atof ne s'en occupe pas)
        return atof(szBuffer+ptSzBuffer);
    }
    return 0.0;
}

char* ReadString(char* szBuffer)
{
    int ptSzBuffer = 0;
    if(szBuffer==NULL || strlen(szBuffer)==0) return NULL;
    if(strchr(szBuffer, ':')==NULL) return NULL;
    for( ;szBuffer[ptSzBuffer]!=':' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
    if(szBuffer[ptSzBuffer] !='\0') {
        ptSzBuffer+=2;//arrive après le : 
        return Strdup(szBuffer+ptSzBuffer);
    }
    return NULL;
}

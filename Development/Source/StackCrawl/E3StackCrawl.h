#pragma once

/*  NAME:
        E3StackCrawl.h

    DESCRIPTION:
        Header file for E3StackCrawl.c.

    COPYRIGHT:
        Quesa Copyright � 1999-2002, Quesa Developers.
        
        For the list of Quesa Developers, and contact details, see:
        
            Documentation/contributors.html

        For the current version of Quesa, see:

        	<http://www.quesa.org/>

		This library is free software; you can redistribute it and/or
		modify it under the terms of the GNU Lesser General Public
		License as published by the Free Software Foundation; either
		version 2 of the License, or (at your option) any later version.

		This library is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
		Lesser General Public License for more details.

		You should have received a copy of the GNU Lesser General Public
		License along with this library; if not, write to the Free Software
		Foundation Inc, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
    ___________________________________________________________________________
*/
#ifndef E3STACKCRAWL_HDR
#define E3STACKCRAWL_HDR

//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#include "Quesa.h"


//=============================================================================
//		C++ preamble
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//      Types
//-----------------------------------------------------------------------------
typedef struct TQ3StackCrawlRec*	TQ3StackCrawl;


//=============================================================================
//      Function prototypes
//-----------------------------------------------------------------------------
TQ3StackCrawl	E3StackCrawl_New();
TQ3Uns32		E3StackCrawl_Count( TQ3StackCrawl inCrawl );
const char*		E3StackCrawl_Get( TQ3StackCrawl inCrawl, TQ3Uns32 inIndex );
void			E3StackCrawl_Dispose( TQ3StackCrawl inCrawl );


//=============================================================================
//		C++ postamble
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif
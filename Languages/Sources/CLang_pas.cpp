/*	$Id$
	
	Copyright 1996, 1997, 1998, 2002
	        Hekkelman Programmatuur B.V.  All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.
	3. All advertising materials mentioning features or use of this software
	   must display the following acknowledgement:
	   
	    This product includes software developed by Hekkelman Programmatuur B.V.
	
	4. The name of Hekkelman Programmatuur B.V. may not be used to endorse or
	   promote products derived from this software without specific prior
	   written permission.
	
	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
	AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 	

	Created: 12/07/97 22:01:11 by Maarten Hekkelman
	Modified from CLang_cpp.cpp for pascal by Kelvin W Sherlock
*/

#include "CLanguageAddOn.h"
#include <stack>
#include <ctype.h>

extern "C" {
_EXPORT const char kLanguageName[] = "Pascal";
_EXPORT const char kLanguageExtensions[] = "pas";
_EXPORT const char kLanguageCommentStart[] = "{";
_EXPORT const char kLanguageCommentEnd[] = "}";
_EXPORT const char kLanguageKeywordFile[] = "keywords.pas";
}

enum {
	START, 
	IDENT, OTHER, 
	COMMENT1, 		// { ... }
	COMMENT2, 		// (* ... *)
	STRING
};

#define GETCHAR			(c = (i++ < size) ? text[i - 1] : 0)

_EXPORT void ColorLine(CLanguageProxy& proxy, int& state)
{
	const char *text = proxy.Text();
	int size = proxy.Size();
	int i = 0, s = 0, kws, cc_cnt, esc = 0;
	char c;
	bool leave = false;
	
	if (state == COMMENT1 || state == COMMENT2)
		proxy.SetColor(0, kLCommentColor);
	else
		proxy.SetColor(0, kLTextColor);
	
	if (size <= 0)
		return;
	
	while (!leave)
	{
		GETCHAR;
		
		switch (state) {
			case START:

				if (isalpha(c) )
				{
					kws = proxy.Move(tolower(c), 1);
					state = IDENT;
				}
				else if (c == '{')
				{
					state=COMMENT1;	
				}
				else if (c == '(' && text[i] == '*')
				{
					i++;
					state = COMMENT2;
				}
				else if (c == '\'')
					state = STRING;

				else if (c == '\n' || c == 0)
					leave = true;
					
				if (leave || (state != START && s < i))
				{
					proxy.SetColor(s, kLTextColor);
					s = i - 1;
				}
				break;
			
			// {. .. } format comments
			case COMMENT1:
				if (c == '}')
				{
					proxy.SetColor(s, kLCommentColor);
					s = i;
					state = START;
				}
				else if (c == '\n' || c == 0)
				{
					proxy.SetColor(s, kLCommentColor);
					leave = true;	
				}
				break;
			// (* ... *) format comments
			case COMMENT2:
				if ((s == 0 || i > s + 1) && c == '*' && text[i] == ')')
				{
					proxy.SetColor(s - 1, kLCommentColor);
					s = i + 1;
					state = START;
				}
				else if (c == 0 || c == '\n')
				{
					proxy.SetColor(s - 1, kLCommentColor);
					leave = true;
				}
				break;

			
			case IDENT:
				if (!isalnum(c))
				{
					int kwc;

					if (i > s + 1 && (kwc = proxy.IsKeyWord(kws)) != 0)
					{
						switch (kwc)
						{
							case 1:	proxy.SetColor(s, kLKeyWordColor); break;
							case 2:	proxy.SetColor(s, kLUser1); break;
							case 3:	proxy.SetColor(s, kLUser2); break;
							case 4:	proxy.SetColor(s, kLUser3); break;
							case 5:	proxy.SetColor(s, kLUser4); break;
//							default:	ASSERT(false);
						}
					}
					else
					{
						proxy.SetColor(s, kLTextColor);
					}
					
					s = --i;
					state = START;
				}
				else if (kws)
					kws = proxy.Move((int)(unsigned char)tolower(c), kws);
				break;
			

			
			// ' ... ' '' indicates is how we escape a single quote, so we can ignore it.
			case STRING:
				if (c == '\'')
				{
					proxy.SetColor(s, kLStringColor);
					s = i;
					state = START;			
				}
				else if (c == '\n' || c == 0)
				{
					proxy.SetColor(s, kLTextColor);
					state = START;
					s = size;
					leave = true;	
				}
				break;
			
			default:	// error condition, gracefully leave the loop
				leave = true;
				break;
		}
	}
} /* ColorLine */

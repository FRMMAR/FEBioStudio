/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

// MathParser.cpp: implementation of the CMathParser class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdlib.h>
#include "MathParser.h"
#include "string.h"
#include "ctype.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMathParser::CMathParser()
{
	// add default constants to map
	m_table["pi"] = 3.1415926535897932385;
	m_table["e" ] = 2.7182818284590452354;
}

CMathParser::~CMathParser()
{

}

void CMathParser::set_variable(const char* szname, const double val)
{
	m_table[szname] = val;
}

double CMathParser::eval(const char* szexpr, int& ierr)
{
	m_szexpr = szexpr;

	m_nerrs = 0;

	// evaluate the expression
	double val = expr();

	ierr = m_nerrs;

	return val;
}

double CMathParser::expr()
{
	double left = term();

	for(;;)
		switch(curr_tok)
		{
		case PLUS:
			left += term();
			break;
		case MINUS:
			left -= term();
			break;
		default:
			return left;			
		}
}

double CMathParser::term()
{
	double left = power();

	for(;;)
		switch(curr_tok)
		{
		case MUL:
			left *= power();
			break;
		case DIV:
			if (double d=power())
			{
				left /= d;
				break;
			}
			else return error("divide by zero");
		default:
			return left;
		}
}

double CMathParser::power()
{
	double left = prim();

	for (;;)
		switch(curr_tok)
		{
		case POW:
			left = pow(left, prim());
			break;
		default:
			return left;
		}
}

double CMathParser::prim()
{
	get_token();

	switch (curr_tok)
	{
	case NUMBER:
		{
			double v = number_value;
			get_token();
			return v;
		}
	case NAME:
		{
			std::map<std::string, double>::iterator it = m_table.find(string_value);
			if (it != m_table.end())
			{
				double& v = m_table[string_value];
				get_token();
				return v;
			}
			else
			{
				// check for functions

				double (*fnc1)(double) = 0;
				double (*fnc2)(double, double) = 0;

				if (strcmp(string_value, "cos" )==0) fnc1 =   cos;
				if (strcmp(string_value, "sin" )==0) fnc1 =   sin;
				if (strcmp(string_value, "tan" )==0) fnc1 =   tan;
				if (strcmp(string_value, "ln"  )==0) fnc1 =   log;
				if (strcmp(string_value, "log" )==0) fnc1 = log10;
				if (strcmp(string_value, "sqrt")==0) fnc1 =  sqrt;
				if (strcmp(string_value, "exp" )==0) fnc1 =   exp;
				if (strcmp(string_value, "atan2") == 0) fnc2 = atan2;

				get_token();

				if (fnc1)
				{
					if (curr_tok != LP) return error("'(' expected");
					double v = fnc1( expr() );
					if (curr_tok != RP) return error("')' expected");
					get_token(); // eat ')'
					return v;
				}
				else if (fnc2)
				{
					if (curr_tok != LP) return error("'(' expected");
					double x = expr();
					if (curr_tok != COMMA) return error("',' expected");
					double y = expr();
					if (curr_tok != RP) return error("')' expected");
					get_token(); // eat ')'
					return fnc2(x, y);
				}
				else return error("unknown variable or function name");
			}
		}
	case MINUS:
		return -prim();
	case LP:
		{
			double e = expr();
			if (curr_tok != RP) return error("')' expected");
			get_token();	// eat ')'
			return e;
		}
	default:
		return error("primary expected");
	}
}

CMathParser::Token_value CMathParser::get_token()
{
	// remove leading whitespace
	while ((*m_szexpr==' ') || (*m_szexpr=='\t')) m_szexpr++;

	// get the first character
	char ch = *m_szexpr++;

	switch(ch)
	{
	case 0:
		return curr_tok = END;
	case '^':
	case '*':
	case '/':
	case '+':
	case '-':
	case '(':
	case ')':
	case ',':
		return curr_tok = Token_value(ch);
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '.':
		m_szexpr--;
		number_value = get_number();
		return curr_tok = NUMBER;
	default:
		if (isalpha(ch))
		{
			m_szexpr--;
			get_name(string_value);
			return curr_tok = NAME;
		}
		error("bad token");
		return curr_tok=PRINT;
	}
}

double CMathParser::error(const char* str)
{
	m_nerrs++;
	strcpy(m_szerr, str);
	return 1;
}

double CMathParser::get_number()
{	
	const char* ch = m_szexpr;
	while (isdigit(*ch) || (*ch=='.') || (*ch=='e') || (*ch=='E')) ch++;

	double val = atof(m_szexpr);
	m_szexpr = ch;

	return val;
}

void CMathParser::get_name(char* str)
{
	int n = 0;
	while (isalnum(*m_szexpr)) str[n++] = *m_szexpr++;
	str[n] = 0;
}


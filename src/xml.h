/***************************************************************************
 *   Copyright (C) 2004 by Roman Savochenko                                *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef XML_H
#define XML_H

#include <string>
#include <vector>

#include <expat.h>

using std::string;
using std::vector;

class XMLNode 
{
    public:
    	XMLNode( const string &name = "" ) : 
    	    m_root(NULL), m_current_node(NULL), m_name ( name ), m_text("") {  }
	~XMLNode() { clean(); }

	string 	name() const { return m_name; }
        void 	name( const string &s ) { m_name = s; }		

	string	text() const { return m_text; }
        void 	text( const string &s ) { m_text = s; }		
	
	void	attrList( vector<string> & list ) const;
        string 	attr( const string &name ) const;
        void 	attr( const string &name, const string &val, const bool add = true );		

	void 	load( const string & );	
	string 	save( bool humen = false ) const;
	void 	clean();

	int 	childSize() const { return m_children.size(); }
	void 	childAdd( XMLNode * );
	XMLNode* childAdd( const string &name = "" );
	int 	childIns( unsigned id, XMLNode * );
	XMLNode* childIns( unsigned id, const string &name = "" );
	void 	childDel( const unsigned id );
	void 	childClean( const string &name = "" );
	XMLNode* childGet( const int ) const;
	XMLNode* childGet( const string &name, const int numb = 0 ) const;	
	XMLNode* childGet( const string &attr, const string &name ) const;	

    private:
	string encode ( const string &s ) const;
	
	static void start_element ( void *data, const char *el, const char **attr );
	static void end_element ( void *data, const char *el );
	static void characters ( void *userData, const XML_Char *s, int len );
	
    private:	
	string m_name;
    	string m_text;
	vector<XMLNode *> m_children;
	vector<string>   n_attr;
	vector<string>   v_attr;	

	static const char *o_name;
	
    private:    
	vector<XMLNode*>& node_stack() { return m_node_stack; }
        XMLNode* current_node()        { return m_current_node; }
	void set_current_node ( XMLNode* p ) { m_current_node = p; }
	void set_root( XMLNode* p )    { if( m_root ) delete (m_root); m_root = p; }
        XMLNode* root()                { return m_root; }

    private:
        XMLNode *m_root;
    	vector<XMLNode*> m_node_stack;
        XMLNode* m_current_node;
};

#endif  //XML_H


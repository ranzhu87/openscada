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


#include <tsys.h>
#include <tmessage.h>
#include "tfunctions.h"

//List of function libraries
TFunctionS::TFunctionS(TKernel *app) : m_owner(app)
{
    m_lb = grpAdd();
    nodeEn();
}

TFunctionS::~TFunctionS()
{

}

string TFunctionS::name()
{
    return Mess->I18N("Functions");
}    

void TFunctionS::cntrCmd_( const string &a_path, XMLNode *opt, int cmd )
{
    if( cmd==TCntrNode::Info )
    {
	ctrMkNode("oscada_cntr",opt,a_path.c_str(),"/",Mess->I18N("Function subsystem"));
	ctrMkNode("area",opt,a_path.c_str(),"/lslib",Mess->I18N("Function's libraries"));
	ctrMkNode("list",opt,a_path.c_str(),"/lslib/lib",Mess->I18N("Libraries"),0444,0,0,"br")->
	    attr_("idm","1")->attr_("mode","att")->attr_("br_pref","_");	
    }
    else if( cmd==TCntrNode::Get )
    {
	if( a_path == "/lslib/lib" )
	{
	    vector<string> list_el;
	    list(list_el);
	    opt->childClean();
	    for( unsigned i_f=0; i_f < list_el.size(); i_f++ )
		ctrSetS( opt, at(list_el[i_f]).at().name(), list_el[i_f].c_str() );
      	}    	
	else throw TError("(Functions) Branch %s error",a_path.c_str());
    }
}

AutoHD<TCntrNode> TFunctionS::ctrAt1( const string &br )
{
    if( br.substr(0,1) == "_" )	return at(br.substr(1));
    else throw TError("(Functions) Branch %s error!",br.c_str());
}

//Function library abstract object
TLibFunc::TLibFunc( const string &iid ) : m_id(iid)
{
    m_fnc = grpAdd();
    nodeEn();
}

TLibFunc::~TLibFunc()
{

}

void TLibFunc::cntrCmd_( const string &a_path, XMLNode *opt, int cmd )
{
    if( cmd==TCntrNode::Info )
    {
	ctrMkNode("oscada_cntr",opt,a_path.c_str(),"/","Function's library: "+id());
	ctrMkNode("area",opt,a_path.c_str(),"/lib","Library");
	ctrMkNode("fld",opt,a_path.c_str(),"/lib/id","Id",0444,0,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/lib/name","Name",0444,0,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/lib/descr","Description",0444,0,0,"str")->
	    attr_("cols","90")->attr_("rows","4");
	ctrMkNode("area",opt,a_path.c_str(),"/func","Functions");	
	ctrMkNode("list",opt,a_path.c_str(),"/func/func","Functions",0444,0,0,"br")->
	    attr_("idm","1")->attr_("mode","att")->attr_("br_pref","_");
    }
    else if( cmd==TCntrNode::Get )
    {
	if( a_path == "/lib/id" )		ctrSetS( opt, id() );
	else if( a_path == "/lib/name" )	ctrSetS( opt, name() );
	else if( a_path == "/lib/descr" )	ctrSetS( opt, descr() );
	else if( a_path == "/func/func" )
	{
	    vector<string> list_el;
	    list(list_el);
	    opt->childClean();
	    for( unsigned i_f=0; i_f < list_el.size(); i_f++ )
		ctrSetS( opt, at(list_el[i_f]).at().name(), list_el[i_f].c_str() );
	}
	else throw TError("(LibFunc)Branch %s error",a_path.c_str());
    }    
}

AutoHD<TCntrNode> TLibFunc::ctrAt1( const string &br )
{
    if( br.substr(0,1) == "_" )	return at(pathEncode(br.substr(1),true));
    else throw TError("(LibFunc)Branch %s error",br.c_str());
}

//Function abstract object
TFunction::TFunction( const string &iid ) : m_id(iid), m_tval(NULL)
{

}

TFunction::~TFunction()
{
    for( int i_io = 0; i_io < m_io.size(); i_io++ )
	delete m_io[i_io];
}

int TFunction::ioSize()
{
    return m_io.size();
}

IO *TFunction::io( int id )
{    
    if( id >= m_io.size() || m_io[id] == NULL ) throw TError("Index broken!");
    return m_io[id];
}

int TFunction::ioId( const string &id )
{    
    for( int i_io = 0; i_io < m_io.size(); i_io++ )
	if( m_io[i_io]->id() == id ) return i_io;
    return -1;	
}

void TFunction::ioList( vector<string> &list )
{
    for( int i_io = 0; i_io < m_io.size(); i_io++ )
	list.push_back( m_io[i_io]->id() );
}

void TFunction::ioAdd( IO *io )
{
    m_io.push_back(io);
}

void TFunction::ioIns( IO *io, int pos )
{
    if( pos < 0 || pos > m_io.size() )	
	pos = m_io.size();
    m_io.insert(m_io.begin()+pos,io);
}

void TFunction::ioDel( int pos )
{
    if( pos < 0 || pos >= m_io.size() )
        throw TError("Delete position <%d> error.",pos);
    m_io.erase(m_io.begin()+pos);	
}

void TFunction::ioMove( int pos, int to )
{
    if( pos < 0 || pos >= m_io.size() || to < 0 || to >= m_io.size() )
	throw TError("Move parameters <%d:%d> error.",pos,to);
    IO *io = m_io[to];
    m_io[to] = m_io[pos];
    m_io[pos] = io;    	
}    

void TFunction::cntrCmd_( const string &a_path, XMLNode *opt, int cmd )
{
    if( cmd==TCntrNode::Info )
    {
	ctrMkNode("oscada_cntr",opt,a_path.c_str(),"/",Mess->I18N("Function: ")+id());
	ctrMkNode("area",opt,a_path.c_str(),"/func",Mess->I18N("Function"));
	ctrMkNode("fld",opt,a_path.c_str(),"/func/id",Mess->I18N("Id"),0444,0,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/func/name",Mess->I18N("Name"),0444,0,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/func/descr",Mess->I18N("Description"),0444,0,0,"str")->
	    attr_("cols","90")->attr_("rows","4");
	ctrMkNode("area",opt,a_path.c_str(),"/io",Mess->I18N("IO"));	
	ctrMkNode("table",opt,a_path.c_str(),"/io/io",Mess->I18N("IO"),0440,0,0);
	ctrMkNode("list",opt,a_path.c_str(),"/io/io/0",Mess->I18N("Id"),0440,0,0,"str");
	ctrMkNode("list",opt,a_path.c_str(),"/io/io/1",Mess->I18N("Name"),0440,0,0,"str");
	ctrMkNode("list",opt,a_path.c_str(),"/io/io/2",Mess->I18N("Type"),0440,0,0,"str");
	ctrMkNode("list",opt,a_path.c_str(),"/io/io/3",Mess->I18N("Mode"),0440,0,0,"str");
	ctrMkNode("list",opt,a_path.c_str(),"/io/io/4",Mess->I18N("Hide"),0440,0,0,"bool");
	ctrMkNode("list",opt,a_path.c_str(),"/io/io/5",Mess->I18N("Default"),0440,0,0,"str");
	ctrMkNode("list",opt,a_path.c_str(),"/io/io/6",Mess->I18N("Vector"),0440,0,0,"str");
	ctrMkNode("area",opt,a_path.c_str(),"/test",Mess->I18N("Test"));
	ctrMkNode("fld",opt,a_path.c_str(),"/test/en",Mess->I18N("Enable"),0660,0,0,"bool");
	//Add test form
	if( m_tval )
	{
	    ctrMkNode("area",opt,a_path.c_str(),"/test/io",Mess->I18N("IO"));
    	    //Put io
    	    for( int i_io = 0; i_io < ioSize(); i_io++ )
    	    {
		if( m_io[i_io]->hide() ) continue;
	    
		char *tp = "";
		if(io(i_io)->type() == IO::String)	tp = "str";
		else if(io(i_io)->type() == IO::Integer)tp = "dec";
		else if(io(i_io)->type() == IO::Real)	tp = "real";
		else if(io(i_io)->type() == IO::Boolean)tp = "bool";
		
		ctrMkNode("fld",opt,a_path.c_str(),("/test/io/"+io(i_io)->id()).c_str(),io(i_io)->name(),0664,0,0,tp);
	    }
	    //Add Calc button and Calc time
	    ctrMkNode("fld",opt,a_path.c_str(),"/test/tm",Mess->I18N("Calc time (mks)"),0444,0,0,"real");
	    ctrMkNode("comm",opt,a_path.c_str(),"/test/calc",Mess->I18N("Calc"));
	}
    }
    else if( cmd==TCntrNode::Get )
    {
	if( a_path == "/func/id" )		ctrSetS( opt, id() );
	else if( a_path == "/func/name" )	ctrSetS( opt, name() );
	else if( a_path == "/func/descr" )	ctrSetS( opt, descr() );
	else if( a_path == "/io/io" )
	{
	    XMLNode *n_id	= ctrId(opt,"0");
	    XMLNode *n_nm  	= ctrId(opt,"1");
	    XMLNode *n_type 	= ctrId(opt,"2");
	    XMLNode *n_mode 	= ctrId(opt,"3");
	    XMLNode *n_hide 	= ctrId(opt,"4");
	    XMLNode *n_def 	= ctrId(opt,"5");
	    XMLNode *n_vect	= ctrId(opt,"6");
	    for( int i_io = 0; i_io < ioSize(); i_io++ )
	    { 
	      	ctrSetS(n_id,io(i_io)->id());
		ctrSetS(n_nm,io(i_io)->name());
		//Make type
		if( io(i_io)->type() == IO::String )	ctrSetS(n_type,Mess->I18N("String"));
		else if( io(i_io)->type() == IO::Integer )	ctrSetS(n_type,Mess->I18N("Integer"));
		else if( io(i_io)->type() == IO::Real )	ctrSetS(n_type,Mess->I18N("Real"));
		else if( io(i_io)->type() == IO::Boolean )	ctrSetS(n_type,Mess->I18N("Bool"));
		else if( io(i_io)->type() == IO::Vector )	ctrSetS(n_type,Mess->I18N("Vector"));
		//Make mode
		if( io(i_io)->mode() == IO::Output )		ctrSetS(n_mode,Mess->I18N("Output"));
		else if( io(i_io)->mode() == IO::Return )	ctrSetS(n_mode,Mess->I18N("Return"));
		else if( io(i_io)->mode() == IO::Input )	ctrSetS(n_mode,Mess->I18N("Input"));
		
		if( io(i_io)->hide() )	ctrSetB(n_hide,true);
		else		        ctrSetB(n_hide,false);
		
		ctrSetS(n_def,io(i_io)->def());
		ctrSetS(n_vect,io(i_io)->vector());
	    }	
    	}
	else if( a_path == "/test/en" )	ctrSetB( opt, m_tval?true:false );    
	else if( m_tval && a_path == "/test/tm" )	ctrSetR( opt, m_tval->calcTm() );
	else if( m_tval && a_path.substr(0,8) == "/test/io" )
	{
	    for( int i_io = 0; i_io < m_io.size(); i_io++ )
		if( pathLev(a_path,2) == io(i_io)->id() )
		{
		    if(io(i_io)->type() == IO::String) 	ctrSetS( opt, m_tval->getS(i_io) );
		    else if(io(i_io)->type() == IO::Integer)	ctrSetI( opt, m_tval->getI(i_io) );
		    else if(io(i_io)->type() == IO::Real)	ctrSetR( opt, m_tval->getR(i_io) );
		    else if(io(i_io)->type() == IO::Boolean)  	ctrSetB( opt, m_tval->getB(i_io) );
		}    
	}
	else throw TError("(Function)Branch %s error",a_path.c_str());	
    }
    else if( cmd==TCntrNode::Set )
    {
	if( a_path == "/test/en" )
	{
	    if( ctrGetB( opt ) && !m_tval ) 
	    { 
		m_tval = new TValFunc(this); 
		m_tval->dimens(true); 
	    }
	    if( !ctrGetB( opt ) && m_tval ) { delete m_tval; m_tval = NULL; }
	}	
	else if( m_tval && a_path.substr(0,8) == "/test/io" )
	{
	    for( int i_io = 0; i_io < ioSize(); i_io++ )
		if( pathLev(a_path,2) == io(i_io)->id() )
		{
		    if(io(i_io)->type() == IO::String)	m_tval->setS(i_io, ctrGetS( opt ));
		    else if(io(i_io)->type() == IO::Integer)	m_tval->setI(i_io, ctrGetI( opt ));
		    else if(io(i_io)->type() == IO::Real)       m_tval->setR(i_io, ctrGetR( opt ));
		    else if(io(i_io)->type() == IO::Boolean)	m_tval->setB(i_io, ctrGetB( opt ));
		}
	}
	else if( m_tval && a_path == "/test/calc" )	m_tval->calc();
	else throw TError("(Function)Branch %s error",a_path.c_str());    
    }                   
}

IO::IO( const char *iid, const char *iname, IO::Type itype, IO::Mode imode, const char *idef, bool ihide, const char *ivect )
{
    m_id = iid;
    m_name = iname;
    m_type = itype;
    m_mode = imode;
    m_hide = ihide;
    m_def  = idef;
    m_vect = ivect;
}	

//TValFunc
TValFunc::TValFunc( TFunction *ifunc ) : m_func(NULL), m_dimens(false), tm_calc(0.0)
{   
    func(ifunc);
}

TValFunc::~TValFunc( )
{
    if( m_func ) funcDisConnect();
}

void TValFunc::func( TFunction *ifunc )
{
    if( m_func ) funcDisConnect();
    if( ifunc ) 
    {
	m_func = ifunc;
	for( int i_vl = 0; i_vl < m_func->ioSize(); i_vl++ )
	{
	    SVl val;
	    val.tp = m_func->io(i_vl)->type();
	    if( val.tp == IO::String ) 	val.vl = new string(m_func->io(i_vl)->def());
	    else if( val.tp == IO::Integer )	val.vl = new int(atoi(m_func->io(i_vl)->def().c_str()));
	    else if( val.tp == IO::Real ) 	val.vl = new double(atof(m_func->io(i_vl)->def().c_str()));
	    else if( val.tp == IO::Boolean )	val.vl = new bool(atoi(m_func->io(i_vl)->def().c_str()));
	    m_val.push_back(val);
	}
    }
}

void TValFunc::funcDisConnect( )
{
    if( m_func )
    {
	for( int i_vl = 0; i_vl < m_val.size(); i_vl++ )
	    if( m_val[i_vl].tp == IO::String )	delete (string *)m_val[i_vl].vl;
	    else if( m_val[i_vl].tp == IO::Integer )	delete (int *)m_val[i_vl].vl;
	    else if( m_val[i_vl].tp == IO::Real )	delete (double *)m_val[i_vl].vl;
	    else if( m_val[i_vl].tp == IO::Boolean )	delete (bool *)m_val[i_vl].vl;
	m_val.clear();    
	m_func = NULL;
    }
}

int TValFunc::ioId( const string &id )
{
    if( !m_func )	throw TError("Function no attached!");
    return m_func->ioId(id);
}

void TValFunc::ioList( vector<string> &list )
{
    if( !m_func )       throw TError("Function no attached!");
    return m_func->ioList(list);
}

void TValFunc::calc( )
{ 
    if( !m_func ) return;    
    if( !m_dimens ) m_func->calc(this);
    else
    {
	unsigned long long t_cnt = SYS->shrtCnt();
	m_func->calc(this); 
	tm_calc = 1.0e6*((double)(SYS->shrtCnt()-t_cnt))/((double)SYS->sysClk());
    }
}


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

#include <time.h>
#include <getopt.h>
#include <string>

#include <config.h>
#include <tsys.h>
#include <tkernel.h>
#include <tmessage.h>
#include <tsequrity.h>

#include "web_cfg.h"

//============ Modul info! =====================================================
#define MOD_ID      "web_cfg"
#define MOD_NAME    "System configurator (WEB)"
#define MOD_TYPE    "UI"
#define VER_TYPE    VER_UI
#define SUB_TYPE    "WWW"
#define MOD_VERSION "1.1.0"
#define AUTORS      "Roman Savochenko"
#define DESCRIPTION "Allow the WEB based OpenSCADA system configurator."
#define LICENSE     "GPL"
//==============================================================================

extern "C"
{
    TModule::SAt module( int n_mod )
    {
    	TModule::SAt AtMod;

	if(n_mod==0)
	{
	    AtMod.id	= MOD_ID;
	    AtMod.type  = MOD_TYPE;
    	    AtMod.t_ver = VER_TYPE;
	}
	else
	    AtMod.id	= "";

	return( AtMod );
    }

    TModule *attach( const TModule::SAt &AtMod, const string &source )
    {
	WebCfg::TWEB *self_addr = NULL;

	if( AtMod.id == MOD_ID && AtMod.type == MOD_TYPE && AtMod.t_ver == VER_TYPE )
	    self_addr = new WebCfg::TWEB( source );       

	return ( self_addr );
    }    
}

using namespace WebCfg;

//==============================================================================
//================= WebCfg::TWEB ===============================================
//==============================================================================
TWEB::TWEB( string name ) : m_t_auth(10)
{
    mId		= MOD_ID;
    mName       = MOD_NAME;
    mType	= MOD_TYPE;
    Vers	= MOD_VERSION;
    Autors	= AUTORS;
    DescrMod  	= DESCRIPTION;
    License   	= LICENSE;
    Source    	= name;

    //Reg export functions
    modFuncReg( new ExpFunc("void HttpGet(const string&,string&,const string&,vector<string>&);",
	"Process Get comand from http protocol's!",(void(TModule::*)( )) &TWEB::HttpGet) );
    modFuncReg( new ExpFunc("void HttpPost(const string&,string&,const string&,vector<string>&,const string&);",
        "Process Set comand from http protocol's!",(void(TModule::*)( )) &TWEB::HttpPost) );
    
    m_res = ResAlloc::resCreate( );
    
    // Init html headers
    w_head_ = "</html>\n";
    w_body_ = "<hr width=\"100%\" size=\"3\"/>\n"
    	"</body>\n";    
}

TWEB::~TWEB()
{
    ResAlloc res(m_res,true);
    while( m_auth.size() )
    { 
	delete m_auth[0]; 
	m_auth.erase(m_auth.begin()); 
    }
    res.release();
    
    ResAlloc::resDelete( m_res );
}

string TWEB::modInfo( const string &name )
{
    if( name == "SubType" ) return(SUB_TYPE);
    else return( TModule::modInfo( name) );
}

void TWEB::modInfo( vector<string> &list )
{
    TModule::modInfo(list);
    list.push_back("SubType");
}

string TWEB::optDescr( )
{
    char buf[STR_BUF_LEN];
    snprintf(buf,sizeof(buf),I18N(
        "======================= The module <%s:%s> options =======================\n"
        "---------- Parameters of the module section <%s> in config file ----------\n"
	"ses_t_life <time>      time of the sesion life, minets (default 10);\n\n"),
 	MOD_TYPE,MOD_ID,MOD_ID);

    return(buf);
}

void TWEB::modCheckCommandLine(  )
{
    int next_opt;
    char *short_opt="h";
    struct option long_opt[] =
    {
	{"help"    ,0,NULL,'h'},
	{NULL      ,0,NULL,0  }
    };

    optind=opterr=0;
    do
    {
	next_opt=getopt_long(SYS->argc,(char * const *)SYS->argv,short_opt,long_opt,NULL);
	switch(next_opt)
	{
	    case 'h': fprintf(stdout,optDescr().c_str()); break;
	    case -1 : break;
	}
    } while(next_opt != -1);
}

void TWEB::modUpdateOpt()
{
    try{ m_t_auth = atoi( modCfgNode()->childGet("ses_t_life")->text().c_str() ); }
    catch(...) {  }
}

string TWEB::w_ok( )
{
    return("HTTP/1.0 200 OK\nContent-type: text/html; charset="+Mess->charset()+"\n\n");
}

string TWEB::w_head( )
{
    return(
	"<?xml version='1.0' ?>\n"
        "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN'\n"
        "'DTD/xhtml1-transitional.dtd'>\n"
	"<html xmlns='http://www.w3.org/1999/xhtml'>\n<head>\n"    
	//"<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'\n"
        //"'http://www.w3.org/TR/html4/strict.dtd'>\n"
	//"<html>\n<head>\n"
    	"<meta http-equiv='Content-Type' content='text/html; charset="+Mess->charset()+"'/>\n"
    	"<title>"+PACKAGE_NAME+". "+I18N(MOD_NAME)+
	"</title>\n"
	"</head>\n");
}

string TWEB::w_body( )
{
    return(static_cast<string>("<body bgcolor='#818181' text='#000000' link='#3366ff' vlink='#339999' alink='#33ccff'>\n"
    	"<h1 align='center'><font color='#ffff00'>")+PACKAGE_NAME+". "+I18N(MOD_NAME)+"</font></h1>\n"
    	"<hr width='100%' size='3'/><br/>\n");
}

void TWEB::HttpGet( const string &urli, string &page, const string &sender, vector<string> &vars )
{
    SSess ses(url_encode(urli),page,sender,vars,"");
    
    ses.page = w_ok()+w_head()+w_body();    
    try
    {
	if( SYS->pathLev(ses.url,0) == "about" ) get_about(ses);
	else
	{
	    check_ses( ses );
	    if( !ses.user.size() ) get_auth( ses );
	    else
	    {
		SYS->cntrCmd(ses.url,&ses.root,TCntrNode::Info);
		get_head( ses );
		get_area( ses, ses.root, "/" );
	    }
	}
    }catch(TError err) 
    { post_mess(ses.page,err.what(),3); }
    
    down_colont( ses );
    ses.page = ses.page+w_body_+w_head_;
    page = ses.page;
}

void TWEB::get_about( SSess &ses )
{
    ses.page = ses.page+"<h1 align='center'><font color='White'><i>"+I18N("About")+"</i></font></h1>\n"
	"<table border='1' align='center'>\n"
	"<TR bgcolor='#8EE5EE' align='center'><TD>"+PACKAGE+" "+VERSION+"</TD></TR>\n"
	"<TR bgcolor='#cccccc'><TD>\n"
	"<table border='0'>\n"
	"<TR><TD><font color='Blue'>"+I18N("Name: ")+"</font></TD><TD>"+I18N("Open Supervisory Control And Data Acquisition")+"</TD></TR>\n"
	"<TR><TD><font color='Blue'>"+I18N("License: ")+"</font></TD><TD>GPL</TD></TR>\n"
	"<TR><TD><font color='Blue'>"+I18N("Autors: ")+"</font></TD><TD>Roman Savochenko</TD></TR>\n"
	"</table>\n"
	"</TD></TR></table><br/>\n"   
	"<table border='1' align='center'>\n"
	"<TR bgcolor='#8EE5EE' align='center'><TD>"+MOD_ID+" "+MOD_VERSION+"</TD></TR>\n"
	"<TR bgcolor='#cccccc'><TD>\n"
	"<table border='0'>\n"
	"<TR><TD><font color='Blue'>"+I18N("Name: ")+"</font></TD><TD>"+I18N(MOD_NAME)+"</TD></TR>"
	"<TR><TD><font color='Blue'>"+I18N("Desription: ")+"</font></TD><TD>"+I18N(DESCRIPTION)+"</TD></TR>"
	"<TR><TD><font color='Blue'>"+I18N("License: ")+"</font></TD><TD>"+I18N(LICENSE)+"</TD></TR>"
       	"<TR><TD><font color='Blue'>"+I18N("Autors: ")+"</font></TD><TD>"+I18N(AUTORS)+"</TD></TR>"
	"</table>\n"
        "</TD></TR>\n</table><br/>\n";             
}

void TWEB::get_head( SSess &ses )
{
    string path = string("/")+MOD_ID+ses.url;

    ses.page = ses.page+ "<table width='100%' align='center' border='1' bgcolor='#6495ED'><tr>\n"
	"<td width='10%' align='center' bgcolor='#cccccc' nowrap='nowrap'>\n"
	"<a href='/"+MOD_ID+"'>"+I18N("Root page")+"</a><br/>\n"
	"<a href='"+path+"'>"+I18N("Curent page")+"</a><br/>\n"
	"<a href='"+path.substr(0,path.rfind("/"))+"'>"+I18N("Previos page")+"</a><br/>\n"
	"<a href='/"+MOD_ID+"/about'>"+I18N("About")+"</a><br/>\n"
	"</td>\n"
	"<td align='center' bgcolor='#cccccc'><font size='+3'><b><i>"+ses.root.text()+"</i></b></font></td>\n"
	"<td width='120' align='left'";
    if(ses.user == "root") ses.page = ses.page + " bgcolor='red'";
    else                   ses.page = ses.page + " bgcolor='LawnGreen'";
    ses.page = ses.page+" nowrap='nowrap'>"+I18N("user:")+" <b>"+ses.user+"</b><br/>"+I18N("from:")+" <b>"+ses.sender+"</b>\n"
	"<form action='"+path+"' method='post' enctype='multipart/form-data'>\n"
	"<input name='auth_ch' type='submit' value='"+I18N("Change user")+"'/>\n"
	"</form></td>\n"
	"</tr></table><br/>\n";    
}

void TWEB::get_area( SSess &ses, XMLNode &node, string a_path )
{
    unsigned 	i_cf,c_cfg;    
    bool     	wr;
    string  	area_path;
    string 	path = string("/")+MOD_ID+ses.url;
    
    for( i_cf = 0; i_cf < node.childSize(); i_cf++)
    {
	XMLNode *t_s = node.childGet(i_cf);
	if( t_s->name() == "area" && chk_access(t_s, ses.user, SEQ_RD) )
	{
	    wr = false;
	    area_path = a_path+t_s->attr("id")+'/';
	    if( a_path == "/" )
	    	ses.page = ses.page+"<table width='100%' border='2' bgcolor='#A9A9A9' cellpadding='5'>\n"
	    	    "<tr bgcolor='#9999ff'><td><font size='+1'><b><i><u>"+t_s->attr("dscr")+
		    "</u></i></b></font><br/></td></tr>\n"
		    "<tr bgcolor='#CCCCCC'><td>";
	    else
		ses.page = ses.page+"<fieldset><legend>"+t_s->attr("dscr")+"</legend>\n";
	    
	    //ses.page = ses.page+"<tr bgcolor=#CCCCCC><td>";
	    //Get area's fields
	    c_cfg = 0;
	    bool f_open = false;
	    for( unsigned i_el = 0; i_el < t_s->childSize(); i_el++)
	    {
		XMLNode *t_c = t_s->childGet(i_el);
		if( !chk_access(t_c, ses.user, SEQ_RD) ) continue;
		if( t_c->name() == "fld" )
		{
		    // First element
		    if(c_cfg++ == 0)
		    {
			ses.page = ses.page+"<form action='"+path+"' method='post' enctype='multipart/form-data'>\n" 
			    "<table><tbody>\n";
		    }
		    // Start full element
		    if( t_c->attr("dscr").size() )
		    {
			if(f_open) ses.page = ses.page+"</td></tr>";
			ses.page = ses.page+"<tr><td valign='top'>"+t_c->attr("dscr")+":</td><td>";
			f_open = true;
		    }
    		    wr |= get_val(ses,*t_c,area_path+t_c->attr("id"));
		}		
	    }
	    if(f_open) ses.page = ses.page+"</td></tr>";
	    if(c_cfg > 0)
	    {
		ses.page = ses.page+"</tbody></table><br/>\n";
		if(wr) ses.page = ses.page+"<input type='submit' name='apply:"+area_path+"' value='"+I18N("Apply")+"'/>\n";    // Submit button 
		ses.page = ses.page+"</form>\n"
		    "<hr width='100%' size='1'/>\n";
	    }

	    //Get area's lists
	    c_cfg = 0;
	    for( unsigned i_el = 0; i_el < t_s->childSize(); i_el++)
	    {
		XMLNode *t_c = t_s->childGet(i_el);
		if( !chk_access(t_c, ses.user, SEQ_RD) ) continue;
		if( t_c->name() == "list" )
		{
		    c_cfg++;		    
		    ses.page = ses.page+t_c->attr("dscr")+":<br/>";
    		    get_val(ses,*t_c,area_path+t_c->attr("id"));
		}
	    }	    
	    if(c_cfg > 0) ses.page = ses.page+"<hr width='100%' size='1'/>\n";
	    
	    //Get area's tables
	    c_cfg = 0;
	    for( unsigned i_el = 0; i_el < t_s->childSize(); i_el++)
	    {
		XMLNode *t_c = t_s->childGet(i_el);
		if( !chk_access(t_c, ses.user, SEQ_RD) ) continue;
		if( t_c->name() == "table" )
		{
		    c_cfg++;		    
		    ses.page = ses.page+t_c->attr("dscr")+":<br/>";
    		    get_val(ses,*t_c,area_path+t_c->attr("id"));
		}
	    }	    
	    if(c_cfg > 0) ses.page = ses.page+"<hr width='100%' size='1'/>\n";
	    
            //Get area's commands
	    for( unsigned i_el = 0; i_el < t_s->childSize(); i_el++)
	    {
    		// First element
		XMLNode *t_c = t_s->childGet(i_el);
		if( !chk_access(t_c, ses.user, SEQ_RD) ) continue;
		if( t_c->name() == "comm" )
		    get_cmd(ses,*t_c,area_path+t_c->attr("id"));        
	    }	    
	    
    	    get_area( ses, *t_s, area_path);
	    if( a_path == "/" ) ses.page = ses.page+"</td></tr></table><br/>\n";
	    else	ses.page = ses.page+"</fieldset>\n";
	}
    }
}

void TWEB::get_cmd( SSess &ses, XMLNode &node, string a_path )
{
    string 	path = string("/")+MOD_ID+ses.url;
    
    ses.page = ses.page+"<form action='"+path+"' method='post' enctype='multipart/form-data'>\n" 
	"<input type='submit' name='comm:"+a_path+"' value='"+node.attr("dscr")+"'/>\n";    
    int f_cfg=0;
    for( unsigned i_el=0; i_el < node.childSize(); i_el++)
    {
	XMLNode *t_c = node.childGet(i_el);
	if( t_c->name() == "fld" )
	{
	    if(f_cfg++ > 0) ses.page = ses.page+", ";
	    else            ses.page = ses.page+"<br/>("; 
	    ses.page = ses.page+t_c->attr("dscr")+":";
	    get_val(ses,*t_c,a_path,false);
	}
    }
    if(f_cfg > 0) ses.page = ses.page+")<br/>\n";

    ses.page = ses.page+"</form>\n"; 
}

bool TWEB::get_val( SSess &ses, XMLNode &node, string a_path, bool rd )
{
    bool 	wr = false;
    string 	path = string("/")+MOD_ID+ses.url;
    
    if( chk_access(&node, ses.user, SEQ_WR) ) wr = true;
    if( node.name() == "fld" )
    {
	if( rd )
	{ 
	    try{ SYS->cntrCmd(ses.url+"/"+TCntrNode::pathCode( a_path, false ), &node, TCntrNode::Get); }
	    catch(TError err){ ses.mess.push_back( err.what() ); }
	}
 	if( node.attr("dest") == "select" && wr )
	{
	    //New select API	    
	    XMLNode x_lst("list");
	    
	    try{ SYS->cntrCmd(ses.url+"/"+TCntrNode::pathCode( node.attr("select"), false ), &x_lst, TCntrNode::Get); }
	    catch(TError err){ ses.mess.push_back( err.what() ); }
	    
	    ses.page = ses.page+ "<select name='"+node.attr("id")+"'>";
	    
            bool sel_ok = false;
            for( unsigned i_el = 0, c_el = 0; i_el < x_lst.childSize(); i_el++ )
        	if( x_lst.childGet(i_el)->name() == "el")
        	{
            	    bool ind_ok = x_lst.childGet(i_el)->attr("id").size();  //Index avoid
		    if( ind_ok )ses.page = ses.page+"<option value='"+x_lst.childGet(i_el)->attr("id")+"'";
		    else 	ses.page = ses.page+"<option value='"+x_lst.childGet(i_el)->text()+"'";
            	    if( (ind_ok && x_lst.childGet(i_el)->attr("id") == node.text()) || 
			(!ind_ok && x_lst.childGet(i_el)->text() == node.text()) )
                    {
                        sel_ok = true;
                        ses.page = ses.page+" selected='selected'";
                    }
                    ses.page = ses.page+">"+x_lst.childGet(i_el)->text()+"</option>";
        	}
            //Check no selected
            if( !sel_ok ) ses.page = ses.page+"<option value='' selected='selected'></option>";
	    ses.page = ses.page+ "</select>\n";
	}
	else
	{
	    if( node.attr("tp") == "bool" )
	    {
		ses.page = ses.page+"<input type='checkbox' name='"+node.attr("id")+"'";
		if( node.text() == "true" ) ses.page=ses.page+" checked='checked'";
		if( !wr ) ses.page=ses.page+" disabled='disabled'";
		ses.page = ses.page + "/>\n";
	    }
	    else
	    {
		// Draw text area 
		if( node.attr("tp") == "str" && (node.attr("rows").size() || node.attr("cols").size()) )
		{
		    ses.page = ses.page+"<textarea name='"+node.attr("id")+"' cols='"+node.attr("cols")+"' rows='"+node.attr("rows")+"'";
		    if( !wr ) ses.page=ses.page+" readonly='readonly'";
		    ses.page = ses.page+">"+mess2html(node.text())+"</textarea>\n";
		}
		else if( node.attr("tp") == "time" )
		{
		    struct tm *tm_tm;
		    time_t tm_t;
		    if( node.text().size() ) tm_t = strtol(node.text().c_str(),NULL,16);
		    else                         tm_t = time(NULL);
                    tm_tm = localtime(&tm_t);
		    if( !wr )
		    {
			char *c_tm = ctime( &tm_t );
			for( int i_ch = 0; i_ch < strlen(c_tm); i_ch++ )
			    if( c_tm[i_ch] == '\n' ) c_tm[i_ch] = '\0';
			ses.page=ses.page+"<b>"+c_tm+"</b>";
		    }
		    else
		    {
			string s_id = node.attr("id");		    
			ses.page = ses.page+"<input type='text' name='"+s_id+"_d' value='"+TSYS::int2str(tm_tm->tm_mday)+"' maxlength='2' size='2'/>\n";
			ses.page = ses.page+"<input type='text' name='"+s_id+"_ms' value='"+TSYS::int2str(tm_tm->tm_mon+1)+"' maxlength='2' size='2'/>\n";
			ses.page = ses.page+"<input type='text' name='"+s_id+"_y' value='"+TSYS::int2str(tm_tm->tm_year+1900)+"' maxlength='4' size='4'/>\n";
			ses.page = ses.page+ " , ";
			ses.page = ses.page+"<input type='text' name='"+s_id+"_h' value='"+TSYS::int2str(tm_tm->tm_hour)+"' maxlength='2' size='2'/>\n";
			ses.page = ses.page+"<input type='text' name='"+s_id+"_m' value='"+TSYS::int2str(tm_tm->tm_min)+"' maxlength='2' size='2'/>\n";
			ses.page = ses.page+"<input type='text' name='"+s_id+"_s' value='"+TSYS::int2str(tm_tm->tm_sec)+"' maxlength='2' size='2'/>\n";
		    }
		}		
		else
		{
		    // Other fields
		    if( !wr ) ses.page=ses.page+"<b>"+node.text()+"</b>";
		    else
		    {
			ses.page = ses.page+"<input type='text' name='"+node.attr("id")+"' value='"+mess2html(node.text())+"'";
			// addon parameters
			int val_n = atoi(node.attr("len").c_str());
			if( val_n > 0 ) ses.page = ses.page + " maxlength='"+TSYS::int2str(val_n)+"' size='"+TSYS::int2str((val_n>50)?50:val_n)+"'";
			else
			{
			    if( node.attr("tp") == "dec" || 
				 node.attr("tp") == "hex" || 
				 node.attr("tp") == "oct" ||
				 node.attr("tp") == "real" )
				ses.page = ses.page + " size='5'";
			    else ses.page = ses.page + " size='50'";
			}
			ses.page = ses.page + "/>\n";
		    }
		}
	    }
	}
    }
    else if( node.name() == "list" )
    {       
	if( node.attr("tp") == "br" || wr )
	    ses.page = ses.page+"<form action='"+path+"' method='post' enctype='multipart/form-data'>\n";
	    
	try{ SYS->cntrCmd(ses.url+"/"+SYS->pathCode( a_path, false ), &node, TCntrNode::Get); }
	catch(TError err){ ses.mess.push_back( err.what() ); }
	
	bool ind_m = atoi(node.attr("idm").c_str());
	
	int p_size = node.childSize();
	p_size = (p_size > 20)?20:(p_size < 4)?4:p_size;
	ses.page = ses.page+"<select name='"+node.attr("id")+"' size='"+TSYS::int2str(p_size)+"'>\n";
	for( unsigned i_lel = 0; i_lel < node.childSize(); i_lel++)
	{
	    XMLNode *t_c = node.childGet(i_lel);
	    if( t_c->name() == "el" )	    
		ses.page = ses.page+"<option value='"+TSYS::int2str(i_lel)+":"+mess2html(t_c->attr("id"))+":"+
			mess2html(t_c->text())+"'>"+mess2html(t_c->text())+"</option>\n";
	}
	ses.page = ses.page+"</select><br/>\n";
	if( node.attr("tp") == "br" )	
	    ses.page = ses.page+"<input type='submit' name='list:"+a_path+"/go' value='"+I18N("Go")+"'/><br/>\n";   //Go branch command
 	if( wr && node.attr("s_com").size() )
	{
	    bool p_edit = false;
	    ses.page = ses.page+"<br/>\n";
	    if( node.attr("s_com").find("add") != string::npos )
	    {
    		ses.page = ses.page+"<input type='submit' name='list:"+a_path+"/add' value='"+I18N("Add")+"'/>\n";    //Add element to list            
		p_edit = true;		
	    }
	    if( node.attr("s_com").find("ins") != string::npos )
	    {
    		ses.page = ses.page+"<input type='submit' name='list:"+a_path+"/ins' value='"+I18N("Insert")+"'/>\n"; //Insert element to list            
		p_edit = true;		
	    }
	    if( node.attr("s_com").find("edit") != string::npos )
	    {
		ses.page = ses.page+"<input type='submit' name='list:"+a_path+"/edit' value='"+I18N("Edit")+"'/>\n";   //Edit list element 
		p_edit = true;		
	    }	    
	    if( p_edit )
	    {
		if( ind_m ) 
		    ses.page = ses.page+"<input type='text' name='ener_id' value=''/>\n";
		node.name("fld");
		node.attr("id","ener_f");
		get_val( ses, node, node.attr("id"), false );
	    	ses.page = ses.page+"<br/>\n";
	    }
	    if( node.attr("s_com").find("move") != string::npos )
		ses.page = ses.page+
		    "<input type='submit' name='list:"+a_path+"/up' value='"+I18N("Up")+"'/>\n"
		    "<input type='submit' name='list:"+a_path+"/down' value='"+I18N("Down")+"'/>\n";			    	    
	    
	    if( node.attr("s_com").find("del") != string::npos )
		ses.page = ses.page+"<input type='submit' name='list:"+a_path+"/del' value='"+I18N("Del")+"'/>\n";    //Del element from list	    
	}
	if( node.attr("tp") == "br" || wr )
	    ses.page = ses.page+"</form>\n";
    }
    else if( node.name() == "table" )
    {
	if( wr ) ses.page = ses.page+"<form action='"+path+"' method='post' enctype='multipart/form-data'>\n";
	
	try{ SYS->cntrCmd(ses.url+"/"+SYS->pathCode( a_path, false ), &node, TCntrNode::Get); }
	catch(TError err){ ses.mess.push_back( err.what() ); }
	
	ses.page = ses.page + "<table border='1'>\n";
	int clm_sz[node.childSize()]; 
	//Draw head
	ses.page = ses.page + "<tr bgcolor='#FFA07A' align='center'>";
	for( int i_cl=0; i_cl < node.childSize(); i_cl++)
	{
	    ses.page = ses.page + "<td>"+mess2html(node.childGet(i_cl)->attr("dscr"))+"</td>";
	    //Calc column sizes
	    clm_sz[i_cl] = node.childGet(i_cl)->attr("dscr").size();
	    for( int i_rw=0; i_rw < node.childGet(i_cl)->childSize(); i_rw++)
	    {
		XMLNode *x_el  = node.childGet(i_cl)->childGet(i_rw);
		if( x_el->text().size() > clm_sz[i_cl] ) clm_sz[i_cl] = x_el->text().size();
	    }
	    if( clm_sz[i_cl] < 5 )	clm_sz[i_cl] = 5;
	    if( clm_sz[i_cl] > 40 )	clm_sz[i_cl] = 40;
	}
	if( wr ) ses.page = ses.page + "<td bgcolor='Green'>*</td>";
	ses.page = ses.page + "</tr>\n";
	
	for( int i_rw=0; i_rw < node.childGet(0)->childSize(); i_rw++)
	{
	    ses.page = ses.page + "<tr bgcolor='#cccccc'>";
	    for( int i_cl=0; i_cl < node.childSize(); i_cl++)
	    {			    
		XMLNode *x_el  = node.childGet(i_cl)->childGet(i_rw);
		if( node.childGet(i_cl)->attr("tp") == "time" )
		{
		    time_t tm_t = strtol(x_el->text().c_str(),NULL,16);
                    char *c_tm = ctime( &tm_t );
                    for( int i_ch = 0; i_ch < strlen(c_tm); i_ch++ )
                	if( c_tm[i_ch] == '\n' ) c_tm[i_ch] = '\0';
                    ses.page = ses.page+"<td nowrap='nowrap'>"+c_tm+"</td>";
		}
		else if( node.childGet(i_cl)->attr("tp") == "str" && wr && node.childGet(i_cl)->attr("select").size() )
                {
		    ses.page = ses.page+ "<td><select name='"+TSYS::int2str(i_rw)+":"+TSYS::int2str(i_cl)+"'>";
		
		    XMLNode x_lst("list");
		    bool sel_ok = false;
		    bool u_ind = atoi(node.childGet(i_cl)->attr("idm").c_str());
		    SYS->cntrCmd(ses.url+"/"+SYS->pathCode( node.childGet(i_cl)->attr("select"), false ), &x_lst, TCntrNode::Get);
		    for( int i_ls = 0; i_ls < x_lst.childSize(); i_ls++ )
		    {
			if( u_ind ) ses.page = ses.page+"<option value='"+x_lst.childGet(i_ls)->attr("id")+"'";
			else ses.page = ses.page+"<option value='"+x_lst.childGet(i_ls)->text()+"'";
			
                	if( (u_ind && x_lst.childGet(i_ls)->attr("id") == x_el->text()) ||
                            (!u_ind && x_lst.childGet(i_ls)->text() == x_el->text()) )
            		{
                    	    sel_ok = true;
                    	    ses.page = ses.page+" selected='selected'";
                	}
			ses.page = ses.page+">"+x_lst.childGet(i_ls)->text()+"</option>";			
		    }
		    if( !sel_ok ) ses.page = ses.page+"<option value='' selected='selected'></option>";
            	    ses.page = ses.page+ "</select></td>\n";
		}
		else if( node.childGet(i_cl)->attr("tp") == "bool" )
		{
		    ses.page = ses.page+"<td><input type='checkbox' name='"+TSYS::int2str(i_rw)+":"+TSYS::int2str(i_cl)+"'";
		    if( x_el->text() == "true" ) ses.page=ses.page+" checked='checked'";
		    if( !wr ) ses.page=ses.page+" disabled='disabled'";
		    ses.page = ses.page + "/></td>\n";
		}
		else
		{		
		    if( !wr ) ses.page = ses.page+"<td>"+mess2html(x_el->text())+"</td>";
		    else ses.page = ses.page+"<td><input type='text' name='"+TSYS::int2str(i_rw)+
			":"+TSYS::int2str(i_cl)+"' value='"+mess2html(x_el->text())+"' size='"+TSYS::int2str(clm_sz[i_cl])+"'/></td>";
		}
	    }
	    //Add modify buttons
	    if( wr ) ses.page = ses.page+"<td><input type='checkbox' name='row:"+TSYS::int2str(i_rw)+"'/></td>";
	    ses.page = ses.page + "</tr>\n";
	}
	ses.page = ses.page + "</table>\n";	
	
	if( wr )
	{
	    ses.page = ses.page+"<input type='submit' name='tbl:"+a_path+"/apply' value='"+I18N("Apply")+"'/><br/>";
            if( node.attr("s_com").find("add") != string::npos )
                ses.page = ses.page+"<input type='submit' name='tbl:"+a_path+"/add' value='"+I18N("Add row")+"'/>";
	    if( node.attr("s_com").find("ins") != string::npos )
                ses.page = ses.page+"<input type='submit' name='tbl:"+a_path+"/ins' value='"+I18N("Insert row")+"'/>";
	    if( node.attr("s_com").find("del") != string::npos )
                ses.page = ses.page+"<input type='submit' name='tbl:"+a_path+"/del' value='"+I18N("Delete row")+"'/>";
	    if( node.attr("s_com").find("move") != string::npos ) 
	    {
                ses.page = ses.page+"<input type='submit' name='tbl:"+a_path+"/up' value='"+I18N("Move up row")+"'/>";
                ses.page = ses.page+"<input type='submit' name='tbl:"+a_path+"/down' value='"+I18N("Move down row")+"'/>";
	    }	    
	    
	    ses.page = ses.page+"</form>\n";
	}
    }
    
    return(wr);
}

void TWEB::get_auth( SSess &ses )
{
    ses.page = ses.page+"<table border='2' width='40%' align='center'>"
	       "<tr bgcolor='#9999ff'><td><b>"+I18N("Enter to module")+"</b></td></tr>\n"
	       "<tr bgcolor='#cccccc'> <td align='center'>\n"
	       "<form method='post' action='/"+MOD_ID+ses.url+"' enctype='multipart/form-data'>\n"	       
	       "<table cellpadding='3'>\n"
	       "<tr><td><b>"+I18N("User name")+"</b></td><td><input type='text' name='user' size='20'/></td></tr>\n"
	       "<tr><td><b>"+I18N("Password")+"</b></td><td><input type='password' name='pass' size='20'/></td></tr>\n"
	       "<tr><td colspan='2' align='center'><input type='submit' name='auth_enter' value='"+I18N("Enter")+"'/>\n"
	       "<input type='reset' name='clean' value='"+I18N("Clean")+"'/>\n"
	       "</td></tr></table>\n</form>\n"
	       "</td></tr></table>\n";
}

void TWEB::HttpPost( const string &urli, string &page, const string &sender, vector<string> &vars, const string &contein )
{
    bool my = false, err = false;    
    int  kz;
    //string ses_user;
    
    SSess ses(url_encode(urli),page,sender,vars,contein);    
    
    ses.page = w_ok()+w_head()+w_body();
    try
    {
	cont_frm_data( ses );	//Parse contein
	// Check autentification POST request
	if( !my )
	{
	    kz = post_auth( ses );    
	    if( kz&0x01 ) my  = true;
	    if( kz&0x02 ) err = true;
	}
	// Check avoid sesion
	if( !my )
	{
	    check_ses( ses );
	    if( ses.user.size() )
	    {
		SYS->cntrCmd(ses.url,&ses.root,TCntrNode::Info);
		//Parse post category and path to area
		string prs_cat, prs_path;
	    	unsigned i_el;
		
		string prs_comm;
		for( i_el = 0; i_el < ses.cnt_names.size(); i_el++)
		{
		    prs_comm = ses.cnt_names[i_el];
		    string comm_t = TSYS::strSepParse(prs_comm,0,':');
		    if( comm_t == "apply" || comm_t == "comm" || comm_t == "list" || comm_t == "tbl" ) break;
		}
		if( i_el < ses.cnt_names.size() )		    
		{
		    //int psep = ses.cnt_names[i_el].find(":",0);
	    	    //prs_cat  = ses.cnt_names[i_el].substr(0,psep);
		    //prs_path = ses.cnt_names[i_el].substr(psep+1);
		    ses.cnt_names.erase(ses.cnt_names.begin()+i_el);
		    ses.cnt_vals.erase(ses.cnt_vals.begin()+i_el);
	    	}
		kz = post_area( ses, ses.root, prs_comm );		
		if( kz&0x01 ) my  = true;
		if( kz&0x02 ) err = true;
	    }
	    else 
	    { 
		get_auth( ses ); 
		my  = true; 
		err = true; 
	    }
	}
	//Request error
	if( !my )       post_mess(ses.page,"Post request broken!",3);
	else if( !err )
	{
	    SYS->cntrCmd(ses.url,&ses.root,TCntrNode::Info);
	    get_head( ses );
	    get_area( ses, ses.root, "/" );	    
	}
    }catch(TError err) 
    { post_mess(ses.page,err.what(),3); }
    
    down_colont( ses );
    ses.page = ses.page+w_body_+w_head_;
    page = ses.page;
}

//int TWEB::post_area( SSess &ses, XMLNode &node,  const string &prs_cat, const string &prs_path, int level )
int TWEB::post_area( SSess &ses, XMLNode &node, const string &prs_comm, int level )
{
    string prs_cat = TSYS::strSepParse(prs_comm,0,':');
    string prs_path = TSYS::strSepParse(prs_comm,1,':');
    
    if( !TCntrNode::pathLev(prs_path,level).size() ) return(0x00);
    try
    {	
	XMLNode *t_nd = TCntrNode::ctrId(&node, TCntrNode::pathLev(prs_path,level));
	if( prs_cat == "apply" && !TCntrNode::pathLev(prs_path,level+1).size() )
	    return( post_val(ses,*t_nd,prs_path) );
	else if( prs_cat == "comm" && !TCntrNode::pathLev(prs_path,level+1).size() )
    	    return( post_cmd(ses,*t_nd,prs_path));
    	else if( prs_cat == "list" && t_nd->name() == "list" )
	    return( post_list(ses,*t_nd,prs_path) );
    	else if( prs_cat == "tbl" && t_nd->name() == "table" )
	    return( post_table(ses,*t_nd,prs_path) );
	return( post_area( ses, *t_nd, prs_comm, ++level ));
    }
    catch(TError err)
    {
	post_mess(ses.page,err.what(),3);
	return(0x01|0x02);
    }	
}

int  TWEB::post_val( SSess &ses, XMLNode &node, string prs_path)
{
    // Free no changed elements (polimorfic problem fix)    
    for( unsigned i_cf = 0; i_cf < node.childSize(); i_cf++)
    {
	XMLNode *t_c = node.childGet(i_cf);
	if( (t_c->name() == "fld") && chk_access(t_c, ses.user, SEQ_RD|SEQ_WR) )
	{
	    if( t_c->attr("tp") != "bool" && !prepare_val( ses, *t_c,prs_path, true ) )
		for( unsigned i_cnt = 0; i_cnt < ses.cnt_names.size(); i_cnt++ )
		    if( ses.cnt_names[i_cnt] == t_c->attr("id") )
	    	    {
			ses.cnt_names.erase( ses.cnt_names.begin()+i_cnt );
			ses.cnt_vals.erase( ses.cnt_vals.begin()+i_cnt );
			break;
		    }
	}
    }    
    // Check and modify
    for( unsigned i_cf = 0; i_cf < node.childSize(); i_cf++)
    {
	XMLNode *t_c = node.childGet(i_cf);
	if( (t_c->name() == "fld") && chk_access(t_c, ses.user, SEQ_RD|SEQ_WR) )
	{
	    if( !prepare_val( ses, *t_c,prs_path, true ) ) 
		continue;
	    string kz = TCntrNode::ctrChk(t_c,true);
	    if( !kz.size() )
	    {
		try
		{ 	    
		    mPut("CONTROL",MESS_INFO,"%s| Change <%s:%s> to %s",
		    	ses.user.c_str(),
			t_c->attr("id").c_str(),
			t_c->attr("dscr").c_str(),
			t_c->text().c_str());
		    SYS->cntrCmd(ses.url+"/"+SYS->pathCode(prs_path+t_c->attr("id"), false ),t_c,TCntrNode::Set);
		    continue; 
		}
		catch(TError err)
		{
		    post_mess(ses.page,err.what(),3);	
		    return(0x01|0x02);
		}
	    }
	    else 
	    {
		post_mess(ses.page,kz,2);
		return(0x01|0x02);
	    }
	}
    }    
    return(0x01);
}

int TWEB::post_cmd( SSess &ses, XMLNode &node, string prs_path )
{ 
    // Prepare command options
    try
    { 	
	//Check link
	if( node.attr("tp") == "lnk" )
	{
	    SYS->cntrCmd(ses.url+"/"+TCntrNode::pathCode(prs_path,false), &node, TCntrNode::Get);
	    string url = string("/")+MOD_ID+"/"+url_code(node.text(),false);
	    
	    mPut("CONTROL",MESS_INFO,"%s| Go to link <%s>",ses.user.c_str(),url.c_str());
	    	    
	    ses.page = ses.page + "<meta http-equiv='Refresh' content='0; url="+url+"'>\n";
	    post_mess( ses.page, "Go to <"+url+"> !",1);
	    return( 0x01|0x02 );  //No error. That no draw curent page
	}    
        
	for( int i_prm = 0; i_prm < node.childSize(); i_prm++ )
	{
	    XMLNode *t_c = node.childGet(i_prm);
	    if( t_c->name() == "fld" )
	    prepare_val( ses, *t_c, "", false );
	    string kz = TCntrNode::ctrChk(t_c,true);
	    if( kz.size() ) throw TError(kz);
	}
		    
	mPut("CONTROL",MESS_INFO,"%s| Put command <%s:%s>",
	    ses.user.c_str(),
	    node.attr("id").c_str(),
	    node.attr("dscr").c_str());	
	SYS->cntrCmd(ses.url+"/"+SYS->pathCode(prs_path,false),&node,TCntrNode::Set);
	//return( 0x01 );
    }
    catch(TError err){ ses.mess.push_back( err.what() ); }
    //{
    	//post_mess(ses.page,err.what(),3);
	//return(0x01|0x02);
    //}
    return( 0x01 );
}

int TWEB::post_list( SSess &ses, XMLNode &node, string prs_path )
{ 
    string 	path = string("/")+MOD_ID+ses.url;
    string 	f_path, l_path, nd_nm, l_com, ener_f, ener_id;

    bool ind_m = atoi(node.attr("idm").c_str());    
    //Get command name and path
    int c_pos = 1;    
    while(true)
    {
	int t_pos = prs_path.find("/",c_pos);
	if( t_pos == string::npos )
	{
	    l_com = prs_path.substr(c_pos,t_pos-c_pos);
	    break;
	}
	l_path = l_path + "/" + prs_path.substr(c_pos,t_pos-c_pos);//+"/";
	c_pos = t_pos+1;	
    }
    f_path = ses.url+"/"+SYS->pathCode(l_path,false);
    //Get new element value
    
    for( int i_le = 0; i_le < ses.cnt_names.size(); i_le++ )
        if( ses.cnt_names[i_le] == "ener_id" )
        { ener_id = ses.cnt_vals[i_le]; break; }		    
    for( int i_le = 0; i_le < ses.cnt_names.size(); i_le++ )
	if( ses.cnt_names[i_le] == "ener_f" ) 
	{ ener_f = ses.cnt_vals[i_le]; break; }

    string i_pos, i_el, l_el;
    //Check selected item    
    if( l_com == "go" || l_com == "ins" || l_com == "edit" || 
	l_com == "del" || l_com == "up" || l_com == "down" )
    {
        for( int i_le = 0; i_le < ses.cnt_names.size(); i_le++ )
            if( ses.cnt_names[i_le] == node.attr("id") )
            {
                l_el = ses.cnt_vals[i_le];
                break;
            }
        if( !l_el.size() )
        {
	    ses.mess.push_back( "No select list element of <"+node.attr("dscr")+">!" );
	    return( 0x01 );
            //post_mess(ses.page,"No select list element for list <"+node.attr("dscr")+">!",2);
    	    //return( 0x01|0x02 );
        }
	i_pos = TSYS::strSepParse(l_el,0,':');
        i_el =  TSYS::strSepParse(l_el,1,':');
        l_el =  TSYS::strSepParse(l_el,2,':');
    }
    if( l_com == "go" )
    { 
        //Get branch prefix
        string br_pref;
        if( node.attr("br_pref").size() )	br_pref = node.attr("br_pref");
        else                                	br_pref = l_path;
	//Check branch type
	string url;
	//Check attach mode
	if( node.attr("mode") == "at")	url = path+"/s";
	else				url = path+"/d";
	//Check Index-list mode
	if( i_el.size() )	url.append(url_code(br_pref+TCntrNode::pathCode(i_el,true),true));
	else			url.append(url_code(br_pref+TCntrNode::pathCode(l_el,true),true));
	    
        ses.page = ses.page + "<meta http-equiv='Refresh' content='0; url="+url+"'>\n";
	post_mess( ses.page, "Go to <"+url+"> !",1);
	return( 0x01|0x02 );  //No error. That no draw curent page
    }
    
    XMLNode n_el1;
    if( l_com == "del" )
    {
	n_el1.name("del");
	n_el1.attr("pos",i_pos);
	if( ind_m ) n_el1.attr("id",i_el);
	n_el1.text(l_el);
	
	Mess->put("WEB_CONTROL",MESS_INFO,"%s| Delete <%s> element <%s:%s>.",
             ses.user.c_str(), f_path.c_str(), i_el.c_str(), l_el.c_str());
    }    
    else if( l_com == "ins" )
    {
	n_el1.name("ins");
	n_el1.attr("pos",i_pos);
	if( ind_m ) n_el1.attr("id",ener_id);
	n_el1.text(ener_f);
	
	Mess->put("WEB_CONTROL",MESS_INFO,"%s| Insert <%s> element <%s:%s> to %s.",
             ses.user.c_str(), f_path.c_str(), ener_id.c_str(), ener_f.c_str(), i_pos.c_str());
    }
    else if( l_com == "add" )
    {
	n_el1.name("add");
	if( ind_m ) n_el1.attr("id",ener_id);
	n_el1.text(ener_f);
	
	Mess->put("WEB_CONTROL",MESS_INFO,"%s| Add <%s> element <%s:%s>.",
             ses.user.c_str(), f_path.c_str(), ener_id.c_str(), ener_f.c_str());
    }
    else if( l_com == "edit" )
    {
	n_el1.name("edit");
	n_el1.attr("pos",i_pos);
	if( ind_m ) n_el1.attr("id",ener_id);
	n_el1.text(ener_f);
	
	Mess->put("WEB_CONTROL",MESS_INFO,"%s| Set <%s> element %s to <%s:%s>.",
             ses.user.c_str(), f_path.c_str(), i_pos.c_str(), ener_id.c_str(), ener_f.c_str());
    }
    else if( l_com == "up" || l_com == "down" )
    {
	string i_pos_to;
	if( l_com == "down" )	i_pos_to = TSYS::int2str(atoi(i_pos.c_str())-1);
	else 			i_pos_to = TSYS::int2str(atoi(i_pos.c_str())+1);
	n_el1.name("move");
	n_el1.attr("pos",i_pos);
	n_el1.attr("to",i_pos_to);
	
	Mess->put("WEB_CONTROL",MESS_INFO,"%s| Move <%s> from %s to %s.",
             ses.user.c_str(), f_path.c_str(), i_pos.c_str(), i_pos_to.c_str());
    }		
    
    SYS->cntrCmd(f_path,&n_el1,TCntrNode::Set);

    return( 0x01 );
} 

int TWEB::post_table( SSess &ses, XMLNode &node, string prs_path )
{
    XMLNode 	n_el1;
    string 	path = string("/")+MOD_ID+ses.url;
    string 	f_path, l_path, l_com;
    int 	op_cnt = 0;
    
    //Get command name and path
    int c_pos = 1;    
    while(true)
    {
	int t_pos = prs_path.find("/",c_pos);
	if( t_pos == string::npos )
	{
	    l_com = prs_path.substr(c_pos,t_pos-c_pos);
	    break;
	}
	l_path = l_path + "/" + prs_path.substr(c_pos,t_pos-c_pos);//+"/";
	c_pos = t_pos+1;	
    }
    f_path = ses.url+"/"+SYS->pathCode(l_path,false);
    
    try{ SYS->cntrCmd(f_path, &node, TCntrNode::Get); }
    catch(TError err){ ses.mess.push_back( err.what() ); }
    
    if( l_com == "apply" )
	for( int i_cl=0; i_cl < node.childSize(); i_cl++)	    
	    for( int i_rw=0; i_rw < node.childGet(i_cl)->childSize(); i_rw++)
	    {		
		XMLNode *w_el = node.childGet(i_cl)->childGet(i_rw);
		string new_val = cntGet(ses,TSYS::int2str(i_rw)+":"+TSYS::int2str(i_cl));
		if( node.childGet(i_cl)->attr("tp") == "bool" )
		    new_val = new_val.size()?"true":"false";
		if( new_val != w_el->text() )
		{
		    n_el1.name("set");
		    n_el1.attr("row",TSYS::int2str(i_rw))->attr("col",TSYS::int2str(i_cl))->text(new_val);
		    Mess->put("WEB_CONTROL",MESS_INFO,"%s| Set <%s> cell (%d:%d) to: %s.",
	                ses.user.c_str(), f_path.c_str(), i_rw, i_cl, new_val.c_str());

		    SYS->cntrCmd(f_path,&n_el1,TCntrNode::Set);
		}
    	    }
    else if( l_com == "add" )
    {
	n_el1.name("add");	
	Mess->put("WEB_CONTROL",MESS_INFO,"%s| Add <%s> record.", ses.user.c_str(), f_path.c_str() );
	
	SYS->cntrCmd(f_path,&n_el1,TCntrNode::Set);
    }
    else if( l_com == "ins" && node.childSize() > 0 )
    {
	for( int i_rw=0; i_rw < node.childGet(0)->childSize(); i_rw++)
	    if( cntGet(ses,"row:"+TSYS::int2str(i_rw)).size() )
	    {
		n_el1.name("ins");
		n_el1.attr("row",TSYS::int2str(i_rw+op_cnt));		    
		Mess->put("WEB_CONTROL",MESS_INFO,"%s| Insert <%s> record %d.",
		    ses.user.c_str(), f_path.c_str(), i_rw+op_cnt );
		
		SYS->cntrCmd(f_path,&n_el1,TCntrNode::Set);
		op_cnt++;
	    }	    
    }
    else if( l_com == "del" && node.childSize() > 0 )
    {
	for( int i_rw=0; i_rw < node.childGet(0)->childSize(); i_rw++)
	    if( cntGet(ses,"row:"+TSYS::int2str(i_rw)).size() )
	    {
		n_el1.name("del");
		n_el1.attr("row",TSYS::int2str(i_rw-op_cnt));		    
		Mess->put("WEB_CONTROL",MESS_INFO,"%s| Delete <%s> record %d.",
		    ses.user.c_str(), f_path.c_str(), i_rw-op_cnt );
		    
		SYS->cntrCmd(f_path,&n_el1,TCntrNode::Set);
		op_cnt++;
	    }	    
    }
    else if( (l_com == "up" || l_com == "down") && node.childSize() > 0 )
    {
	for( int i_rw=0; i_rw < node.childGet(0)->childSize(); i_rw++)
	    if( cntGet(ses,"row:"+TSYS::int2str(i_rw)).size() )
	    {
		int r_new = i_rw-1;
		if( l_com == "down" )  r_new = i_rw+1;
		n_el1.name("move");
		n_el1.attr("row",TSYS::int2str(i_rw))->attr("to",TSYS::int2str(r_new));		    
		Mess->put("WEB_CONTROL",MESS_INFO,"%s| Move <%s> record from %d to %d.",
		    ses.user.c_str(), f_path.c_str(), i_rw, r_new );
		
		SYS->cntrCmd(f_path,&n_el1,TCntrNode::Set);
	    }	    
    }

    return( 0x01 );
}

void TWEB::post_mess( string &page, string mess, int type )
{
    //Put system message.    
    mPutS("SYS",(type==3)?MESS_ERR:(type==2)?MESS_WARNING:MESS_INFO,mess); 
    
    page = page+"<table border='2' width='40%' align='center'><tbody>\n";
    if(type == 2 )      
	page = page+"<tr bgcolor='yellow'><td align='center'><b>Warning!</b></td></tr>\n";
    else if(type == 3 ) 
	page = page+"<tr bgcolor='red'><td align='center'><b>Error!</b></td></tr>\n";
    else page = page+"<tr bgcolor='#9999ff'><td align='center'><b>Message!</b></td></tr>\n"; 
    page = page+"<tr bgcolor='#cccccc'> <td align='center'>"+mess2html(mess)+"</td></tr>\n";
    page = page+"</tbody></table>\n";
}

bool TWEB::prepare_val( SSess &ses, XMLNode &node, string prs_path, bool compare )
{
    unsigned i_cnt;
    string   val;
    
    if( node.attr("tp") == "bool" )
    {    	
	for( i_cnt = 0; i_cnt < ses.cnt_names.size(); i_cnt++ )
	    if( ses.cnt_names[i_cnt] == node.attr("id") ) break;
	if( i_cnt < ses.cnt_names.size() ) val = "true";
	else                       val = "false";
	if( compare )
	{
	    SYS->cntrCmd(ses.url+"/"+SYS->pathCode(prs_path+node.attr("id"), false ), &node, TCntrNode::Get);
	    if( node.text() == val) return(false);   //No change fld 
	}
    }
    else if( node.attr("tp") == "time" )
    {
	time_t c_tm = time(NULL);
	struct tm tm_tm;
	tm_tm = *localtime(&c_tm);
	string s_id = node.attr("id");
	//tm_tm.tm_isdst = 1;
	
	for( i_cnt = 0, val = ""; i_cnt < ses.cnt_names.size(); i_cnt++ )
	{
	    if( ses.cnt_names[i_cnt] == s_id+"_d" )       
		tm_tm.tm_mday = atoi( ses.cnt_vals[i_cnt].c_str() );
	    else if( ses.cnt_names[i_cnt] == s_id+"_ms" ) 
		tm_tm.tm_mon  = atoi( ses.cnt_vals[i_cnt].c_str() )-1;
	    else if( ses.cnt_names[i_cnt] == s_id+"_y" )  
		tm_tm.tm_year = atoi( ses.cnt_vals[i_cnt].c_str() )-1900;
	    else if( ses.cnt_names[i_cnt] == s_id+"_h" )  
		tm_tm.tm_hour = atoi( ses.cnt_vals[i_cnt].c_str() );
	    else if( ses.cnt_names[i_cnt] == s_id+"_m" )  
		tm_tm.tm_min  = atoi( ses.cnt_vals[i_cnt].c_str() );
	    else if( ses.cnt_names[i_cnt] == s_id+"_s" )  
		tm_tm.tm_sec  = atoi( ses.cnt_vals[i_cnt].c_str() );	    
	}
	val = TSYS::int2str(mktime(&tm_tm),C_INT_HEX);
	if( compare )
	{
	    SYS->cntrCmd(ses.url+"/"+SYS->pathCode(prs_path+node.attr("id"), false ), &node, TCntrNode::Get);
	    if( node.text() == val) return(false);   //No change time 
	}
    }
    else
    {
	for( i_cnt = 0; i_cnt < ses.cnt_names.size(); i_cnt++ )
	    if( ses.cnt_names[i_cnt] == node.attr("id") ) break;
	if( i_cnt < ses.cnt_names.size() )
	{
	    val = ses.cnt_vals[i_cnt];
	    if( compare )
	    {
		SYS->cntrCmd(ses.url+"/"+SYS->pathCode(prs_path+node.attr("id"), false ), &node, TCntrNode::Get);
		if( node.text() == val) return(false);   //No change fld 
	    }
	}
	else return(false);
    } 
    node.text(val);

    return(true);
}


int TWEB::post_auth( SSess &ses )
{
    unsigned i_cnt; 

    for( i_cnt = 0; i_cnt < ses.cnt_names.size(); i_cnt++ )
	if( ses.cnt_names[i_cnt] == "auth_enter" ) break;	
    if( i_cnt < ses.cnt_names.size() )
    {
	string pass;
	//Get user name
	for( i_cnt = 0; i_cnt < ses.cnt_names.size(); i_cnt++ ) 
	    if( ses.cnt_names[i_cnt] == "user" ) ses.user = ses.cnt_vals[i_cnt];
	//Get user password
	for( i_cnt = 0; i_cnt < ses.cnt_names.size(); i_cnt++ ) 
	    if( ses.cnt_names[i_cnt] == "pass" ) pass = ses.cnt_vals[i_cnt];
	try
	{ 
	    if( owner().owner().Sequrity().usrAt(ses.user).at().auth(pass) )
	    {
		ses.page = string("HTTP/1.0 200 OK\nContent-type: text/html\nSet-Cookie: oscd_u_id=")+TSYS::int2str(open_ses(ses.user))+"; path=/;\n\n";
		ses.page = ses.page+w_head()+w_body();
		return( 0x01 );
	    }
	}
	catch(TError err){ mPut("SYS",MESS_WARNING,"Auth %s!",err.what().c_str()); }
	
	post_mess(ses.page,"Auth wrong! Retry please.",3);
	ses.page = ses.page+"\n";
	get_auth( ses );
	return( 0x01|0x02 );
    }    
    
    for( i_cnt = 0; i_cnt < ses.cnt_names.size(); i_cnt++ )
	if( ses.cnt_names[i_cnt] == "auth_ch" ) break;	
    if( i_cnt < ses.cnt_names.size() )
    {
	get_auth( ses );
	return( 0x01|0x02 );    
    }
    
    return( 0x00 );
}

int TWEB::open_ses( string name )
{    
    SAuth *Auth;    
    bool  n_ses = true;
    //Check sesion and close old sesion
    ResAlloc res(m_res,false);
    for( int i_s = 0; i_s < m_auth.size(); i_s++ )
	if( time(NULL) > (m_auth[i_s]->t_auth+m_t_auth*60) ) 
	{
	    delete m_auth[i_s];
	    m_auth.erase(m_auth.begin() + i_s);
	    i_s--;
	}
    //Find opened sesion
    for( int i_s = 0; i_s < m_auth.size(); i_s++ )
	if( name == m_auth[i_s]->name ) 
	{
	    Auth = m_auth[i_s];
	    Auth->t_auth = time(NULL);
	    n_ses = false;
	    break;
	}
    res.release();
    
    if(n_ses)
    {
	//Make new sesion    
	Auth = new SAuth();
	Auth->t_auth = time(NULL);
	Auth->name   = name;
	Auth->id_ses = rand();

	res.request(m_res,true);
	m_auth.push_back( Auth );
	res.release( );
    }
	    
    return(Auth->id_ses);
}

void TWEB::check_ses( SSess &ses )
{    
    int id = atoi(get_cookie( "oscd_u_id", ses.vars ).c_str());
    //Check sesion and close old sesion
    ResAlloc res(m_res,false);
    for( unsigned i_s = 0; i_s < m_auth.size(); i_s++ )
	if( time(NULL) > (m_auth[i_s]->t_auth+m_t_auth*60) ) 
	{
	    delete m_auth[i_s];
	    m_auth.erase(m_auth.begin() + i_s);
	}
    //Find id sesion
    for( unsigned i_s = 0; i_s < m_auth.size(); i_s++ )
	if( m_auth[i_s]->id_ses == id )
	{
	    ses.user = m_auth[i_s]->name; 
	    m_auth[i_s]->t_auth = time(NULL); 
	}
}

void TWEB::cont_frm_data( SSess &ses )
{
    int pos = 0, i_bnd;
    string boundary;
    char *c_bound = "boundary=";
    char *c_term = "\r\n";
    char *c_end = "--";
    char *c_fd = "Content-Disposition: form-data;";
    char *c_name = "name=\"";    
    char *c_file = "filename=\"";    
    
    for( int i_vr = 0; i_vr < ses.vars.size(); i_vr++ )
	if( ses.vars[i_vr].substr(0,ses.vars[i_vr].find(":",0)) == "Content-Type" )
	{
	    int pos = ses.vars[i_vr].find(c_bound,0)+strlen(c_bound);
	    boundary = ses.vars[i_vr].substr(pos,ses.vars[i_vr].size()-pos);
	}
    if( !boundary.size() ) return;
    
    while(true)
    {
	pos = ses.content.find(boundary,pos);
	if( pos == string::npos || ses.content.substr(pos+boundary.size(),2) == "--" ) break;
	pos += boundary.size()+strlen(c_term);
	string c_head = ses.content.substr(pos, ses.content.find(c_term,pos)-pos);
        if( c_head.find(c_fd,0) == string::npos ) continue;
	//Get name
	i_bnd = c_head.find(c_name,0)+strlen(c_name);
	string c_name = c_head.substr(i_bnd,c_head.find("\"",i_bnd)-i_bnd);	
	i_bnd = c_head.find(c_file,0);
	if( i_bnd == string::npos )
	{	
    	    //Get value
    	    pos += c_head.size()+(2*strlen(c_term));
	    if(pos >= ses.content.size()) break;
    	    //string c_val  = ses.content.substr(pos, ses.content.find(c_term,pos)-pos);
    	    string c_val  = ses.content.substr(pos, ses.content.find(string(c_term)+c_end+boundary,pos)-pos);
    	    //Save
    	    ses.cnt_names.push_back(c_name);
    	    ses.cnt_vals.push_back(c_val);
	}
	else
	{
    	    ses.cnt_names.push_back(c_name);
	    i_bnd += strlen(c_file);
    	    ses.cnt_vals.push_back(c_head.substr(i_bnd,c_head.find("\"",i_bnd)-i_bnd));
	}
	//mPut("DEBUG",MESS_DEBUG,"%s:%s",name[name.size()-1].c_str(),val[val.size()-1].c_str());
    }    
}

string TWEB::get_cookie( string name, vector<string> &vars )
{
    for( unsigned i_var = 0; i_var < vars.size(); i_var++)
	if( vars[i_var].substr(0, vars[i_var].find(":",0)) == "Cookie" )
	{
	    int i_beg = vars[i_var].find(name+"=",0);
	    if( i_beg == string::npos ) return("");
	    i_beg += name.size()+1;
	    return( vars[i_var].substr(i_beg,vars[i_var].find(";",i_beg)-i_beg) );
	}
    return("");
}

bool TWEB::chk_access( XMLNode *fld, string user, char mode )
{
    string s_acc = fld->attr("acs");
    string s_own = fld->attr("own");
    string s_grp = fld->attr("grp");
    if( !s_acc.size() ) s_acc = "0777";
    int accs = strtol(s_acc.c_str(),NULL,8);
    if( !s_own.size() ) s_own = "0";        //root
    int own = atoi(s_own.c_str());
    if( !s_grp.size() ) s_grp = "0";        //root
    int grp = atoi(s_grp.c_str());

    return( owner().owner().Sequrity().access( user, mode, own, grp, accs) );
}

string TWEB::mess2html( string mess )
{    
    string rez,s_tmp;
    int i_pr, i_c;
    
    for( i_c = 0, i_pr = 0; i_c < mess.size(); i_c++ )
    {
	if(mess[i_c] == '>')      s_tmp = "&gt;";
	else if(mess[i_c] == '<') s_tmp = "&lt;";
	else if(mess[i_c] == '"') s_tmp = "&quot;";
	else if(mess[i_c] == '&') s_tmp = "&amp;";
	if( s_tmp.size() )
	{
	    rez = rez+mess.substr(i_pr,i_c-i_pr)+s_tmp;
	    s_tmp = "";
	    i_pr=i_c+1;
	}
    }
    rez = rez+mess.substr(i_pr,i_c-i_pr);

    return(rez);	
}

string TWEB::mess2java( string mess )
{
    string rez,s_tmp;
    int i_pr, i_c;
       
    for( i_c = 0, i_pr = 0; i_c < mess.size(); i_c++ )
    {
	if(mess[i_c] == '\n')     	s_tmp = "\\n";
        if( s_tmp.size() )
        {
    	    rez = rez+mess.substr(i_pr,i_c-i_pr)+s_tmp;
            s_tmp = "";
            i_pr=i_c+1;
	}
    }
    rez = rez+mess.substr(i_pr,i_c-i_pr);
    
    return(rez);														       
}

string TWEB::url_code( string url, bool contr )
{
    char buf[4];
    for( int i_c = 0; i_c < url.size(); i_c++ )
    {
	if(url[i_c] == '/' && contr)      url[i_c] = ':';	
	else if(url[i_c] == ' ') url.replace(i_c,1,"%20");
	else if(url[i_c]&0x80)
	{
	    snprintf(buf,sizeof(buf),"%%%2X",(unsigned char)url[i_c]);
	    url.replace(i_c,1,buf);
	}
    }

    return(url);	
}

string TWEB::url_encode( const string &url, bool contr )
{
    int n_pos, o_pos = 0;
    string rez;
    
    while(true)
    {
	n_pos = url.find("%",o_pos);
	if( n_pos == string::npos )
	{	 
	    rez=rez+url.substr(o_pos,n_pos-o_pos);
	    break;
	}
	rez=rez+url.substr(o_pos,n_pos-o_pos);
	rez=rez+(char)strtol(url.substr(n_pos+1,2).c_str(),NULL,16);
	o_pos=n_pos+3;
    }
    if( contr )
    	for( int i_c = 0; i_c < rez.size(); i_c++ )
    	    if(rez[i_c] == ':') rez[i_c] = '/';
	
    return(rez);
}

void TWEB::down_colont( SSess &ses )
{
    // Draw path
    ses.page = ses.page+"<hr width='100%' size='2'/>"+ses.url+"<br/>\n";
    // Make warning messages
    if( ses.mess.size() )
    {
	ses.page = ses.page+"<SCRIPT LANGUAGE='JavaScript'>\n<!--\n";
	for( int i_m = 0; i_m < ses.mess.size(); i_m++)
	    ses.page = ses.page+"alert('"+mess2java(ses.mess[i_m])+"');\n";
	ses.page = ses.page+"//-->\n</SCRIPT>\n";
    }
}

string TWEB::cntGet( SSess &ses, const string &nm )
{
    for( int i_c = 0; i_c < ses.cnt_names.size(); i_c++ )
	if( ses.cnt_names[i_c] == nm )
	    return ses.cnt_vals[i_c];
    return "";
}

//================== Controll functions ========================
void TWEB::cntrCmd_( const string &a_path, XMLNode *opt, int cmd )
{
    if( cmd==TCntrNode::Info )
    {
	TUI::cntrCmd_( a_path, opt, cmd );

	ctrInsNode("area",1,opt,a_path.c_str(),"/bs",I18N(MOD_NAME));
	ctrMkNode("fld",opt,a_path.c_str(),"/bs/lf_tm",I18N("Life time of auth sesion(min)"),0660,0,0,"dec");
	ctrMkNode("fld",opt,a_path.c_str(),"/help/g_help",Mess->I18N("Options help"),0440,0,0,"str")->
	    attr_("cols","90")->attr_("rows","5");
    }
    else if( cmd==TCntrNode::Get )
    {
	if( a_path == "/bs/lf_tm" )		ctrSetI( opt, m_t_auth );
	else if( a_path == "/help/g_help" )	ctrSetS( opt, optDescr() );       
	else TUI::cntrCmd_( a_path, opt, cmd );
    }
    else if( cmd==TCntrNode::Set )
    {
	if( a_path == "/bs/lf_tm" ) 	m_t_auth = ctrGetI( opt );
	else TUI::cntrCmd_( a_path, opt, cmd );
    }
}


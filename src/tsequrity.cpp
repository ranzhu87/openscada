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

#include <getopt.h>

#include "tsys.h"
#include "tkernel.h"
#include "tmessage.h"
#include "tsequrity.h"

TSequrity::TSequrity( TKernel *app ) : 
    m_owner(app), m_bd_usr("", "", "seq_usr"), m_bd_grp("", "", "seq_grp")
{
    m_usr = TCntrNode::grpAdd();
    m_grp = TCntrNode::grpAdd();
    nodeEn();
    
    //User BD structure
    user_el.fldAdd( new TFld("NAME",Mess->I18N("Name"),T_STRING|F_KEY,"20") );
    user_el.fldAdd( new TFld("DESCR",Mess->I18N("Full name"),T_STRING,"50") );
    user_el.fldAdd( new TFld("ID",Mess->I18N("Identificator"),T_DEC,"3") );
    user_el.fldAdd( new TFld("PASS",Mess->I18N("Password"),T_STRING,"20") );
    user_el.fldAdd( new TFld("GRP",Mess->I18N("Default group"),T_STRING,"20") );
    //Group BD structure
    grp_el.fldAdd( new TFld("NAME",Mess->I18N("Name"),T_STRING|F_KEY,"20") );
    grp_el.fldAdd( new TFld("DESCR",Mess->I18N("Full name"),T_STRING,"50") );
    grp_el.fldAdd( new TFld("ID",Mess->I18N("Identificator"),T_DEC,"3") );
    grp_el.fldAdd( new TFld("USERS",Mess->I18N("Users"),T_STRING,"50") );
        
    //Add surely users, groups and set parameters
    usrAdd("root");
    usrAt("root").at().lName("Administrator (superuser)!!!"); 
    usrAt("root").at().pass("openscada"); 
    
    grpAdd("root");
    grpAt("root").at().lName("Administrators group.");     
}

TSequrity::~TSequrity(  )
{
    delAll();
}
	
string TSequrity::name()
{
    return Mess->I18N("Sequrity"); 
}

TBDS::SName TSequrity::userBD()
{ 
    return owner().nameDBPrep(m_bd_usr); 
}

TBDS::SName TSequrity::grpBD() 
{ 
    return owner().nameDBPrep(m_bd_grp);
}	

void TSequrity::usrAdd( const string &name )
{    
    if( chldAvoid(m_usr,name) ) return;
    chldAdd(m_usr,new TUser(this,name,usr_id_f(),&user_el)); 
}

void TSequrity::grpAdd( const string &name )
{
    if( chldAvoid(m_grp,name) ) return;
    chldAdd(m_grp,new TGroup(this,name,grp_id_f(),&grp_el)); 
}

unsigned TSequrity::usr_id_f()
{
    unsigned id = 0;
    vector<string> list;
    usrList(list); 
    for( int i_l = 0; i_l < list.size(); i_l++ )
	if( usrAt(list[i_l]).at().id() == id ){ id++; i_l=-1; }
    return(id);
}

unsigned TSequrity::grp_id_f()
{
    unsigned id = 0;
    vector<string> list;
    grpList(list); 
    for( int i_l = 0; i_l < list.size(); i_l++ )
	if( grpAt(list[i_l]).at().id() == id ){ id++; i_l=-1; }
    return(id);
}

string TSequrity::usr( int id )
{
    vector<string> list;
    
    usrList(list); 
    for( int i_l = 0; i_l < list.size(); i_l++ )
	if( usrAt(list[i_l]).at().id() == id ) return(list[i_l]);
    return("");
}

string TSequrity::grp( int id )
{
    vector<string> list;
    
    grpList(list); 
    for( int i_l = 0; i_l < list.size(); i_l++ )
	if( grpAt(list[i_l]).at().id() == id ) return(list[i_l]);
    return("");
}

bool TSequrity::access( const string &user, char mode, int owner, int group, int access )
{
    bool rez = false;

    try
    {
    	AutoHD<TUser> r_usr = usrAt(user);
	// Check owner permision
	if( r_usr.at().id() == 0 || r_usr.at().id() == owner )
	    if( ((mode&SEQ_RD)?access&0400:true) && 
		((mode&SEQ_WR)?access&0200:true) && 
		((mode&SEQ_XT)?access&0100:true) )
	    rez = true; 
	// Check other permision
	if( !rez && ((mode&SEQ_RD)?access&0004:true) && 
	    ((mode&SEQ_WR)?access&0002:true) && 
	    ((mode&SEQ_XT)?access&0001:true) )
	    rez = true; 	
	// Check groupe permision
	if( !rez )
	{
	    string n_grp = grp(group);
	    if( n_grp.size() )
	    {
		if( (n_grp == r_usr.at().grp() || grpAt(n_grp).at().user(user)) &&
		    ((mode&SEQ_RD)?access&0040:true) && 
		    ((mode&SEQ_WR)?access&0020:true) && 
		    ((mode&SEQ_XT)?access&0010:true) )
		    rez = true;
	    }
	}	
    }catch(...){  }

    return(rez);
}

XMLNode *TSequrity::cfgNode()
{
    int i_k = 0;
    while(true)
    {
	XMLNode *t_n = owner().cfgNode()->childGet("section",i_k++);
	if( t_n->attr("id") == id() ) return( t_n );
    }
}

void TSequrity::init( )
{
    loadBD();
}

string TSequrity::optDescr( )
{
    char buf[STR_BUF_LEN];
    snprintf(buf,sizeof(buf),Mess->I18N(
	"======================= The Sequrity subsystem options =====================\n"
	"------------ Parameters of section <%s> in config file -----------\n"
	"UserBD  <fullname>  User bd, recorded:  \"<TypeBD>:<NameBD>:<NameTable>\";\n"
	"GrpBD   <fullname>  Group bd, recorded: \"<TypeBD>:<NameBD>:<NameTable>\";\n\n"),id().c_str());
    
    return(buf);
}

void TSequrity::checkCommandLine(  )
{
#if OSC_DEBUG
    owner().mPut("DEBUG",MESS_INFO,"%s: Read commandline options!",name().c_str());
#endif
	
    int next_opt;
    char *short_opt="h";
    struct option long_opt[] =
    {
	{"help"     ,0,NULL,'h'},
	{NULL       ,0,NULL,0  }
    };

    optind=opterr=0;	 
    do
    {
	next_opt=getopt_long(SYS->argc,( char *const * ) SYS->argv,short_opt,long_opt,NULL);
	switch(next_opt)
	{
	    case 'h': fprintf(stdout,optDescr().c_str()); break;
	    case -1 : break;
	}
    } while(next_opt != -1);
    
#if OSC_DEBUG
    owner().mPut("DEBUG",MESS_DEBUG,"%s: Read commandline options ok!",name().c_str());
#endif
}

void TSequrity::updateOpt()
{
    string opt;

    try
    {
    	opt = cfgNode()->childGet("id","UserBD")->text(); 
        m_bd_usr.tp  = TSYS::strSepParse(opt,0,':');
        m_bd_usr.bd  = TSYS::strSepParse(opt,1,':');
	m_bd_usr.tbl = TSYS::strSepParse(opt,2,':');
    }
    catch(...) {  }    
    
    try
    {
    	opt = cfgNode()->childGet("id","GrpBD")->text(); 
        m_bd_grp.tp  = TSYS::strSepParse(opt,0,':');
        m_bd_grp.bd  = TSYS::strSepParse(opt,1,':');
	m_bd_grp.tbl = TSYS::strSepParse(opt,2,':');
    }
    catch(...) {  }    
}

void TSequrity::loadBD( )
{
    int 	fld_cnt;
    string 	name;
    AutoHD<TTable> tbl;    
    
    // Load users from bd
    try
    {
	TConfig g_cfg(&user_el);
        fld_cnt=0;
	AutoHD<TTable> tbl = owner().BD().open(userBD());
        while( tbl.at().fieldSeek(fld_cnt++,g_cfg) )
	{
	    name = g_cfg.cfg("NAME").getS();
	    if( !usrAvoid(name) )
	    {
		usrAdd(name);
		((TConfig &)usrAt(name).at()) = g_cfg;
	    }
            else usrAt(name).at().load();
	}
	tbl.free();
	owner().BD().close(userBD());   
    }catch(...){}
    
    // Load groups from bd
    try
    {
	TConfig g_cfg(&grp_el);
        fld_cnt=0;
	AutoHD<TTable> tbl = owner().BD().open(grpBD());
        while( tbl.at().fieldSeek(fld_cnt++,g_cfg) )
	{
	    name = g_cfg.cfg("NAME").getS();
	    if( !grpAvoid(name) )
	    { 
		grpAdd(name);
		((TConfig &)grpAt(name).at()) = g_cfg;
	    }
            else grpAt(name).at().load();	
	}
	tbl.free();
	owner().BD().close(grpBD());
    }catch(...){}
}

void TSequrity::saveBD( )
{
    vector<string> list;
    
    // Save users to bd
    usrList(list);
    for( int i_l = 0; i_l < list.size(); i_l++ )
	usrAt(list[i_l]).at().save();
    
    // Save groups to bd
    for( int i_l = 0; i_l < list.size(); i_l++ )
	grpAt(list[i_l]).at().save();
}

//================== Controll functions ========================
void TSequrity::cntrCmd_( const string &a_path, XMLNode *opt, int cmd )
{
    vector<string> list;
    
    if( cmd==TCntrNode::Info )
    {
	ctrMkNode("oscada_cntr",opt,a_path.c_str(),"/",Mess->I18N("Sequrity subsystem"));	
	ctrMkNode("area",opt,a_path.c_str(),"/bd",Mess->I18N("Subsystem"),0440);
	if( owner().genDB( ) )
	{
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/u_tbl",Mess->I18N("User table"),0660,0,0,"str");
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/g_tbl",Mess->I18N("Group table"),0660,0,0,"str");
	}
	else
	{
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/u_t_bd",Mess->I18N("User BD (module:bd:table)"),0660,0,0,"str")->
		attr_("dest","select")->attr_("select","/bd/b_mod");
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/u_bd","",0660,0,0,"str");
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/u_tbl","",0660,0,0,"str");
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/g_t_bd",Mess->I18N("Group BD (module:bd:table)"),0660,0,0,"str")->
		attr_("dest","select")->attr_("select","/bd/b_mod");
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/g_bd","",0660,0,0,"str");
	    ctrMkNode("fld",opt,a_path.c_str(),"/bd/g_tbl","",0660,0,0,"str");
	}
	ctrMkNode("comm",opt,a_path.c_str(),"/bd/load_bd",Mess->I18N("Load from BD"));
	ctrMkNode("comm",opt,a_path.c_str(),"/bd/upd_bd",Mess->I18N("Save to BD"));
	ctrMkNode("area",opt,a_path.c_str(),"/usgr",Mess->I18N("Users and groups"));
	ctrMkNode("list",opt,a_path.c_str(),"/usgr/users",Mess->I18N("Users"),0644,0,0,"br")->
	    attr_("s_com","add,del")->attr_("mode","att")->attr_("br_pref","_usr_");
	ctrMkNode("list",opt,a_path.c_str(),"/usgr/grps",Mess->I18N("Groups"),0644,0,0,"br")->
	    attr_("s_com","add,del")->attr_("mode","att")->attr_("br_pref","_grp_");
	ctrMkNode("area",opt,a_path.c_str(),"/help",Mess->I18N("Help"));
	ctrMkNode("fld",opt,a_path.c_str(),"/help/g_help",Mess->I18N("Options help"),0440,0,0,"str")->
	    attr_("cols","90")->attr_("rows","5");
    }
    else if( cmd==TCntrNode::Get )
    {    
	if( a_path == "/bd/u_t_bd" )     ctrSetS( opt, m_bd_usr.tp );
	else if( a_path == "/bd/u_bd" )  ctrSetS( opt, m_bd_usr.bd );
	else if( a_path == "/bd/u_tbl" ) ctrSetS( opt, m_bd_usr.tbl );
	else if( a_path == "/bd/g_t_bd" )ctrSetS( opt, m_bd_grp.tp );
	else if( a_path == "/bd/g_bd" )  ctrSetS( opt, m_bd_grp.bd );
	else if( a_path == "/bd/g_tbl" ) ctrSetS( opt, m_bd_grp.tbl );
	else if( a_path == "/bd/b_mod" )
	{
	    owner().BD().gmdList(list);
	    opt->childClean();
	    ctrSetS( opt, "" );
	    for( unsigned i_a=0; i_a < list.size(); i_a++ )
		ctrSetS( opt, list[i_a] );
	}
	else if( a_path == "/help/g_help" ) ctrSetS( opt, optDescr() );       
	else if( a_path == "/usgr/users" )
	{
	    usrList(list);
	    opt->childClean();
	    for( unsigned i_a=0; i_a < list.size(); i_a++ )
		ctrSetS( opt, list[i_a] ); 	
	}
	else if( a_path == "/usgr/grps" )
	{
	    grpList(list);
	    opt->childClean();
	    for( unsigned i_a=0; i_a < list.size(); i_a++ )
		ctrSetS( opt, list[i_a] ); 	
	}    
	else throw TError("(Sequrity)Branch %s error!",a_path.c_str());
    }
    else if( cmd==TCntrNode::Set )
    {
	if( a_path == "/bd/u_t_bd" )     	m_bd_usr.tp  = ctrGetS( opt );
	else if( a_path == "/bd/u_bd" )  	m_bd_usr.bd  = ctrGetS( opt );
	else if( a_path == "/bd/u_tbl" ) 	m_bd_usr.tbl = ctrGetS( opt );
	else if( a_path == "/bd/g_t_bd" )	m_bd_grp.tp  = ctrGetS( opt );
	else if( a_path == "/bd/g_bd" )  	m_bd_grp.bd  = ctrGetS( opt );
	else if( a_path == "/bd/g_tbl" )	m_bd_grp.tbl = ctrGetS( opt );
	else if( a_path == "/usgr/users" )
	{
	    if( opt->name() == "add" )      	usrAdd(opt->text());
	    else if(opt->name() == "del")	chldDel(m_usr,opt->text(),-1,1);
	}
	else if( a_path == "/bd/load_bd" )	loadBD();
	else if( a_path == "/bd/upd_bd" )	saveBD();
	else if( a_path == "/usgr/grps" )
	{
	    if( opt->name() == "add" )      	grpAdd(opt->text());
	    else if(opt->name() == "del")	chldDel(m_grp,opt->text(),-1,1);
	}
	else throw TError("(Sequrity)Branch %s error!",a_path.c_str());    
    }
}

AutoHD<TCntrNode> TSequrity::ctrAt1( const string &br )
{
    if( br.substr(0,5) == "_usr_" )		return usrAt(br.substr(5));
    else if( br.substr(0,5) == "_grp_" ) 	return grpAt(br.substr(5));
    else throw TError("<{%s}> Branch %s error!",__func__,br.c_str());
}

//**************************************************************
//*********************** TUser ********************************
//**************************************************************
    
TUser::TUser( TSequrity *owner, const string &nm, unsigned id, TElem *el ) : 
    m_owner(owner), TConfig(el),
    m_lname(cfg("DESCR").getS()), m_pass(cfg("PASS").getS()), m_name(cfg("NAME").getS()), 
    m_id(cfg("ID").getI()), m_grp(cfg("GRP").getS())
{
    m_name = nm;
    m_id = id;
}

TUser::~TUser(  )
{

}

void TUser::postDisable(int flag)
{
    try
    {
        if( flag )
        {
            TBDS &bds = owner().owner().BD();
            bds.open(owner().userBD()).at().fieldDel(*this);
            bds.close(owner().userBD());
        }
    }catch(TError err)
    { owner().owner().mPut("SYS",MESS_ERR,"%s",err.what().c_str()); }
}
									    

void TUser::load( )
{
    TBDS &bds  = owner().owner().BD();
    bds.open( owner().userBD() ).at().fieldGet(*this);
    bds.close(owner().userBD());
}

void TUser::save( )
{
    TBDS &bds  = owner().owner().BD();
    bds.open( owner().userBD(), true ).at().fieldSet(*this);
    bds.close(owner().userBD());
}
//==============================================================
//================== Controll functions ========================
//==============================================================
void TUser::cntrCmd_( const string &a_path, XMLNode *opt, int cmd )
{
    vector<string> list;
    
    if( cmd==TCntrNode::Info )
    {
	ctrMkNode("oscada_cntr",opt,a_path.c_str(),"/",Mess->I18Ns("User ")+name());	
	ctrMkNode("area",opt,a_path.c_str(),"/prm",Mess->I18N("User"));
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/name",cfg("NAME").fld().descr(),0644,m_id,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/dscr",cfg("DESCR").fld().descr(),0644,m_id,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/grp",cfg("GRP").fld().descr(),0644,0,0,"str")->
	    attr_("dest","select")->attr_("select","/prm/grps");
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/id",cfg("ID").fld().descr(),0644,0,0,"dec");
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/pass",cfg("PASS").fld().descr(),0600,m_id,0,"str");
	ctrMkNode("comm",opt,a_path.c_str(),"/prm/load",Mess->I18N("Load from BD"),0550);
	ctrMkNode("comm",opt,a_path.c_str(),"/prm/save",Mess->I18N("Save to BD"),0550);
    }
    else if( cmd==TCntrNode::Get )
    {
	if( a_path == "/prm/name" )       ctrSetS( opt, name() );
	else if( a_path == "/prm/dscr" )  ctrSetS( opt, lName() );
	else if( a_path == "/prm/grp" )   ctrSetS( opt, grp() );
	else if( a_path == "/prm/id" )    ctrSetI( opt, id() );
	else if( a_path == "/prm/pass" )  ctrSetS( opt, "**********" );
	else if( a_path == "/prm/grps" )  
	{
	    owner().grpList(list);
	    opt->childClean();
	    for( unsigned i_a=0; i_a < list.size(); i_a++ )
		ctrSetS( opt, list[i_a] );
	}
	else throw TError("(User)Branch %s error!",a_path.c_str());
    }
    else if( cmd==TCntrNode::Set )
    {
	if( a_path == "/prm/name" )    	name( ctrGetS( opt ) );
	else if( a_path == "/prm/dscr" )lName( ctrGetS( opt ) );
	else if( a_path == "/prm/grp" )	grp( ctrGetS( opt ) );
	else if( a_path == "/prm/id" ) 	id( ctrGetI( opt ) );
	else if( a_path == "/prm/pass" )pass( ctrGetS( opt ) );
	else if( a_path == "/prm/load" )load();
	else if( a_path == "/prm/save" )save();	
	else throw TError("(User)branch %s error!",a_path.c_str());
    }
}

//**************************************************************
//*********************** TGroup *******************************
//**************************************************************
    
TGroup::TGroup( TSequrity *owner, const string &nm, unsigned id, TElem *el ) : 
    m_owner(owner), TConfig(el),
    m_lname(cfg("DESCR").getS()), m_usrs(cfg("USERS").getS()), m_name(cfg("NAME").getS()), m_id(cfg("ID").getI())
{
    m_name = nm;
    m_id = id;    
}

TGroup::~TGroup(  )
{

}

void TGroup::postDisable(int flag)
{
    try
    {
        if( flag )
        {
            TBDS &bds = owner().owner().BD();
            bds.open(owner().grpBD()).at().fieldDel(*this);
            bds.close(owner().grpBD());
        }
    }catch(TError err)
    { owner().owner().mPut("SYS",MESS_ERR,"%s",err.what().c_str()); }
}									    

void TGroup::load( )
{
    TBDS &bds  = owner().owner().BD();
    bds.open( owner().grpBD() ).at().fieldGet(*this);
    bds.close(owner().grpBD());
}

void TGroup::save( )
{
    TBDS &bds  = owner().owner().BD();
    bds.open( owner().grpBD(), true ).at().fieldSet(*this);
    bds.close(owner().grpBD());
}

bool TGroup::user( const string &name )
{
    if( m_usrs.find(name,0) != string::npos ) return(true);
    return(false);
}

//==============================================================
//================== Controll functions ========================
//==============================================================
void TGroup::cntrCmd_( const string &a_path, XMLNode *opt, int cmd )
{
    vector<string> list;
    
    if( cmd==TCntrNode::Info )
    {
	ctrMkNode("oscada_cntr",opt,a_path.c_str(),"/",Mess->I18Ns("Group ")+name());	
	ctrMkNode("area",opt,a_path.c_str(),"/prm",Mess->I18N("Group"));
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/name",cfg("NAME").fld().descr(),0644,0,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/dscr",cfg("DESCR").fld().descr(),0644,0,0,"str");
	ctrMkNode("fld",opt,a_path.c_str(),"/prm/id",cfg("ID").fld().descr(),0644,0,0,"dec");
	ctrMkNode("list",opt,a_path.c_str(),"/prm/users",cfg("USERS").fld().descr(),0644,0,0,"str")->
	    attr_("s_com","add,del")->attr_("dest","select")->attr_("select","/prm/usrs");
	ctrMkNode("comm",opt,a_path.c_str(),"/prm/load",Mess->I18N("Load from BD"),0550);
	ctrMkNode("comm",opt,a_path.c_str(),"/prm/save",Mess->I18N("Save to BD"),0550);
    }
    else if( cmd==TCntrNode::Get )
    {
	if( a_path == "/prm/name" )       ctrSetS( opt, name() );
	else if( a_path == "/prm/dscr" )  ctrSetS( opt, lName() );
	else if( a_path == "/prm/id" )    ctrSetI( opt, id() );
	else if( a_path == "/prm/users" )
	{
	    int pos = 0,c_pos;
	    opt->childClean();
	    do
	    {
		c_pos = m_usrs.find(";",pos);
		string val = m_usrs.substr(pos,c_pos-pos);
		if( val.size() ) ctrSetS( opt, val );
		pos = c_pos+1;
	    }while(c_pos != string::npos);
	}
	else if( a_path == "/prm/usrs" )  
	{
	    vector<string> list;
	    owner().usrList(list);
	    opt->childClean();
	    for( unsigned i_a=0; i_a < list.size(); i_a++ )
		ctrSetS( opt, list[i_a] );
	}
	else throw TError("(Group)branch %s error!",a_path.c_str());
    }
    else if( cmd==TCntrNode::Set )
    {
	if( a_path == "/prm/name" )       	name(ctrGetS( opt ));
	else if( a_path == "/prm/dscr" )  	lName(ctrGetS( opt ));
	else if( a_path == "/prm/id" )    	id(ctrGetI( opt ));
	else if( a_path == "/prm/users" )
	{
	    if( opt->name() == "add" )
	    {
		if(m_usrs.size()) m_usrs=m_usrs+";";
		m_usrs=m_usrs+opt->text();
	    }
	    if( opt->name() == "del" )
	    {
		int pos = m_usrs.find(string(";")+opt->text(),0);
		if(pos != string::npos) 
		    m_usrs.erase(pos,opt->text().size()+1);
		else                    
		    m_usrs.erase(m_usrs.find(opt->text(),0),opt->text().size()+1);
	    }
	}
	else if( a_path == "/prm/load" )	load();
	else if( a_path == "/prm/save" )	save();	
	else throw TError("(Group)branch %s error!",a_path.c_str());
    }
}

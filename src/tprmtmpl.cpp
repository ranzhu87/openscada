
//OpenSCADA file: tprmtmpl.cpp
/***************************************************************************
 *   Copyright (C) 2003-2018 by Roman Savochenko, <rom_as@oscada.org>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
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

#include "tsys.h"
#include "tprmtmpl.h"

using namespace OSCADA;

//*************************************************
//* TPrmTempl                                     *
//*************************************************
TPrmTempl::TPrmTempl( const string &iid, const string &iname ) :
    TFunction("tmpl_"+iid), TConfig(&SYS->daq().at().elTmpl()),
    mId(cfg("ID")), mTimeStamp(cfg("TIMESTAMP").getId())
{
    mId = iid;
    setName(iname);
    cfg("PROGRAM").setExtVal(true);
}

TPrmTempl::~TPrmTempl( )
{

}

TCntrNode &TPrmTempl::operator=( const TCntrNode &node )
{
    const TPrmTempl *src_n = dynamic_cast<const TPrmTempl*>(&node);
    if(!src_n) return *this;

    exclCopy(*src_n, "ID;");
    *(TFunction *)this = *(TFunction*)src_n;

    if(src_n->startStat() && !startStat()) setStart(true);

    return *this;
}

void TPrmTempl::postEnable( int flag )
{
    //Create default IOs
    if(flag&TCntrNode::NodeConnect) {
	ioIns(new IO("f_frq",_("Frequency of calculation of the function (Hz)"),IO::Real,TPrmTempl::LockAttr,"1000",false), 0);
	ioIns(new IO("f_start",_("Function start flag"),IO::Boolean,TPrmTempl::LockAttr,"0",false), 1);
	ioIns(new IO("f_stop",_("Function stop flag"),IO::Boolean,TPrmTempl::LockAttr,"0",false), 2);
	ioIns(new IO("f_err",_("Function error"),IO::String,TPrmTempl::LockAttr,"0",false), 3);
    }
}

void TPrmTempl::postDisable(int flag)
{
    if(flag) {
	SYS->db().at().dataDel(owner().fullDB(), owner().owner().nodePath()+owner().tbl(), *this, true);

	//Delete template's IO
	TConfig cfg(&owner().owner().elTmplIO());
	cfg.cfg("TMPL_ID").setS(id(), true);
	SYS->db().at().dataDel(owner().fullDB()+"_io",owner().owner().nodePath()+owner().tbl()+"_io/",cfg);
    }
}

bool TPrmTempl::cfgChange( TCfg &co, const TVariant &pc )
{
    if(co.name() == "PR_TR") cfg("PROGRAM").setNoTransl(!progTr());
    modif();
    return true;
}

TPrmTmplLib &TPrmTempl::owner( ) const	{ return *(TPrmTmplLib*)nodePrev(); }

string TPrmTempl::name( )		{ string nm = cfg("NAME").getS(); return nm.size() ? nm : id(); }

void TPrmTempl::setName( const string &inm )	{ cfg("NAME").setS(inm); }

string TPrmTempl::descr( )		{ return cfg("DESCR").getS(); }

void TPrmTempl::setDescr( const string &idsc )	{ cfg("DESCR").setS(idsc); }

string TPrmTempl::stor( ) const		{ return owner().DB(); }

int TPrmTempl::maxCalcTm( )		{ return cfg("MAXCALCTM").getI(); }

void TPrmTempl::setMaxCalcTm( int vl )	{ cfg("MAXCALCTM").setI(vl); }

string TPrmTempl::progLang( )	{ return TSYS::strLine(cfg("PROGRAM").getS(),0); }

string TPrmTempl::prog( )
{
    string tPrg = cfg("PROGRAM").getS();
    size_t lngEnd = tPrg.find("\n");
    return tPrg.substr((lngEnd==string::npos)?0:lngEnd+1);
}

void TPrmTempl::setProgLang( const string &ilng )
{
    if(startStat()) setStart(false);
    cfg("PROGRAM").setS(ilng+"\n"+prog());
}

void TPrmTempl::setProg( const string &iprg )
{
    if(startStat()) setStart(false);
    cfg("PROGRAM").setS(progLang()+"\n"+iprg);
}

void TPrmTempl::setStart( bool vl )
{
    if(startStat() == vl) return;
    if(vl) {
	//Check for doubled attributes clear
	std::map<string,bool> ioIds;
	for(int id = 0; id < ioSize(); )
	    if(ioIds.find(io(id)->id()) != ioIds.end()) { ioDel(id); modif(); }
	    else { ioIds[io(id)->id()] = true; id++; }

	//Compile new function
	if(prog().size())
	    work_prog = SYS->daq().at().at(TSYS::strSepParse(progLang(),0,'.')).at().
					compileFunc(TSYS::strSepParse(progLang(),1,'.'),*this,prog(),"",maxCalcTm());
    }
    TFunction::setStart(vl);
}

AutoHD<TFunction> TPrmTempl::func( )
{
    if(!startStat())	throw err_sys(_("Template is off."));
    if(!prog().size())	return AutoHD<TFunction>(this);
    try { return SYS->nodeAt(work_prog); }
    catch(TError &err) {
	//Template restart try
	setStart(false);
	setStart(true);
	return SYS->nodeAt(work_prog);
    }
}

void TPrmTempl::load_( TConfig *icfg )
{
    if(!SYS->chkSelDB(owner().DB())) throw TError();

    if(icfg) *(TConfig*)this = *icfg;
    else SYS->db().at().dataGet(owner().fullDB(), owner().owner().nodePath()+owner().tbl(), *this);

    //Load IO
    vector<string> u_pos;
    vector<vector<string> > full;
    TConfig ioCfg(&owner().owner().elTmplIO());
    ioCfg.cfg("TMPL_ID").setS(id(),true);
    for(int ioCnt = 0; SYS->db().at().dataSeek(owner().fullDB()+"_io",owner().owner().nodePath()+owner().tbl()+"_io",ioCnt++,ioCfg,false,&full); ) {
	string sid = ioCfg.cfg("ID").getS();

	// Position storing
	int pos = ioCfg.cfg("POS").getI();
	while((int)u_pos.size() <= pos)	u_pos.push_back("");
	u_pos[pos] = sid;

	int iid = ioId(sid);
	if(iid < 0)
	    ioIns(new IO(sid.c_str(),ioCfg.cfg("NAME").getS().c_str(),(IO::Type)ioCfg.cfg("TYPE").getI(),ioCfg.cfg("FLAGS").getI(),
			ioCfg.cfg("VALUE").getS().c_str(),false), pos);
	else {
	    io(iid)->setName(ioCfg.cfg("NAME").getS());
	    io(iid)->setType((IO::Type)ioCfg.cfg("TYPE").getI());
	    io(iid)->setFlg(ioCfg.cfg("FLAGS").getI());
	    io(iid)->setDef(ioCfg.cfg("VALUE").getS());
	}
    }

    //Remove holes
    for(unsigned iP = 0; iP < u_pos.size(); )
	if(u_pos[iP].empty()) u_pos.erase(u_pos.begin()+iP);
	else iP++;

    //Position fixing
    for(int iP = 0; iP < (int)u_pos.size(); iP++) {
	int iid = ioId(u_pos[iP]);
	if(iid != iP) try{ ioMove(iid,iP); } catch(...){ }
    }
}

void TPrmTempl::save_( )
{
    string w_db = owner().fullDB();
    string w_cfgpath = owner().owner().nodePath()+owner().tbl();

    //Self save
    mTimeStamp = SYS->sysTm();
    SYS->db().at().dataSet(w_db, w_cfgpath, *this);

    //Save IO
    TConfig cfg(&owner().owner().elTmplIO());
    cfg.cfg("TMPL_ID").setS(id(),true);
    for(int i_io = 0; i_io < ioSize(); i_io++) {
	if(io(i_io)->flg()&TPrmTempl::LockAttr) continue;
	cfg.cfg("ID").setS(io(i_io)->id());
	cfg.cfg("NAME").setS(io(i_io)->name());
	cfg.cfg("TYPE").setI(io(i_io)->type());
	cfg.cfg("FLAGS").setI(io(i_io)->flg());
	cfg.cfg("VALUE").setNoTransl(!(io(i_io)->type()==IO::String || io(i_io)->flg()&TPrmTempl::CfgLink));
	cfg.cfg("VALUE").setS(io(i_io)->def());
	cfg.cfg("POS").setI(i_io);
	SYS->db().at().dataSet(w_db+"_io",w_cfgpath+"_io",cfg);
    }
    //Clear IO
    vector<vector<string> > full;
    cfg.cfgViewAll(false);
    for(int fld_cnt = 0; SYS->db().at().dataSeek(w_db+"_io",w_cfgpath+"_io",fld_cnt++,cfg,false,&full); ) {
	string sio = cfg.cfg("ID").getS();
	if(ioId(sio) < 0 || io(ioId(sio))->flg()&TPrmTempl::LockAttr) {
	    if(!SYS->db().at().dataDel(w_db+"_io",w_cfgpath+"_io",cfg,true,false,true))	break;
	    if(full.empty()) fld_cnt--;
	}
    }
}

TVariant TPrmTempl::objFuncCall( const string &iid, vector<TVariant> &prms, const string &user )
{
    //Configuration functions call
    TVariant cfRez = objFunc(iid, prms, user);
    if(!cfRez.isNull()) return cfRez;

    return TCntrNode::objFuncCall(iid, prms, user);
}

void TPrmTempl::cntrCmdProc( XMLNode *opt )
{
    //Get page info
    if(opt->name() == "info") {
	TCntrNode::cntrCmdProc(opt);
	ctrMkNode("oscada_cntr",opt,-1,"/",_("Parameter template: ")+name(),RWRWR_,"root",SDAQ_ID,1,"doc","Documents/User_API|Documents/User_API");
	if(ctrMkNode("area",opt,-1,"/tmpl",_("Template"))) {
	    if(ctrMkNode("area",opt,-1,"/tmpl/st",_("State"))) {
		ctrMkNode("fld",opt,-1,"/tmpl/st/st",_("Accessing"),RWRWR_,"root",SDAQ_ID,1,"tp","bool");
		ctrMkNode("fld",opt,-1,"/tmpl/st/use",_("Used"),R_R_R_,"root",SDAQ_ID,1,"tp","dec");
		ctrMkNode("fld",opt,-1,"/tmpl/st/timestamp",_("Date of modification"),R_R_R_,"root",SDAQ_ID,1,"tp","time");
	    }
	    if(ctrMkNode("area",opt,-1,"/tmpl/cfg",_("Configuration"))) {
		ctrMkNode("fld",opt,-1,"/tmpl/cfg/ID",_("Identifier"),R_R_R_,"root",SDAQ_ID,1,"tp","str");
		ctrMkNode("fld",opt,-1,"/tmpl/cfg/NAME",_("Name"),RWRWR_,"root",SDAQ_ID,2,"tp","str","len",OBJ_NM_SZ);
		ctrMkNode("fld",opt,-1,"/tmpl/cfg/DESCR",_("Description"),RWRWR_,"root",SDAQ_ID,3,"tp","str","cols","100","rows","4");
		ctrMkNode("fld",opt,-1,"/tmpl/cfg/MAXCALCTM",_("Maximum calculate time, seconds"),(startStat()?R_R_R_:RWRWR_),"root",SDAQ_ID,3,"tp","dec","min","0","max","3600");
	    }
	}
	if(ctrMkNode("area",opt,-1,"/io",_("IO"))) {
	    if(ctrMkNode("table",opt,-1,"/io/io",_("IO"),RWRWR_,"root",SDAQ_ID,2,"s_com","add,del,ins,move","rows","5")) {
		ctrMkNode("list",opt,-1,"/io/io/0",_("Identifier"),RWRWR_,"root",SDAQ_ID,1,"tp","str");
		ctrMkNode("list",opt,-1,"/io/io/1",_("Name"),RWRWR_,"root",SDAQ_ID,1,"tp","str");
		ctrMkNode("list",opt,-1,"/io/io/2",_("Type"),RWRWR_,"root",SDAQ_ID,5,"tp","dec","idm","1","dest","select",
		    "sel_id",TSYS::strMess("%d;%d;%d;%d;%d;%d",IO::Real,IO::Integer,IO::Boolean,IO::String,IO::String|(IO::FullText<<8),IO::Object).c_str(),
		    "sel_list",_("Real;Integer;Boolean;String;Text;Object"));
		ctrMkNode("list",opt,-1,"/io/io/3",_("Mode"),RWRWR_,"root",SDAQ_ID,5,"tp","dec","idm","1","dest","select",
		    "sel_id",TSYS::strMess("%d;%d",IO::Default,IO::Output).c_str(),"sel_list",_("Input;Output"));
		ctrMkNode("list",opt,-1,"/io/io/4",_("Attribute"),RWRWR_,"root",SDAQ_ID,5,"tp","dec","idm","1","dest","select",
		    "sel_id",TSYS::strMess("%d;%d;%d",IO::Default,TPrmTempl::AttrRead,TPrmTempl::AttrFull).c_str(),
		    "sel_list",_("Not attribute;Read only;Full access"));
		ctrMkNode("list",opt,-1,"/io/io/5",_("Configuration"),RWRWR_,"root",SDAQ_ID,5,"tp","dec","idm","1","dest","select",
		    "sel_id",TSYS::strMess("%d;%d;%d",IO::Default,TPrmTempl::CfgConst,TPrmTempl::CfgLink).c_str(),
		    "sel_list",_("Variable;Constant;Link"));
		ctrMkNode("list",opt,-1,"/io/io/6",_("Value"),RWRWR_,"root",SDAQ_ID,1,"tp","str");
	    }
	    ctrMkNode("fld",opt,-1,"/io/prog_lang",_("Program language"),RWRW__,"root",SDAQ_ID,3,"tp","str","dest","sel_ed","select","/plang/list");
	    ctrMkNode("fld",opt,-1,"/io/prog_tr",cfg("PR_TR").fld().descr().c_str(),RWRW__,"root",SDAQ_ID,1,"tp","bool");
	    ctrMkNode("fld",opt,-1,"/io/prog",cfg("PROGRAM").fld().descr().c_str(),RWRW__,"root",SDAQ_ID,3,"tp","str","rows","10","SnthHgl","1");
	}
	return;
    }

    //Process command to page
    vector<string> list;
    string a_path = opt->attr("path");
    if(a_path == "/tmpl/st/st") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(startStat()?"1":"0");
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	setStart(s2i(opt->text()));
    }
    else if(a_path == "/tmpl/st/use" && ctrChkNode(opt))	opt->setText(i2s(startStat()?func().at().use():0));
    else if(a_path == "/tmpl/st/timestamp" && ctrChkNode(opt))	opt->setText(i2s(timeStamp()));
    else if(a_path == "/tmpl/cfg/ID" && ctrChkNode(opt))	opt->setText(id());
    else if(a_path == "/tmpl/cfg/NAME") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(name());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	setName(opt->text());
    }
    else if(a_path == "/tmpl/cfg/DESCR") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(descr());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	setDescr(opt->text());
    }
    else if(a_path == "/tmpl/cfg/MAXCALCTM") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(i2s(maxCalcTm()));
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	setMaxCalcTm(s2i(opt->text()));
    }
    else if(a_path == "/io/io") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD)) {
	    XMLNode *n_id   = ctrMkNode("list",opt,-1,"/io/io/0","");
	    XMLNode *n_nm   = ctrMkNode("list",opt,-1,"/io/io/1","");
	    XMLNode *n_type = ctrMkNode("list",opt,-1,"/io/io/2","");
	    XMLNode *n_mode = ctrMkNode("list",opt,-1,"/io/io/3","");
	    XMLNode *n_attr = ctrMkNode("list",opt,-1,"/io/io/4","");
	    XMLNode *n_accs = ctrMkNode("list",opt,-1,"/io/io/5","");
	    XMLNode *n_val  = ctrMkNode("list",opt,-1,"/io/io/6","");

	    for(int id = 0; id < ioSize(); id++) {
		if(n_id)	n_id->childAdd("el")->setText(io(id)->id());
		if(n_nm)	n_nm->childAdd("el")->setText(io(id)->name());
		if(n_type)	n_type->childAdd("el")->setText(i2s(io(id)->type()|((io(id)->flg()&IO::FullText)<<8)));
		if(n_mode)	n_mode->childAdd("el")->setText(i2s(io(id)->flg()&(IO::Output|IO::Return)));
		if(n_attr)	n_attr->childAdd("el")->setText(i2s(io(id)->flg()&(TPrmTempl::AttrRead|TPrmTempl::AttrFull)));
		if(n_accs)	n_accs->childAdd("el")->setText(i2s(io(id)->flg()&(TPrmTempl::CfgConst|TPrmTempl::CfgLink)));
		if(n_val)	n_val->childAdd("el")->setText(io(id)->def());
	    }
	}
	if(ctrChkNode(opt,"add",RWRWR_,"root",SDAQ_ID,SEC_WR)) {
	    IO *ioPrev = ioSize() ? io(ioSize()-1) : NULL;
	    if(ioPrev) ioAdd(new IO(TSYS::strLabEnum(ioPrev->id()).c_str(),TSYS::strLabEnum(ioPrev->name()).c_str(),ioPrev->type(),ioPrev->flg()&(~LockAttr)));
	    else ioAdd(new IO("new",_("New IO"),IO::Real,IO::Default));
	    modif();
	}
	if(ctrChkNode(opt,"ins",RWRWR_,"root",SDAQ_ID,SEC_WR)) {
	    int row = s2i(opt->attr("row"));
	    IO *ioPrev = row ? io(row-1) : NULL;
	    if(ioPrev) ioIns(new IO(TSYS::strLabEnum(ioPrev->id()).c_str(),TSYS::strLabEnum(ioPrev->name()).c_str(),ioPrev->type(),ioPrev->flg()&(~LockAttr)), row);
	    else ioIns(new IO("new",_("New IO"),IO::Real,IO::Default), row);
	    modif();
	}
	if(ctrChkNode(opt,"del",RWRWR_,"root",SDAQ_ID,SEC_WR)) {
	    int row = s2i(opt->attr("row"));
	    if(io(row)->flg()&TPrmTempl::LockAttr)
		throw err_sys(_("Deleting a locked attribute is not allowed."));
	    ioDel(row);
	    modif();
	}
	if(ctrChkNode(opt,"move",RWRWR_,"root",SDAQ_ID,SEC_WR))	{ ioMove(s2i(opt->attr("row")), s2i(opt->attr("to"))); modif(); }
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR)) {
	    int row = s2i(opt->attr("row"));
	    int col = s2i(opt->attr("col"));
	    if(io(row)->flg()&TPrmTempl::LockAttr)	throw err_sys(_("Changing locked attribute is not allowed."));
	    if((col == 0 || col == 1) && !opt->text().size())	throw err_sys(_("Empty value is not allowed."));
	    modif();
	    switch(col) {
		case 0:	io(row)->setId(opt->text());	break;
		case 1:	io(row)->setName(opt->text());	break;
		case 2:
		    io(row)->setType((IO::Type)(s2i(opt->text())&0xFF));
		    io(row)->setFlg(io(row)->flg()^((io(row)->flg()^(s2i(opt->text())>>8))&IO::FullText));
		    break;
		case 3:	io(row)->setFlg(io(row)->flg()^((io(row)->flg()^s2i(opt->text()))&(IO::Output|IO::Return)));		break;
		case 4:	io(row)->setFlg(io(row)->flg()^((io(row)->flg()^s2i(opt->text()))&(TPrmTempl::AttrRead|TPrmTempl::AttrFull)));		break;
		case 5:	io(row)->setFlg(io(row)->flg()^((io(row)->flg()^s2i(opt->text()))&(TPrmTempl::CfgConst|TPrmTempl::CfgLink)));	break;
		case 6:	io(row)->setDef(opt->text()); setStart(false); break;
	    }
	}
    }
    else if(a_path == "/io/prog_lang") {
	if(ctrChkNode(opt,"get",RWRW__,"root",SDAQ_ID,SEC_RD))	opt->setText(progLang());
	if(ctrChkNode(opt,"set",RWRW__,"root",SDAQ_ID,SEC_WR))	setProgLang(opt->text());
    }
    else if(a_path == "/io/prog_tr") {
	if(ctrChkNode(opt,"get",RWRW__,"root",SDAQ_ID,SEC_RD))	opt->setText(i2s(progTr()));
	if(ctrChkNode(opt,"set",RWRW__,"root",SDAQ_ID,SEC_WR))	setProgTr(s2i(opt->text()));
    }
    else if(a_path == "/io/prog") {
	if(ctrChkNode(opt,"get",RWRW__,"root",SDAQ_ID,SEC_RD))	opt->setText(prog());
	if(ctrChkNode(opt,"set",RWRW__,"root",SDAQ_ID,SEC_WR))	setProg(opt->text());
	if(ctrChkNode(opt,"SnthHgl",RWRW__,"root",SDAQ_ID,SEC_RD))
	    try {
		SYS->daq().at().at(TSYS::strParse(progLang(),0,".")).at().
				compileFuncSynthHighl(TSYS::strParse(progLang(),1,"."),*opt);
	    } catch(...){ }
    }
    else TCntrNode::cntrCmdProc(opt);
}

TPrmTempl::Impl::Impl( TCntrNode *iobj, const string &iname ) : TValFunc(iname.c_str(),NULL,false), obj(iobj)
{

}

bool TPrmTempl::Impl::lnkPresent( int num )
{
    MtxAlloc res(obj->dataRes(), true);
    map<int,SLnk>::iterator it = lnks.find(num);
    return (it != lnks.end());
}

void TPrmTempl::Impl::lnkAdd( int num, const SLnk &l )
{
    obj->dataRes().lock();
    lnks[num] = l;
    obj->dataRes().unlock();
}

string TPrmTempl::Impl::lnkAttr( int num )
{
    MtxAlloc res(obj->dataRes(), true);
    map<int,SLnk>::iterator it = lnks.find(num);
    if(it == lnks.end()) throw TError(obj->nodePath().c_str(), _("Error of parameter ID."));
    return it->second.prmAttr;
}

void TPrmTempl::Impl::lnkAttrSet( int num, const string &vl )
{
    MtxAlloc res(obj->dataRes(), true);
    map<int,SLnk>::iterator it = lnks.find(num);
    if(it == lnks.end()) throw TError(obj->nodePath().c_str(), _("Error of parameter ID."));
    it->second.prmAttr = vl;
}

void TPrmTempl::Impl::lnkClear( bool andFunc )
{
    obj->dataRes().lock();
    lnks.clear();
    if(andFunc) setFunc(NULL);
    obj->dataRes().unlock();
}

bool TPrmTempl::Impl::initTmplLnks( bool checkNoLink )
{
    //Init links
    string nmod, ncntr, nprm, nattr;

    MtxAlloc res(obj->dataRes(), true);
    for(map<int,SLnk>::iterator iL = lnks.begin(); iL != lnks.end(); ++iL) {
	if(checkNoLink && !iL->second.aprm.freeStat()) continue;
	try {
	    iL->second.aprm.free();
	    iL->second.detOff = 0;
	    iL->second.aprm = SYS->daq().at().attrAt(TSYS::strParse(iL->second.prmAttr,0,"#",&iL->second.detOff), '.', true);
	    if(!iL->second.aprm.freeStat()) {
		if(iL->second.aprm.at().fld().type() == TFld::Object && iL->second.detOff < (int)iL->second.prmAttr.size())
		    setS(iL->first, iL->second.aprm.at().getO().at().propGet(iL->second.prmAttr.substr(iL->second.detOff),'.'));
		else setS(iL->first, iL->second.aprm.at().getS());
	    }
	    else return true;
	} catch(TError &err) { return true; }
    }

    return false;
}

void TPrmTempl::Impl::inputLinks( )
{
    MtxAlloc res(obj->dataRes(), true);
    for(map<int,SLnk>::iterator iL = lnks.begin(); iL != lnks.end(); ++iL)
	if(iL->second.aprm.freeStat()) setS(iL->first, EVAL_STR);
	else if(iL->second.aprm.at().fld().type() == TFld::Object && iL->second.detOff < (int)iL->second.prmAttr.size())
	    set(iL->first, iL->second.aprm.at().getO().at().propGet(iL->second.prmAttr.substr(iL->second.detOff),'.'));
	else set(iL->first, iL->second.aprm.at().get());
}

void TPrmTempl::Impl::outputLinks( )
{
    MtxAlloc res(obj->dataRes(), true);
    for(map<int,SLnk>::iterator iL = lnks.begin(); iL != lnks.end(); ++iL)
	if(!iL->second.aprm.freeStat() && ioMdf(iL->first))
	    outputLink(iL->first, get(iL->first));

	/*if(!iL->second.aprm.freeStat() && ioMdf(iL->first) &&
		ioFlg(iL->first)&(IO::Output|IO::Return) && !(iL->second.aprm.at().fld().flg()&TFld::NoWrite))
	{
	    TVariant vl = get(iL->first);
	    if(!vl.isEVal()) {
		if(iL->second.aprm.at().fld().type() == TFld::Object && iL->second.detOff < (int)iL->second.prmAttr.size()) {
		    iL->second.aprm.at().getO().at().propSet(iL->second.prmAttr.substr(iL->second.detOff), '.', vl);
		    iL->second.aprm.at().setO(iL->second.aprm.at().getO());	//For modify object sign
		}
		else iL->second.aprm.at().set(vl);
	    }
	}*/
}

bool TPrmTempl::Impl::outputLink( int num, const TVariant &vl )
{
    if(vl.isEVal()) return false;
    MtxAlloc res(obj->dataRes(), true);
    map<int,SLnk>::iterator it = lnks.find(num);
    if(it == lnks.end() || it->second.aprm.freeStat() || (it->second.aprm.at().fld().flg()&TFld::NoWrite) ||
	    !(ioFlg(num)&(IO::Output|IO::Return)))
	return false;
    if(it->second.aprm.at().fld().type() == TFld::Object && it->second.detOff < (int)it->second.prmAttr.size()) {
	it->second.aprm.at().getO().at().propSet(it->second.prmAttr.substr(it->second.detOff), '.', vl);
	it->second.aprm.at().setO(it->second.aprm.at().getO());	//For modify object sign
    }
    else it->second.aprm.at().set(vl);

    return true;
}

bool TPrmTempl::Impl::cntrCmdProc( XMLNode *opt )
{
    MtxAlloc res(obj->dataRes(), true);
    //Get page info
    if(opt->name() == "info" && ctrMkNode("area",opt,-1,"/cfg",_("Template configuration"))) {
	vector<string> list;
	ctrMkNode("fld",opt,-1,"/cfg/attr_only",_("Only attributes are to be shown"),RWRWR_,"root",SDAQ_ID,1,"tp","bool");
	if(ctrMkNode("area",opt,-1,"/cfg/prm",_("Parameters")))
	    for(int iIO = 0; iIO < ioSize(); iIO++) {
		if(!(func()->io(iIO)->flg()&(TPrmTempl::CfgLink|TPrmTempl::CfgConst)))
		    continue;
		// Check select param
		bool is_lnk = func()->io(iIO)->flg()&TPrmTempl::CfgLink;
		if(is_lnk && func()->io(iIO)->def().size() &&
		    !s2i(TBDS::genDBGet(obj->nodePath()+"onlAttr","0",opt->attr("user"))))
		{
		    string nprm = TSYS::strSepParse(func()->io(iIO)->def(),0,'|');
		    // Check already to present parameters
		    bool f_ok = false;
		    for(unsigned iL = 0; iL < list.size() && !f_ok; iL++)
			if(list[iL] == nprm) f_ok = true;
		    if(!f_ok) {
			ctrMkNode("fld",opt,-1,(string("/cfg/prm/pr_")+i2s(iIO)).c_str(),nprm,RWRWR_,"root",SDAQ_ID,
			    3,"tp","str","dest","sel_ed","select",(string("/cfg/prm/pl_")+i2s(iIO)).c_str());
			list.push_back(nprm);
		    }
		}
		else {
		    const char *tip = "str";
		    bool fullTxt = false;
		    if(!is_lnk)
			switch(ioType(iIO)) {
			    case IO::Integer:	tip = "dec";	break;
			    case IO::Real:		tip = "real";	break;
			    case IO::Boolean:	tip = "bool";	break;
			    case IO::String:
				if(func()->io(iIO)->flg()&IO::FullText) fullTxt = true;
				break;
			    case IO::Object:	fullTxt = true;	break;
			}
		    XMLNode *wn = ctrMkNode("fld",opt,-1,(string("/cfg/prm/el_")+i2s(iIO)).c_str(),
			    func()->io(iIO)->name(),RWRWR_,"root",SDAQ_ID,1,"tp",tip);
		    if(wn && is_lnk) wn->setAttr("dest","sel_ed")->setAttr("select","/cfg/prm/ls_"+i2s(iIO));
		    if(wn && fullTxt)wn->setAttr("cols","100")->setAttr("rows","4");
		}
	    }
	return true;
    }

    //Process command to page
    string a_path = opt->attr("path");
    if(a_path == "/cfg/attr_only") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(TBDS::genDBGet(obj->nodePath()+"onlAttr","0",opt->attr("user")));
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	TBDS::genDBSet(obj->nodePath()+"onlAttr",opt->text(),opt->attr("user"));
    }
    else if(a_path.substr(0,12) == "/cfg/prm/pr_") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD)) {
	    string lnk_val = lnks[s2i(a_path.substr(12))].prmAttr;
	    if(!SYS->daq().at().attrAt(TSYS::strParse(lnk_val,0,"#"),'.',true).freeStat()) {
		opt->setText(lnk_val.substr(0,lnk_val.rfind(".")));
		opt->setText(opt->text()+" (+)");
	    }
	    else opt->setText(lnk_val);
	}
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR)) {
	    TParamContr *pC = dynamic_cast<TParamContr*>(obj);
	    string no_set;
	    string p_nm = TSYS::strSepParse(func()->io(s2i(a_path.substr(12)))->def(),0,'|');
	    string p_vl = TSYS::strParse(opt->text(), 0, " ");
	    if(pC && p_vl == pC->DAQPath()) throw TError(obj->nodePath().c_str(),_("Error, recursive linking."));
	    AutoHD<TValue> prm = SYS->daq().at().prmAt(p_vl, '.', true);

	    for(map<int,SLnk>::iterator iL = lnks.begin(); iL != lnks.end(); ++iL)
		if(p_nm == TSYS::strSepParse(func()->io(iL->first)->def(),0,'|')) {
		    iL->second.prmAttr = p_vl;
		    string p_attr = TSYS::strSepParse(func()->io(iL->first)->def(),1,'|');
		    if(!prm.freeStat()) {
			if(prm.at().vlPresent(p_attr)) {
			    iL->second.prmAttr = p_vl+"."+p_attr;
			    obj->modif();
			}
			else no_set += p_attr+",";
		    }
		}
	    initTmplLnks();
	}
    }
    else if((a_path.compare(0,12,"/cfg/prm/pl_") == 0 || a_path.compare(0,12,"/cfg/prm/ls_") == 0) && ctrChkNode(opt))
    {
	bool is_pl = (a_path.compare(0,12,"/cfg/prm/pl_") == 0);
	string m_prm = lnks[s2i(a_path.substr(12))].prmAttr;
	if(is_pl && !SYS->daq().at().attrAt(m_prm,'.',true).freeStat()) m_prm = m_prm.substr(0,m_prm.rfind("."));
	SYS->daq().at().ctrListPrmAttr(opt, m_prm, is_pl, '.');
    }
    else if(a_path.substr(0,12) == "/cfg/prm/el_") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD)) {
	    int iIO = s2i(a_path.substr(12));
	    if(func()->io(iIO)->flg()&TPrmTempl::CfgLink) {
		opt->setText(lnks[iIO].prmAttr);
		if(!SYS->daq().at().attrAt(TSYS::strParse(opt->text(),0,"#"),'.',true).freeStat()) opt->setText(opt->text()+" (+)");
	    }
	    else if(func()->io(iIO)->flg()&TPrmTempl::CfgConst)
		opt->setText(getS(iIO));
	}
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR)) {
	    int iIO = s2i(a_path.substr(12));
	    if(func()->io(iIO)->flg()&TPrmTempl::CfgLink) {
		string a_vl = TSYS::strParse(opt->text(), 0, " ");
		//if(TSYS::strSepParse(a_vl,0,'.') == owner().owner().modId() &&
		//	TSYS::strSepParse(a_vl,1,'.') == owner().id() &&
		//	TSYS::strSepParse(a_vl,2,'.') == id())
		//    throw TError(nodePath().c_str(),_("Error, recursive linking."));
		lnks[iIO].prmAttr = a_vl;
		initTmplLnks();
	    }
	    else if(func()->io(iIO)->flg()&TPrmTempl::CfgConst) setS(iIO, opt->text());
	    obj->modif();
	}
    }
    else return false;

    return true;
}

//*************************************************
//* TPrmTmplLib                                   *
//*************************************************
TPrmTmplLib::TPrmTmplLib( const string &id, const string &name, const string &lib_db ) :
    TConfig(&SYS->daq().at().elLib()), run_st(false), mId(cfg("ID")), work_lib_db(lib_db)
{
    mId = id;
    setName( name );
    cfg("DB").setS(string("tmplib_")+id);
    m_ptmpl = grpAdd("tmpl_");
}

TPrmTmplLib::~TPrmTmplLib()
{

}

TCntrNode &TPrmTmplLib::operator=( const TCntrNode &node )
{
    const TPrmTmplLib *src_n = dynamic_cast<const TPrmTmplLib*>(&node);
    if(!src_n) return *this;

    //Configuration copy
    exclCopy(*src_n, "ID;DB;");
    work_lib_db = src_n->work_lib_db;

    //Templates copy
    vector<string> ls;
    src_n->list(ls);
    for(unsigned iP = 0; iP < ls.size(); iP++) {
	if(!present(ls[iP])) add(ls[iP].c_str());
	(TCntrNode&)at(ls[iP]).at() = (TCntrNode&)src_n->at(ls[iP]).at();
    }
    if(src_n->startStat() && !startStat()) start(true);

    return *this;
}

void TPrmTmplLib::preDisable(int flag)
{
    start(false);
}

void TPrmTmplLib::postDisable(int flag)
{
    if(flag) {
	//Delete libraries record
	SYS->db().at().dataDel(DB()+"."+owner().tmplLibTable(),owner().nodePath()+"tmplib",*this,true);

	//Delete temlate librarie's DBs
	SYS->db().at().open(fullDB());
	SYS->db().at().close(fullDB(), true);

	SYS->db().at().open(fullDB()+"_io");
	SYS->db().at().close(fullDB()+"_io", true);
    }
}

TDAQS &TPrmTmplLib::owner( ) const	{ return *(TDAQS*)nodePrev(); }

string TPrmTmplLib::name( )	{ string nm = cfg("NAME").getS(); return nm.size() ? nm : id(); }

void TPrmTmplLib::setName( const string &vl )	{ cfg("NAME").setS(vl); }

string TPrmTmplLib::descr( )	{ return cfg("DESCR").getS(); }

void TPrmTmplLib::setDescr( const string &vl )	{ cfg("DESCR").setS(vl); }

void TPrmTmplLib::setFullDB( const string &vl )
{
    size_t dpos = vl.rfind(".");
    work_lib_db = (dpos!=string::npos) ? vl.substr(0,dpos) : "";
    cfg("DB").setS((dpos!=string::npos) ? vl.substr(dpos+1) : "");
    modifG();
}

void TPrmTmplLib::load_( TConfig *icfg )
{
    if(!SYS->chkSelDB(DB())) throw TError();

    if(icfg) *(TConfig*)this = *icfg;
    else SYS->db().at().dataGet(DB()+"."+owner().tmplLibTable(), owner().nodePath()+"tmplib", *this);

    //Load templates
    map<string, bool>	itReg;
    vector<vector<string> > full;
    TConfig cEl(&owner().elTmpl());
    //cEl.cfgViewAll(false);
    for(int fldCnt = 0; SYS->db().at().dataSeek(fullDB(),owner().nodePath()+tbl(), fldCnt++,cEl,false,&full); ) {
	string f_id = cEl.cfg("ID").getS();
	if(!present(f_id)) add(f_id);
	at(f_id).at().load(&cEl);
	itReg[f_id] = true;
    }

    //Check for remove items removed from DB
    if(!SYS->selDB().empty()) {
	vector<string> it_ls;
	list(it_ls);
	for(unsigned i_it = 0; i_it < it_ls.size(); i_it++)
	    if(itReg.find(it_ls[i_it]) == itReg.end())	del(it_ls[i_it]);
    }
}

void TPrmTmplLib::save_( )
{
    SYS->db().at().dataSet(DB()+"."+owner().tmplLibTable(),owner().nodePath()+"tmplib",*this);
}

void TPrmTmplLib::start( bool val )
{
    bool isErr = false;
    vector<string> lst;
    list(lst);
    for(unsigned iF = 0; iF < lst.size(); iF++)
	try{ at(lst[iF]).at().setStart(val); }
	catch(TError &err) {
	    mess_err(err.cat.c_str(), "%s", err.mess.c_str());
	    mess_sys(TMess::Error, _("Error starting the template '%s'."), lst[iF].c_str());
	    isErr = true;
	}

    run_st = val;

    if(isErr)	throw err_sys(_("Error starting some templates."));
}

void TPrmTmplLib::add( const string &id, const string &name )
{
    chldAdd(m_ptmpl, new TPrmTempl(id,name));
}

TVariant TPrmTmplLib::objFuncCall( const string &iid, vector<TVariant> &prms, const string &user )
{
    //Configuration functions call
    TVariant cfRez = objFunc(iid, prms, user);
    if(!cfRez.isNull()) return cfRez;

    return TCntrNode::objFuncCall(iid, prms, user);
}

void TPrmTmplLib::cntrCmdProc( XMLNode *opt )
{
    //Get page info
    if(opt->name() == "info") {
	TCntrNode::cntrCmdProc(opt);
	ctrMkNode("oscada_cntr",opt,-1,"/",_("Parameter templates library: ")+id(),RWRWR_,"root",SDAQ_ID);
	if(ctrMkNode("branches",opt,-1,"/br","",R_R_R_))
	    ctrMkNode("grp",opt,-1,"/br/tmpl_",_("Template"),RWRWR_,"root",SDAQ_ID,2,"idm",OBJ_NM_SZ,"idSz",OBJ_ID_SZ);
	if(ctrMkNode("area",opt,-1,"/lib",_("Library"))) {
	    if(ctrMkNode("area",opt,-1,"/lib/st",_("State"))) {
		ctrMkNode("fld",opt,-1,"/lib/st/st",_("Accessing"),RWRWR_,"root",SDAQ_ID,1,"tp","bool");
		ctrMkNode("fld",opt,-1,"/lib/st/db",_("Library DB"),RWRWR_,"root",SDAQ_ID,4,
		    "tp","str","dest","sel_ed","select",("/db/tblList:tmplib_"+id()).c_str(),
		    "help",_("DB address in format \"{DB module}.{DB name}.{Table name}\".\nSet '*.*.{Table name}' for use the main work DB."));
		ctrMkNode("fld",opt,-1,"/lib/st/timestamp",_("Date of modification"),R_R_R_,"root",SDAQ_ID,1,"tp","time");
	    }
	    if(ctrMkNode("area",opt,-1,"/lib/cfg",_("Configuration"))) {
		ctrMkNode("fld",opt,-1,"/lib/cfg/ID",_("Identifier"),R_R_R_,"root",SDAQ_ID,1,"tp","str");
		ctrMkNode("fld",opt,-1,"/lib/cfg/NAME",_("Name"),RWRWR_,"root",SDAQ_ID,2,"tp","str","len",OBJ_NM_SZ);
		ctrMkNode("fld",opt,-1,"/lib/cfg/DESCR",_("Description"),RWRWR_,"root",SDAQ_ID,3,"tp","str","cols","100","rows","3");
	    }
	}
	if(ctrMkNode("area",opt,-1,"/tmpl",_("Parameter templates")))
	    ctrMkNode("list",opt,-1,"/tmpl/tmpl",_("Templates"),RWRWR_,"root",SDAQ_ID,5,
		"tp","br","idm",OBJ_NM_SZ,"s_com","add,del","br_pref","tmpl_","idSz",OBJ_ID_SZ);
	return;
    }

    //Process command to page
    string a_path = opt->attr("path");
    if(a_path == "/lib/st/st") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(startStat() ? "1" : "0");
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	start(s2i(opt->text()));
    }
    else if(a_path == "/lib/st/db") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(fullDB());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	setFullDB(opt->text());
    }
    else if(a_path == "/lib/st/timestamp" && ctrChkNode(opt)) {
	vector<string> tls;
	list(tls);
	time_t maxTm = 0;
	for(unsigned i_t = 0; i_t < tls.size(); i_t++) maxTm = vmax(maxTm, at(tls[i_t]).at().timeStamp());
	opt->setText(i2s(maxTm));
    }
    else if(a_path == "/lib/cfg/ID" && ctrChkNode(opt))		opt->setText(id());
    else if(a_path == "/lib/cfg/NAME") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(name());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	setName(opt->text());
    }
    else if(a_path == "/lib/cfg/DESCR") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD))	opt->setText(descr());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))	setDescr(opt->text());
    }
    else if(a_path == "/br/tmpl_" || a_path == "/tmpl/tmpl") {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD)) {
	    vector<string> lst;
	    list(lst);
	    for(unsigned iF = 0; iF < lst.size(); iF++)
		opt->childAdd("el")->setAttr("id",lst[iF])->setText(at(lst[iF]).at().name());
	}
	if(ctrChkNode(opt,"add",RWRWR_,"root",SDAQ_ID,SEC_WR))	add(TSYS::strEncode(opt->attr("id"),TSYS::oscdID).c_str(),opt->text().c_str());
	if(ctrChkNode(opt,"del",RWRWR_,"root",SDAQ_ID,SEC_WR))	del(opt->attr("id").c_str(),true);
    }
    else TCntrNode::cntrCmdProc(opt);
}

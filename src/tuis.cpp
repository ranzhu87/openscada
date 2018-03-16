
//OpenSCADA system file: tuis.cpp
/***************************************************************************
 *   Copyright (C) 2003-2015 by Roman Savochenko, <rom_as@oscada.org>      *
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tsys.h"
#include "tmess.h"
#include "tuis.h"

#if HAVE_GD_FORCE
# include <gd.h>
#endif

using namespace OSCADA;

//*************************************************
//* TUIS                                          *
//*************************************************
TUIS::TUIS( ) : TSubSYS(SUI_ID,_("User Interfaces"), true)
{
#if HAVE_GD_FORCE
    gdFTUseFontConfig(1);
#endif
}

string TUIS::optDescr( )
{
    return(_(
	"===================== Subsystem \"User interfaces\" options ===============\n\n"));
}

void TUIS::load_( )
{
    //Load parameters from command line
    if(s2i(SYS->cmdOpt("h")) || s2i(SYS->cmdOpt("help"))) fprintf(stdout, "%s", optDescr().c_str());

    //Load parameters from config-file
}

bool TUIS::icoPresent(const string &inm, string *tp)
{
    int hd = open(icoPath(inm).c_str(),O_RDONLY);
    if(hd != -1) {
	if(tp) *tp = "png";
	close(hd);
	return true;
    }
    return false;
}

string TUIS::icoGet(const string &inm, string *tp, bool retPath )
{
    int len, hd = -1;
    unsigned i_t;
    char buf[STR_BUF_LEN];
    string rez;
    char types[][5] = {"png","gif","jpg","jpeg"};

    for(i_t = 0; i_t < sizeof(types)/5; i_t++) {
	hd = open(icoPath(inm,types[i_t]).c_str(),O_RDONLY);
	if(hd != -1) break;
    }
    if(hd != -1) {
	if(tp) *tp = types[i_t];
	if(retPath) rez = icoPath(inm, types[i_t]);
	else while((len=read(hd,buf,sizeof(buf))) > 0) rez.append(buf,len);
	close(hd);
    }

    return rez;
}

string TUIS::icoPath( const string &ico, const string &tp )
{
    return SYS->icoDir()+"/"+ico+"."+tp;
}

string TUIS::mimeGet( const string &inm, const string &fDt, const string &orig )
{
    string prc = TSYS::strParse(orig,0,";"), stvl;

    //First init to empty orig
    if(prc.empty() || TSYS::pathLev(prc,1).empty())
	prc = "file/"+ ((inm.rfind(".")==string::npos)?"unknown":inm.substr(inm.rfind(".")+1));

    //Adjust to group for used and known ones
    stvl = TSYS::pathLev(prc, 1);
    const char *tvl = stvl.c_str();
    // Text
    if(strcasecmp(tvl,"txt") == 0)					prc = "text/plain";
    else if(strcasecmp(tvl,"xml") == 0)					prc = "text/xml";
    else if(strcasecmp(tvl,"html") == 0)				prc = "text/html";
    else if(strcasecmp(tvl,"css") == 0)					prc = "text/css";
    else if(strcasecmp(tvl,"js") == 0)					prc = "text/javascript";
    else if(strcasecmp(tvl,"sgml") == 0)				prc = "text/sgml";
    else if(strcasecmp(tvl,"docbook") == 0)				prc = "text/docbook";
    else if(strcasecmp(tvl,"csv") == 0)					prc = "text/csv";
    else if(strcasecmp(tvl,"diff") == 0)				prc = "text/diff";
    else if(strcasecmp(tvl,"log") == 0)					prc = "text/log";
    else if(strcasecmp(tvl,"rtf") == 0)					prc = "text/rtf";
    else if(strcasecmp(tvl,"ics") == 0)					prc = "text/calendar";
    else if(strcasecmp(tvl,"vcs") == 0)					prc = "text/vcalendar";
    else if(strcasecmp(tvl,"vcf") == 0 || strcasecmp(tvl,"vct") == 0)	prc = "text/vcard";
    // Images
    else if(strcasecmp(tvl,"png") == 0)					prc = "image/png";
    else if(strcasecmp(tvl,"jpg") == 0 || strcasecmp(tvl,"jpeg") == 0)	prc = "image/jpg";
    else if(strcasecmp(tvl,"gif") == 0)					prc = "image/gif";
    else if(strcasecmp(tvl,"tif") == 0 || strcasecmp(tvl,"tiff") == 0)	prc = "image/tiff";
    else if(strcasecmp(tvl,"xpm") == 0)					prc = "image/xpm";
    else if(strcasecmp(tvl,"ico") == 0)					prc = "image/ico";
    else if(strcasecmp(tvl,"pcx") == 0)					prc = "image/pcx";
    else if(strcasecmp(tvl,"bmp") == 0)					prc = "image/bmp";
    else if(strcasecmp(tvl,"svg") == 0 || strcasecmp(tvl,"svg+xml") == 0)
	prc = string("image/") + ((fDt.find("<?xml ") == string::npos) ? "svg" : "svg+xml");
    // Audio
    else if(strcasecmp(tvl,"wav"))	prc = "audio/wav";
    else if(strcasecmp(tvl,"ogg"))	prc = "audio/ogg";
    else if(strcasecmp(tvl,"mp2"))	prc = "audio/mp2";
    else if(strcasecmp(tvl,"mp3"))	prc = "audio/mp3";
    // Video
    else if(strcasecmp(tvl,"mng"))	prc = "video/mng";
    else if(strcasecmp(tvl,"ogm"))	prc = "video/ogm";
    else if(strcasecmp(tvl,"avi"))	prc = "video/avi";
    else if(strcasecmp(tvl,"mp4"))	prc = "video/mp4";
    else if(strcasecmp(tvl,"mpeg"))	prc = "video/mpeg";
    else if(strcasecmp(tvl,"mkv"))	prc = "video/matroska";

    return prc + ((stvl=TSYS::strParse(orig,1,";")).size()?";"+stvl:"");
}

void TUIS::cntrCmdProc( XMLNode *opt )
{
    //> Get page info
    if(opt->name() == "info")
    {
	TSubSYS::cntrCmdProc(opt);
	ctrMkNode("fld",opt,-1,"/help/g_help",_("Options help"),R_R___,"root",SUI_ID,3,"tp","str","cols","90","rows","10");
	return;
    }
    //> Process command to page
    string a_path = opt->attr("path");
    if(a_path == "/help/g_help" && ctrChkNode(opt,"get",R_R___,"root",SUI_ID))	opt->setText(optDescr());
    else TSubSYS::cntrCmdProc(opt);
}

//*************************************************
//* TUI                                           *
//*************************************************
TUI::TUI( const string &id ) : TModule(id), runSt(false)
{

}

void TUI::cntrCmdProc( XMLNode *opt )
{
    //> Get page info
    if( opt->name() == "info" )
    {
	TModule::cntrCmdProc(opt);
	if(ctrMkNode("area",opt,0,"/prm",_("User interface")))
	    if(ctrMkNode("area",opt,-1,"/prm/st",_("State")))
		ctrMkNode("fld",opt,-1,"/prm/st/r_st",_("Running"),RWRWR_,"root",SUI_ID,1,"tp","bool");
	return;
    }

    //> Process command to page
    string a_path = opt->attr("path");
    if(a_path == "/prm/st/r_st")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SUI_ID,SEC_RD))	opt->setText(runSt?"1":"0");
	if(ctrChkNode(opt,"set",RWRWR_,"root",SUI_ID,SEC_WR))	s2i(opt->text()) ? modStart() : modStop();
    }
    else TModule::cntrCmdProc(opt);
}

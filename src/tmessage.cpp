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

#include <sys/types.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <iconv.h>
#include <langinfo.h>
#include <getopt.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include "../config.h"
#include "terror.h"
#include "tsys.h"
#include "tarchives.h"
#include "tmessage.h"

const char *TMessage::o_name = "TMessage";

TMessage::TMessage(  ) : IOCharSet("UTF8"), m_d_level(0), log_dir(2), head_buf(0)
{
    openlog(PACKAGE,0,LOG_USER);
    setlocale(LC_ALL,"");
    IOCharSet = nl_langinfo(CODESET);

    bindtextdomain(PACKAGE,LOCALEDIR);
    textdomain(PACKAGE);	    
 
    m_res = ResAlloc::resCreate( );
    mess_buf_len( 10 );
}


TMessage::~TMessage(  )
{
    ResAlloc::resDelete( m_res );
    closelog();
}


// Debug level (m_d_level) may changed �������� 0-8 ������������:
// 0 - �� �������� ������� ��������� ������ 
// 8 - ������������ ������� �������
// ������� ��������� (level) ������������� ��� �������������� � ���������� � �������� 0-7:
// 0 - ���������� �������;
// 7 - ������� ���������� ������� ������;
void TMessage::put( const string &categ, int level, char *fmt,  ... )
{
    char str[STR_BUF_LEN];
    va_list argptr;

    va_start (argptr,fmt);
    vsnprintf(str,sizeof(str),fmt,argptr);
    va_end(argptr);
    put_s( categ, level, str );
}

void TMessage::put_s( const string &categ, int level, const string &mess )
{
    if(level<0) level=0; if(level>7) level=7;
    if(level>=(8-m_d_level)) 
    {
	int level_sys=LOG_DEBUG;
	if(level<1)       level_sys=LOG_DEBUG;
	else if(level==1) level_sys=LOG_INFO;
	else if(level==2) level_sys=LOG_NOTICE;
	else if(level==3) level_sys=LOG_WARNING;
	else if(level==4) level_sys=LOG_ERR;
	else if(level==5) level_sys=LOG_CRIT;
	else if(level==6) level_sys=LOG_ALERT;
	else if(level==7) level_sys=LOG_EMERG;
	string s_mess = categ + "| " + mess;
	if(log_dir&1) syslog(level_sys,s_mess.c_str());
	if(log_dir&2) fprintf(stdout,"%s \n",s_mess.c_str());
	if(log_dir&4) fprintf(stderr,"%s \n",s_mess.c_str());
    }
    //Put to message buffer
    ResAlloc res(m_res,true);
    m_buf[head_buf].time  = time(NULL);
    m_buf[head_buf].categ = categ;
    m_buf[head_buf].level = level;
    m_buf[head_buf].mess  = mess;
    if( ++head_buf >= m_buf.size() ) head_buf = 0;    
}

void TMessage::get( time_t b_tm, time_t e_tm, vector<TMessage::SRec> & recs, const string &category, char level )
{
    recs.clear();
    
    ResAlloc res(m_res,false);
    int i_buf = head_buf;
    while(true)
    {
	if( m_buf[i_buf].time >= b_tm && m_buf[i_buf].time != 0 && m_buf[i_buf].time < e_tm &&
		( !category.size() || category == m_buf[i_buf].categ ) && m_buf[i_buf].level >= level )
	    recs.push_back(m_buf[i_buf]);
	if( ++i_buf >= m_buf.size() ) i_buf = 0;
    	if(i_buf == head_buf) break;	    
    }
}

string TMessage::lang( )
{
    return( setlocale(LC_MESSAGES,NULL) );
}

void TMessage::lang( const string &lng )
{
    if( setlocale(LC_MESSAGES,lng.c_str()) == NULL ) throw TError("(%s) Lang %s error!",o_name,lng.c_str());    
    IOCharSet = nl_langinfo(CODESET);
}

string TMessage::Sconv( const string &fromCH, const string &toCH, const string &mess)
{
    //Make convert to blocks 100 bytes !!!    
    string buf = ""; 
    char   *ibuf, outbuf[100], *obuf;
    size_t ilen, olen;
    iconv_t hd;
    
    hd = iconv_open(toCH.c_str(), fromCH.c_str());
    if( hd == (iconv_t)(-1) ) return("Error iconv");
    
    ibuf = (char *)mess.c_str();
    ilen = mess.size();
    
    while(ilen)
    {
	obuf = outbuf;
	olen = sizeof(outbuf)-1;
	iconv(hd,&ibuf,&ilen,&obuf,&olen);
	buf.append(outbuf,sizeof(outbuf)-1-olen);
    }
    iconv_close(hd);
    
    return(buf);
}

char *TMessage::I18N( char *mess, char *d_name )
{
    return( dgettext(d_name, mess) );
}

void TMessage::checkCommandLine( )
{
#if OSC_DEBUG
    Mess->put("DEBUG",MESS_INFO,"(%s)Read commandline options!",o_name);
#endif

    int i,next_opt;
    char *short_opt="hd:";
    struct option long_opt[] =
    {
	{"help"     ,0,NULL,'h'},
	{"debug"    ,1,NULL,'d'},
	{"log"      ,1,NULL,'l'},
	{NULL       ,0,NULL,0  }
    };

    optind=opterr=0;
    do
    {
	next_opt=getopt_long(SYS->argc,(char * const *)SYS->argv,short_opt,long_opt,NULL);
	switch(next_opt)
	{
	    case 'd': i = atoi(optarg); if(i>=0&&i<=8) d_level(i); break;
	    case 'l': log_direct(atoi(optarg)); break;
	    case -1 : break;
	}
    } while(next_opt != -1);
    
#if OSC_DEBUG
    Mess->put("DEBUG",MESS_DEBUG,"(%s)Read commandline options ok!",o_name);
#endif
}

void TMessage::updateOpt()
{
#if OSC_DEBUG
    Mess->put("DEBUG",MESS_INFO,"(%s)Read config options!",o_name);
#endif

    string opt;

    try
    {
	int i = atoi(SYS->cfgNode()->childGet("id","debug")->text().c_str());
	if( i >= 0 && i <= 8 ) d_level(i);
    }catch(...) {  }
    try{ log_direct(atoi(SYS->cfgNode()->childGet("id","target_log")->text().c_str())); }
    catch(...) { }
    try{ mess_buf_len( atoi( SYS->cfgNode()->childGet("id","mess_buf")->text().c_str() ) ); }
    catch(...) { }    
    
#if OSC_DEBUG
    Mess->put("DEBUG",MESS_INFO,"(%s)Read config options ok!",o_name);
#endif
}

void TMessage::mess_buf_len(int len)
{
    ResAlloc res(m_res,true);
    while( m_buf.size() > len )
    {
	m_buf.erase( m_buf.begin() + head_buf );
	if( head_buf >= m_buf.size() ) head_buf = 0;
    }
    while( m_buf.size() < len )
	m_buf.insert( m_buf.begin() + head_buf, TMessage::SRec() );
}


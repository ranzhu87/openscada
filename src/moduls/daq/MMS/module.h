
//OpenSCADA system module DAQ.MMS file: module.h
/***************************************************************************
 *   Copyright (C) 2013-2014 by Roman Savochenko                           *
 *   rom_as@oscada.org, rom_as@fromru.com                                  *
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

#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>

#include <tcontroller.h>
#include <ttipdaq.h>
#include <tparamcontr.h>

#include "libMMS/libMMS.h"
using namespace MMS;

#undef _
#define _(mess) mod->I18N(mess)

using std::string;
using std::vector;
using namespace OSCADA;

namespace ModMMS
{

//*************************************************
//* ModMMS::TMdPrm                                *
//*************************************************
class TMdContr;

class TMdPrm : public TParamContr
{
    public:
	//Methods
	TMdPrm( string name, TTipParam *tp_prm );
	~TMdPrm( );

	string varList( )			{ return cfg("VAR_LS").getS(); }
	void setVarList( const string &vl )	{ cfg("VAR_LS").setS(vl); }

	TElem &elem( )				{ return p_el; }

	void enable( );
	void disable( );

	void attrPrc( XML_N *iVal = NULL, vector<string> *iAls = NULL, const string &vid = "" );
	void getVals( );
	void setEval( );

	TMdContr &owner( );

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	//Methods
	void postEnable( int flag );
	void cntrCmdProc( XMLNode *opt );
	void vlArchMake( TVal &val );
	void vlSet( TVal &vo, const TVariant &vl, const TVariant &pvl );

	//Attributes
	TElem	p_el;	//Work atribute elements
};

//*************************************************
//* ModMMS::TMdContr                              *
//*************************************************
class TMdContr: public TController, public Client
{
    friend class TMdPrm;
    public:
	//Data
	struct StackTp
	{
	    StackTp( TArrayObj *iarr, XML_N *ivl, int iInPos ) : arr(iarr), vl(ivl), inPos(iInPos) { }

	    TArrayObj *arr;
	    XML_N *vl;
	    int inPos;
	};

	//Methods
	TMdContr( string name_c, const string &daq_db, ::TElem *cfgelem );
	~TMdContr( );

	string	getStatus( );

	int64_t	period( )	{ return mPer; }
	string  cron( )		{ return mSched; }
	int	prior( )	{ return mPrior; }
	double  syncPer( )	{ return mSync; }
	string	addr( )		{ return mAddr; }

	AutoHD<TMdPrm> at( const string &nm )	{ return TController::at(nm); }

	// MMS Client methods
	uint16_t COTP_DestTSAP( ){ return cfg("COTP_DestTSAP").getI(); }
	void regVar( const string &vl );

	void reqService( XML_N &io );
	void protIO( XML_N &io );
	int messIO( const char *obuf, int len_ob, char *ibuf = NULL, int len_ib = 0 );
	void debugMess( const string &mess );

	Res &nodeRes( )		{ return cntrRes; }

    protected:
	//Methods
	void prmEn( TMdPrm *prm, bool val );

	void enable_( );
	void disable_( );
	void start_( );
	void stop_( );

	bool cfgChange( TCfg &cfg );
	void cntrCmdProc( XMLNode *opt );       //Control interface command process

    private:
	//Methods
	TParamContr *ParamAttach( const string &name, int type );
	static void *Task( void *icntr );

	//Attributes
	Res	enRes, reqRes, cntrRes;	//Resource for enable params, requests, controller DAQ API
	TCfg	&mSched,	//Schedule
		&mPrior,	//Process task priority
		&mSync,		//Synchronization inter remote station: attributes list update.
		&mAddr,		//MMS server address
		&mVarsRdReq;	//Variables into single request
	int64_t	mPer;

	bool	prcSt,		//Process task active
		callSt,		//Calc now stat
		isReload;

	vector< AutoHD<TMdPrm> > pHD;

	double	tmGath;		//Gathering time
	float	tmDelay;	//Delay time for next try connect

	AutoHD<TTransportOut>	tr;
	map<string, TVariant>	mVars;
};

//*************************************************
//* ModMMS::TTpContr                              *
//*************************************************
class TTpContr: public TTipDAQ
{
    public:
	//Methods
	TTpContr( string name );
	~TTpContr( );

    protected:
	//Methods
	void postEnable( int flag );

	void load_( );
	void save_( );

	bool redntAllow( )	{ return true; }

    private:
	//Methods
	TController *ContrAttach( const string &name, const string &daq_db );
	void cntrCmdProc( XMLNode *opt );       //Control interface command process
};

extern TTpContr *mod;

} //End namespace ModMMS

#endif //MODULE_H

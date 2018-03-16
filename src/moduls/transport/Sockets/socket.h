
//OpenSCADA system module Transport.Sockets file: socket.h
/***************************************************************************
 *   Copyright (C) 2003-2017 by Roman Savochenko, <rom_as@oscada.org>      *
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

#ifndef SOCKET_H
#define SOCKET_H

#include <pthread.h>

#include <ttransports.h>

#undef _
#define _(mess) mod->I18N(mess)

#define S_NM_TCP  "TCP"
#define S_NM_UDP  "UDP"
#define S_NM_UNIX "UNIX"

#define SOCK_TCP  0
#define SOCK_UDP  1
#define SOCK_UNIX 2

using namespace OSCADA;

namespace Sockets
{

class TSocketIn;

//************************************************
//* Sockets::SSockIn				 *
//************************************************
class SSockIn
{
    public:
	SSockIn( TSocketIn *is, int isock, const string &isender ) :
	    pid(0), sock(isock), sender(isender), tmCreate(time(NULL)), tmReq(time(NULL)),
	    trIn(0), trOut(0), prcTm(0), prcTmMax(0), clntDetchCnt(0), s(is)	{ }

	pthread_t pid;		//Client's thread id
	int	sock;
	string	sender;
	time_t	tmCreate, tmReq;
	uint64_t trIn, trOut;	//Traffic in and out counters
	float	prcTm, prcTmMax, clntDetchCnt;

	TSocketIn	*s;
};

//************************************************
//* Sockets::TSocketIn				 *
//************************************************
class TSocketIn: public TTransportIn
{
    public:
	/* Open input socket <name> for locale <address>
	 * address : <type:<specific>>
	 * type:
	 *   TCP  - TCP socket with  "UDP:{host}:{port}"
	 *   UDP  - UDP socket with  "TCP:{host}:{port}"
	 *   UNIX - UNIX socket with "UNIX:{path}"
	 */
	TSocketIn( string name, const string &idb, TElem *el );
	~TSocketIn( );

	string getStatus( );

	int lastConn( )			{ return connTm; }
	unsigned MSS( )			{ return mMSS; }
	unsigned maxQueue( )		{ return mMaxQueue; }
	unsigned maxFork( )		{ return mMaxFork; }
	unsigned maxForkPerHost( )	{ return mMaxForkPerHost; }
	unsigned bufLen( )		{ return mBufLen; }
	unsigned keepAliveReqs( )	{ return mKeepAliveReqs; }
	unsigned keepAliveTm( )		{ return mKeepAliveTm; }
	int taskPrior( )		{ return mTaskPrior; }

	void setMSS( unsigned vl )	{ mMSS = vl ? vmax(100,vmin(1000000,vl)) : 0; modif(); }
	void setMaxQueue( unsigned vl )	{ mMaxQueue = vmax(1,vmin(100,vl)); modif(); }
	void setMaxFork( unsigned vl )	{ mMaxFork = vmax(1,vmin(1000,vl)); modif(); }
	void setMaxForkPerHost( unsigned vl )	{ mMaxForkPerHost = vmin(1000, vl); modif(); }
	void setBufLen( unsigned vl )	{ mBufLen = vmax(1,vmin(1024,vl)); modif(); }
	void setKeepAliveReqs( unsigned vl )	{ mKeepAliveReqs = vl; modif(); }
	void setKeepAliveTm( unsigned vl )	{ mKeepAliveTm = vl; modif(); }
	void setTaskPrior( int vl )	{ mTaskPrior = vmax(-1,vmin(199,vl)); modif(); }

	void start( );
	void stop( );
	unsigned forksPerHost( const string &sender );

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	//Methods
	static void *Task( void* );
	static void *ClTask( void* );

	bool prtInit( AutoHD<TProtocolIn> &prot_in, int sock, const string &sender, bool noex = false );
	void messPut( int sock, string &request, string &answer, const string &sender, AutoHD<TProtocolIn> &prot_in );

	void clientReg( SSockIn *so );
	void clientUnreg( SSockIn *so );

	void cntrCmdProc( XMLNode *opt );	//Control interface command process

	//Attributes
	int		sockFd;
	ResMtx		sockRes;

	bool		endrun;			//Command for stop task
	bool		endrunCl;		//Command for stop client tasks

	int		type;			//Socket's types
	string		path;			//Path to file socket for UNIX socket
	string		host;			//Host for TCP/UDP sockets
	string		port;			//Port for TCP/UDP sockets
	int		mode;			//Mode for TCP/UNIX sockets (0 - no hand; 1 - hand connect)

	unsigned short	mMSS,			//MSS
			mMaxQueue,		//Max queue for TCP, UNIX sockets
			mMaxFork,		//Maximum forking (opened sockets)
			mMaxForkPerHost,	//Maximum forking (opened sockets), per host
			mBufLen,		//Input buffer length
			mKeepAliveReqs,		//KeepAlive requests
			mKeepAliveTm;		//KeepAlive timeout
	int		mTaskPrior;		//Requests processing task prioritet

	bool		clFree;			//Clients stopped
	map<int, SSockIn*> clId;		//Client's control object, by sockets FD
	map<string, int> clS;			//Clients (senders) counters

	// Status atributes
	uint64_t	trIn, trOut;		// Traffic in and out counter
	float		prcTm, prcTmMax, clntDetchCnt;
	int		connNumb, connTm, clsConnByLim;	// Connections number
};

//************************************************
//* Sockets::TSocketOut				 *
//************************************************
class TSocketOut: public TTransportOut
{
    public:
	/* Open output socket <name> for locale <address>
	 * address : <type:<specific>>
	 * type:
	 *   TCP  - TCP socket with  "UDP:{host}:{port}"
	 *   UDP  - UDP socket with  "TCP:{host}:{port}"
	 *   UNIX - UNIX socket with "UNIX:{path}"
	 */
	TSocketOut( string name, const string &idb, TElem *el );
	~TSocketOut( );

	string getStatus( );

	string timings( )		{ return mTimings; }
	unsigned MSS( )			{ return mMSS; }
	int tmCon( )			{ return mTmCon; }

	void setTimings( const string &vl );
	void setMSS( unsigned vl )	{ mMSS = vl ? vmax(100,vmin(1000000,vl)) : 0; modif(); }
	void setTmCon( int vl )		{ mTmCon = vmax(1,vmin(60000,vl)); }

	void start( int time = 0 );
	void stop( );

	int messIO( const char *oBuf, int oLen, char *iBuf = NULL, int iLen = 0, int time = 0 );

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	//Methods
	void cntrCmdProc( XMLNode *opt );	//Control interface command process

	//Attributes
	string		mTimings;
	unsigned short	mMSS,			//MSS
			mTmCon,
			mTmNext,
			mTmRep;

	int		sockFd;

	int		type;			// socket's types
	struct sockaddr_in	nameIn;
	struct sockaddr_un	nameUn;

	// Status atributes
	uint64_t	trIn, trOut;			// Traffic in and out counter
	float		respTm, respTmMax;
	int64_t		mLstReqTm;
};

//************************************************
//* Sockets::TTransSock				 *
//************************************************
class TTransSock: public TTipTransport
{
    public:
	TTransSock( string name );
	~TTransSock( );

	TTransportIn  *In( const string &name, const string &idb );
	TTransportOut *Out( const string &name, const string &idb );

    protected:
	void load_( );

    private:
	//Methods
	void postEnable( int flag );
};

extern TTransSock *mod;
}

#endif //SOCKET_H

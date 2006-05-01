
//OpenSCADA system file: tarchval.h
/***************************************************************************
 *   Copyright (C) 2006-2006 by Roman Savochenko                           *
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

#ifndef TArchVal_H
#define TArchVal_H

#include <string>
#include <vector>

using std::string;
using std::vector;

#define BUF_ARCH_NM "<bufer>"

//================================================================
//===================== Value archivator =========================
//================================================================

//========================= TValBuf ==============================
//Value buffer object. Contein a massif of values types: bool, decimal, 
//real and string.
class TValBuf
{
    public:	
	TValBuf( );
	TValBuf( TFld::Type vtp, int isz, long long ipr, bool ihgrd = false, bool ihres = false );
	~TValBuf( );
	
	void clear();

	TFld::Type valType()	{ return m_val_tp; }
	bool hardGrid()		{ return m_hrd_grd; }
	bool highResTm()	{ return m_hg_res_tm; }
        int size()		{ return m_size; }
        long long period()	{ return m_per; }

	long long end()        	{ return m_end; }
        long long begin()       { return m_beg; }

	void valType( TFld::Type vl );
	void hardGrid( bool vl );
	void highResTm( bool vl );
	void size( int vl );
	void period( long long vl );
	bool vOK( long long ibeg, long long iend );

	//Get value
	virtual void getVal( TValBuf &buf, long long beg = 0, long long end = 0 );
	virtual string getS( long long *tm = NULL, bool up_ord = false );
	virtual double getR( long long *tm = NULL, bool up_ord = false );
	virtual int    getI( long long *tm = NULL, bool up_ord = false );
	virtual char   getB( long long *tm = NULL, bool up_ord = false );

	//Set value
	virtual void setVal( TValBuf &buf, long long beg = 0, long long end = 0 );
	virtual void setS( const string &value, long long tm = 0 );
        virtual void setR( double value, long long tm = 0 );
        virtual void setI( int value, long long tm = 0 );
        virtual void setB( char value, long long tm = 0 );	

    protected:
	void makeBuf( TFld::Type v_tp, int isz, long long ipr, bool hd_grd, bool hg_res );	//Create new or change buffer mode (all data into buffer will lost)
	
    private:
        template <class TpVal> class TBuf
	{
	    public:
		TBuf( TpVal eval, int &isz, long long &ipr, bool &ihgrd, bool &ihres, long long& iend, long long& ibeg );
		~TBuf();
		
		void clear();

		TpVal get( long long *tm = NULL, bool up_ord = false );
		void  set( TpVal value, long long tm = 0 );
		
		//Create new or change buffer mode (all data into buffer will lost) 
		void makeBuf( int isz, long long ipr, bool hd_grd, bool hg_res );	
		
	    private:
		bool	&hg_res_tm,	
			&hrd_grd;	
		long long &end, &beg,
			&per;  	
		int	&size,
			cur;		//Curent position.
		TpVal	eval;		//Error element value
		
		struct SLw  { time_t tm; TpVal val; };
		struct SHg  { long long tm; TpVal val; };
		union
		{
		    vector<TpVal> 	*grid;
                    vector<SLw>		*tm_low;
		    vector<SHg>     	*tm_high;
		}buf;
	};
	
	int     	hd_res;         //Access resource header
	TFld::Type	m_val_tp;	//Store values type
	union
	{
	    TBuf<char>	*bl;
	    TBuf<int>	*dec;
	    TBuf<double>*real;
	    TBuf<string>*str;
	} buf;
	
	bool	m_hg_res_tm,	//High resolution time use (microseconds)
		m_hrd_grd;	//Set hard griding. It mode no support the zero periodic.
	long long m_end, m_beg,	//Time of buffer begin and end (64 bit usec)
		m_per;  	//Periodic grid value (usec). If value = 0 then it no periodic buffer, error for time griding mode!
	int	m_size;		//Buffer size limit. 
	
};


//========================= TVArchive ============================
class TVal;
class TArchiveS;
class TVArchEl;

class TVArchive : public TCntrNode, public TValBuf, public TConfig
{
    public:
        enum SrcMode { Passive, PassiveAttr, ActiveAttr };
	
	//Methods
	TVArchive( const string &iid, const string &idb, TElem *cf_el );
	~TVArchive( );
	
	//- Base functions -
	const string &id()	{ return m_id; }
	string name();
	string dscr()		{ return m_dscr; }
	SrcMode srcMode() 	{ return (TVArchive::SrcMode)m_srcmode; }	
	bool toStart()  	{ return m_start; }
        bool startStat()	{ return run_st; }
	
	long long end( const string &arch = BUF_ARCH_NM );
        long long begin( const string &arch = BUF_ARCH_NM );		
	TFld::Type valType( )	{ return TValBuf::valType(); }
        bool hardGrid()         { return TValBuf::hardGrid(); }
        bool highResTm()        { return TValBuf::highResTm(); }
	int size()              { return TValBuf::size(); }
        long long period()      { return TValBuf::period(); }
	
	void name( const string &inm )	{ m_name = inm; }
        void dscr( const string &idscr ){ m_dscr = idscr; }
	void srcMode( SrcMode vl, const string &isrc = "" );
	void toStart( bool vl )	{ m_start = vl; }
	
	void valType( TFld::Type vl );
        void hardGrid( bool vl );
        void highResTm( bool vl );
        void size( int vl );
        void period( long long vl );
	
	//- Service -
	void load( );
        void save( );
	void start( );
	void stop( bool full_del = false );
	
	//- Get value -
	void getVal( TValBuf &buf, long long beg = 0, long long end = 0, const string &arch = "" );
        string getS( long long *tm = NULL, bool up_ord = false, const string &arch = "" );
        double getR( long long *tm = NULL, bool up_ord = false, const string &arch = "" );
        int    getI( long long *tm = NULL, bool up_ord = false, const string &arch = "" );
        char   getB( long long *tm = NULL, bool up_ord = false, const string &arch = "" );
	
	void setVal( TValBuf &buf, long long beg, long long end, const string &arch );
    	
	//- Active get data from atribute -
	void getActiveData();
	
	//- Phisical archivator's functions -
	void archivatorList( vector<string> &ls );
	bool archivatorPresent( const string &arch );
	void archivatorAttach( const string &arch );
	void archivatorDetach( const string &arch, bool full = false );
	void archivatorSort();
	
	string BD();
	
	TArchiveS &owner();
	
	//Attributes
	long long lstActValTm;
	
    private:
	//Methods
	void preDisable(int flag);
	void postDisable(int flag);
    
	void setUpBuf();
	string nodeName()	{ return m_id; }
	void cntrCmd_( const string &a_path, XMLNode *opt, TCntrNode::Command cmd );
    
	//Attributes
	int     a_res;
	bool	run_st;
	string  m_bd;
	//- Base params -
	string  &m_id, 		//ID
    		&m_name,       	//Name
		&m_dscr,	//Description
		&m_dsourc,	//Data source (parameter atribute)
		&m_archs;	//Use archivators list (separated whith ';')
        bool    &m_start;       //Starting flag
	int	&m_srcmode;	//Source mode
       	//- Buffer params -
	int	&m_vtype;	//Value type (int, real, bool, string)
	double	&m_bper;	//Buffer period
	int	&m_bsize;	//Buffer size
	bool	&m_bhgrd,	//Buffer use hard time griding
		&m_bhres;	//Buffer use high time resolution
	//- Request value params -
        static time_t 	m_beg, m_end;
	static int	m_ubeg, m_uend;
	static string	m_arch;	
	//- Mode params -
	AutoHD<TVal>	pattr_src;
	//- Phisical archive's elements -
	vector<TVArchEl *> arch_el;	//Links
};


//======================= TVArchivator ===========================
class TTipArchivator;

class TVArchivator : public TCntrNode, public TConfig
{
    friend void TVArchive::archivatorAttach( const string &arch );
    friend void TVArchive::archivatorDetach( const string &arch, bool full = false );

    public:
	TVArchivator( const string &iid, const string &idb, TElem *cf_el );
	~TVArchivator();

	const string &id() 	{ return m_id; }
	string name();
	string dscr()		{ return m_dscr; }
	string addr()		{ return m_addr; }
	double valPeriod()	{ return m_v_per; }
	int    archPeriod()	{ return m_a_per; }
	
	string workId();

	bool toStart()  	{ return m_start; }
        bool startStat()	{ return run_st; }	
	
	void name( const string &inm )	{ m_name = inm; }
        void dscr( const string &idscr ){ m_dscr = idscr; }
	virtual void valPeriod( double iper );
	virtual void archPeriod( int iper );
	
	virtual void load( );
	virtual void save( );
	virtual void start( );
	virtual void stop( bool full_del = false );
	
	//Place archive functions
	void archiveList( vector<string> &ls );
	bool archivePresent( const string &iid );

	string BD();
	
	TTipArchivator &owner();

    protected:
	//Methods
	//Protected place archive functions
	TVArchEl *archivePlace( TVArchive &item );
        void archiveRemove( const string &iid, bool full = false );
    
	virtual TVArchEl *getArchEl( TVArchive &arch );
    
	void cntrCmd_( const string &a_path, XMLNode *opt, TCntrNode::Command cmd );
	void postEnable();
	void preDisable(int flag);
	void postDisable(int flag);     //Delete all DB if flag 1	
	
	static void Task(union sigval obj);	//Process task

	//Attributes
	int	a_res;
        bool    run_st;
	//Phisical elements storing
	vector<TVArchEl *> arch_el;

    private:
	//Methods
        string nodeName()       { return m_id; }

	//Attributes
	string	&m_id,		//Var arch id
		&m_name,	//Var arch name
		&m_dscr,	//Var arch description
		&m_addr;	//Mess arch phisical address
	bool    &m_start;       //Var arch starting flag
	double	&m_v_per;	//Value period (sec)
	int	&m_a_per;	//Archivation period
	string  m_bd;
	//Archivate process
	double  tm_calc;	//Archiving time
	bool    prc_st;		//Process stat
	timer_t tmId;   	//Thread timer
};


//========================= TVArchEl =============================
class TVArchEl
{
    friend class TVArchivator;

    public:
	TVArchEl( TVArchive &iachive, TVArchivator &iarchivator );
	virtual ~TVArchEl();
	virtual void fullErase(){ }	
		
	virtual long long end()	{ return 0; }		//Archive end
	virtual long long begin(){ return 0; }		//Archive begin
	long long lastGet()	{ return m_last_get; }	//Last getted value time	
	
	virtual void getVal( TValBuf &buf, long long beg = 0, long long end = 0 ) { }
	virtual string getS( long long *tm, bool up_ord ) { }
        virtual double getR( long long *tm, bool up_ord ) { }
        virtual int    getI( long long *tm, bool up_ord ) { }
        virtual char   getB( long long *tm, bool up_ord ) { }
	
	virtual void setVal( TValBuf &buf, long long beg = 0, long long end = 0 ) { }
	
	TVArchive &archive();
	TVArchivator &archivator();
	
    private:
	TVArchive 	&m_achive;
	TVArchivator	&m_archivator;
	
	long long m_last_get;
};

#endif // TArchVal_H
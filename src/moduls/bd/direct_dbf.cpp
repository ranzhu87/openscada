/* Test Modul
** ==============================================================
*/

#include <getopt.h>
#include <string>

#include "../../tapplication.h"
#include "../../tmessage.h"
#include "dbf.h"
#include "direct_bd.h"

//============ Modul info! =====================================================
#define NAME_MODUL  "direct_bd"
#define NAME_TYPE   "BaseDate"
#define VERSION     "0.1"
#define AUTORS      "Roman Savochenko"
#define DESCRIPTION "Modul for direct use DB files *.dbf type, ver 3.0 !"
//==============================================================================

extern "C" TModule * attach( char *FName );

SExpFunc TDirectDB::ExpFuncLc[] = {
    {"OpenBD", ( void ( TModule::* )(  ) ) &TDirectDB::OpenBD, "int OpenBD( string name );",
     "Open BD <name>"},
    {"CloseBD", ( void ( TModule::* )(  ) ) &TDirectDB::CloseBD, "int CloseBD( int hdi );",
     "Close BD <hdi>"},
    {"GetCell1", ( void ( TModule::* )(  ) ) &TDirectDB::GetCell1, "int GetCell1( int hdi, int row, int line, string & cell);",
     "Get cell from BD <hdi>"},
    {"GetCell2", ( void ( TModule::* )(  ) ) &TDirectDB::GetCell2, "int GetCell2( int hdi, string row, int line, string & cell);",
     "Get cell from BD <hdi>"},
    {"SetCell1", ( void ( TModule::* )(  ) ) &TDirectDB::SetCell1, "int SetCell1( int hdi, int row, int line, const string & cell);",
     "Set cell to BD <hdi>"},
    {"SetCell2", ( void ( TModule::* )(  ) ) &TDirectDB::SetCell2, "int SetCell2( int hdi, string row, int line, const string & cell);",
     "Set cell to BD <hdi>"},
    {"NLines", ( void ( TModule::* )(  ) ) &TDirectDB::NLines, "int NLines( int hdi );",
     "Get number of lines into BD <hdi>"},
    {"NRows", ( void ( TModule::* )(  ) ) &TDirectDB::NRows, "int NRows( int hdi );",
     "Get number of rows into BD <hdi>"}

};


TDirectDB::TDirectDB( char *name ):TModule(  )
{
    NameModul = NAME_MODUL;
    NameType = NAME_TYPE;
    Vers = VERSION;
    Autors = AUTORS;
    DescrMod = DESCRIPTION;
    FileName = strdup( name );

    ExpFunc = ( SExpFunc * ) ExpFuncLc;
    NExpFunc = sizeof( ExpFuncLc ) / sizeof( SExpFunc );

    pathsBD.assign("./");

#if debug
    App->Mess->put( 1, "Run constructor %s file %s is OK!", NAME_MODUL, FileName );
#endif
}

TDirectDB::~TDirectDB(  )
{
#if debug
    App->Mess->put( 1, "Run destructor %s file %s is OK!", NAME_MODUL, FileName );
#endif
    free( FileName );
}

TModule *attach( char *FName )
{

    TDirectDB *self_addr = new TDirectDB( FName );
    return ( self_addr );
}

int TDirectDB::info( const string & name, string & info )
{
    info.erase(  );
    TModule::info( name, info );
    return ( MOD_NO_ERR );
}


void TDirectDB::pr_opt_descr( FILE * stream )
{
    fprintf( stream, 
    "-------------------- %s options --------------------------------------\n"
    "    --DirBDPath=<path>   Set dirs alocate BD files (*.dbf);\n"
    "\n", NAME_MODUL );
}

void TDirectDB::CheckCommandLine(  )
{
    int next_opt;
    char *short_opt = "h";
    struct option long_opt[] = {
	{"DirBDPath", 1 ,NULL ,'m'},
	{NULL, 0, NULL, 0}
    };

    optind = opterr = 0;
    do
    {
	next_opt = getopt_long( App->argc, ( char *const * ) App->argv, short_opt, long_opt, NULL );
	switch ( next_opt )
	{
	case 'h':
	    pr_opt_descr( stdout );
	    break;
	case 'm':
	    pathsBD.assign(optarg); 
	    break;
	case -1:
	    break;
	}
    }
    while ( next_opt != -1 );
}

int TDirectDB::init(  )
{
    CheckCommandLine(  );
    TModule::init(  );
    return ( MOD_NO_ERR );
}


int TDirectDB::OpenBD( string name )
{
    int    i;

    for(i=0; i < hd.size(); i++)
	if(hd[i]->path.compare(name) == 0) break;
    if(i < hd.size())
    {
    	hd[i]->use++; 
	return(i);
    }

    TBasaDBF *basa = new TBasaDBF(  );    
    if( basa->LoadFile( (char *)(pathsBD+'/'+name).c_str() ) == -1 )
    {
	delete basa;
	return(-1);
    }

    for(i=0; i < hd.size(); i++)
	if(hd[i]->use <= 0) break;
    if(i == hd.size())
    {
	Shd *hd_id = new Shd;
	hd.push_back(hd_id);
    }
    hd[i]->use = 1;
    hd[i]->path.assign(name);
    hd[i]->basa = basa;
	
    return ( i );
}

int TDirectDB::CloseBD( int hdi )
{
    if(hdi>=hd.size() || hd[hdi]->use <= 0 ) return(-1);
    if( --(hd[hdi]->use) > 0) return(0);
    
    if(hd[hdi]->basa != NULL) delete hd[hdi]->basa;
    hd[hdi]->use=0;
    hd[hdi]->path.erase();

    return(0);
}


int TDirectDB::GetCell1( int hdi, int row, int line, string & cell)
{
    if(hdi>=hd.size() || hd[hdi]->use <= 0 ) return(-1);
    int kz = hd[hdi]->basa->GetFieldIt( line, row, cell );

    return(kz);    
}

int TDirectDB::GetCell2( int hdi, string row, int line, string & cell)
{
    if(hdi>=hd.size() || hd[hdi]->use <= 0 ) return(-1);
    int kz = hd[hdi]->basa->GetFieldIt( line, (char *)row.c_str(), cell );

    return(kz);    
}

int TDirectDB::SetCell1( int hdi, int row, int line, const string & cell)
{
    if(hdi>=hd.size() || hd[hdi]->use <= 0 ) return(-1);
    int kz = hd[hdi]->basa->ModifiFieldIt( line, row, (char *)cell.c_str() );

    return(kz);    
}

int TDirectDB::SetCell2( int hdi, string row, int line, const string & cell)
{
    if(hdi>=hd.size() || hd[hdi]->use <= 0 ) return(-1);
    int kz = hd[hdi]->basa->ModifiFieldIt( line, (char *)row.c_str(), (char *)cell.c_str() );

    return(kz);    
}

int TDirectDB::NLines( int hdi )
{
    if(hdi>=hd.size() || hd[hdi]->use <= 0 ) return(0);
    return( hd[hdi]->basa->GetCountItems(  ) );
}

int TDirectDB::NRows( int hdi )
{
    int cnt=0;
    if(hdi>=hd.size() || hd[hdi]->use <= 0 ) return(0);
    while( hd[hdi]->basa->getField(cnt) != NULL ) cnt++;
    return( cnt );
}



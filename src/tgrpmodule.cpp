
#include <unistd.h>

#include "tkernel.h"
#include "tmessage.h"
#include "tmodule.h"
#include "tgrpmodule.h"

const char *TGRPModule::o_name = "TGRPModule";

TGRPModule::TGRPModule( TKernel *app, char *NameT ) : NameType(NameT), DirPath(""), owner(app)
{

}

TGRPModule::~TGRPModule(  )
{

}

void TGRPModule::InitAll( )
{
    for(unsigned i=0;i<Moduls.size();i++) 
	if(Moduls[i].stat == GRM_ST_OCCUP) Moduls[i].modul->init(NULL);
}

void TGRPModule::DeinitAll( )
{
    for(unsigned i=0;i<Moduls.size();i++) 
	if(Moduls[i].stat == GRM_ST_OCCUP) Moduls[i].modul->deinit();
}

void TGRPModule::List( vector<string> & moduls ) const
{
    moduls.clear();
    for(unsigned i=0;i < Size();i++) 
	if(Moduls[i].stat == GRM_ST_OCCUP) 
	    moduls.push_back(Moduls[i].name);
}

// Add modul 

int TGRPModule::AddM( TModule *modul )
{
    string NameMod, NameTMod;
    
    //---  Check names and version ---

    modul->info("NameType",NameTMod);
    modul->info("NameModul",NameMod);
    for(unsigned i=0;i < Moduls.size(); i++)
    {
	if( Moduls[i].stat == GRM_ST_FREE ) continue;
	if( Moduls[i].name == NameMod )
	{
	    int major, major1, minor, minor1;
	    modul->Version(major,minor);
    	    Moduls[i].modul->Version(major1,minor1);

	    if(major>major1 || (major==major1 && minor > minor1))
	    {
		delete Moduls[i].modul;
		Moduls[i].name  = NameMod;
		//Moduls[i].id_hd = -1;
		Moduls[i].modul = modul;
		Moduls[i].stat  = GRM_ST_OCCUP; 
#if OSC_DEBUG 
		Mess->put(0, "Update modul is ok!");
#endif	
		return(i);
	    }
	}
    }

    unsigned i;
    for( i=0 ;i < Moduls.size(); i++)
	if(Moduls[i].stat == GRM_ST_FREE ) break;
    if(i == Moduls.size()) Moduls.push_back( );
    Moduls[i].name     = NameMod;
    Moduls[i].modul    = modul;
    //Moduls[i].id_hd    = -1;
    Moduls[i].stat     = GRM_ST_OCCUP; 
#if OSC_DEBUG 
    Mess->put(0, "Add modul %s is ok! Type %s .",NameMod.c_str(),NameTMod.c_str());
#endif	
    return(i);
}

int TGRPModule::DelM( unsigned hd )
{
    if(hd >= Moduls.size() || Moduls[hd].stat == GRM_ST_FREE ) return(-1);
    Moduls[hd].stat = GRM_ST_FREE;

    return(0);
}

unsigned TGRPModule::NameToId(string name) const
{
    for(unsigned i=0; i<Size(); i++)
    {            
	if( Moduls[i].stat == GRM_ST_FREE ) continue;
	if( Moduls[i].name == name )        return(i);
    }
    throw TError("%s: no avoid modul %s!",o_name, name.c_str());
}

TModule *TGRPModule::FUse(unsigned int id, char * func, void (TModule::**offptr)())
{
    if(id >= Size() || Moduls[id].stat != GRM_ST_OCCUP )   throw TError("%s: no id module!",o_name);
    Moduls[id].modul->GetFunc(func, offptr);
    return(Moduls[id].modul);
}

void TGRPModule::FFree(unsigned int id, char * func)
{
    if(id >= Size() || Moduls[id].stat != GRM_ST_OCCUP ) throw TError("%s: no id module!",o_name);
    Moduls[id].modul->FreeFunc(func);
}

void TGRPModule::CheckCommandLineMods()
{
    for(unsigned i_m=0; i_m < Size(); i_m++)
	if( Moduls[i_m].stat == GRM_ST_OCCUP )
	    Moduls[i_m].modul->CheckCommandLine((char **)owner->argv,owner->argc);
}

void TGRPModule::UpdateOptMods()
{
    for(unsigned i_m=0; i_m < Size(); i_m++)
	if( Moduls[i_m].stat == GRM_ST_OCCUP )
	    Moduls[i_m].modul->UpdateOpt();	    
}


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

#include "tsys.h"
#include "tkernel.h"
#include "tparamcontr.h"
#include "tparams.h"
#include "tparam.h"

const char *TParam::o_name = "TParam";


TParam::TParam( SCntrS cntr, string name, TParamS *prms ) : 
	work(0), owner(prms)
{    
    TParam::name = name;
    hd_res = ResAlloc::ResCreate();
    Reg( cntr, name );
}

TParam::~TParam(  )
{
    ResAlloc res(hd_res,true);
    while(PrmC.size())
    {
	Owner().Owner().Controller().at(PrmC[0].c_hd).det(PrmC[0].p_hd);
	Owner().Owner().Controller().det(PrmC[0].c_hd);
	PrmC.erase(PrmC.begin());	
    }
    res.release();
    
    ResAlloc::ResDelete(hd_res);
}

int TParam::Reg( SCntrS cntr, string name )
{
    ResAlloc res(hd_res,true);
    //Check already registry parameters
    for(unsigned i_pr = 0; i_pr < PrmC.size(); i_pr++)
 	if( Owner().Owner().Controller().at(PrmC[i_pr].c_hd).Name() == cntr.obj &&
	    Owner().Owner().Controller().at(PrmC[i_pr].c_hd).Owner().mod_Name() == cntr.tp &&
	    Owner().Owner().Controller().at(PrmC[i_pr].c_hd).at(PrmC[i_pr].p_hd).Name() == name)
	    return( PrmC.size() );
    //Registry parameter
    SParam prm;
    prm.c_hd = Owner().Owner().Controller().att(cntr,o_name);
    try{ prm.p_hd = Owner().Owner().Controller().at(prm.c_hd).att(name,o_name); }
    catch(...)
    {
	Owner().Owner().Controller().det(prm.c_hd);
	throw;
    }
    PrmC.push_back(prm);

    return( PrmC.size() );
} 

int TParam::UnReg( SCntrS cntr, string name )
{
    TControllerS &contr = Owner().Owner().Controller();
    
    ResAlloc res(hd_res,true);
    //Check registry parameters
    for(unsigned i_pr = 0; i_pr < PrmC.size(); i_pr++)
 	if( contr.at(PrmC[i_pr].c_hd).Name() == cntr.obj &&
	    contr.at(PrmC[i_pr].c_hd).Owner().mod_Name() == cntr.tp &&
	    contr.at(PrmC[i_pr].c_hd).at(PrmC[i_pr].p_hd).Name() == name)
	{
	    contr.at(PrmC[i_pr].c_hd).det(PrmC[i_pr].p_hd);
	    contr.det(PrmC[i_pr].c_hd);
	    PrmC.erase(PrmC.begin()+i_pr);
	    break;	    	    
	}
    return( PrmC.size() );	    
}

TParamContr &TParam::at()
{    
    ResAlloc res(hd_res,false);
    TParamContr &c_prm = Owner().Owner().Controller().at(PrmC[work].c_hd).at(PrmC[work].p_hd);
    return( c_prm );      
}




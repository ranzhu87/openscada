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


#include <math.h>

#include <tsys.h>
#include "libcompl1.h"

using namespace StatFunc;

//Complex1 functions library
Complex1::Complex1( ) : TLibFunc("complex1")
{    
    reg( new DigitBlock() );
    reg( new Sum() );
    reg( new Mult() );
    reg( new MultDiv() );
    reg( new Exp() );
    reg( new Pow() );
    reg( new Cond1() );
    reg( new Cond2() );
    reg( new Cond3() );
    reg( new Select() );
    reg( new Increm() );
    reg( new Divider() );
    reg( new PID() );
}

Complex1::~Complex1()
{

}

string Complex1::name()
{
    return "Complex1 functions";
}

string Complex1::descr()
{
    return "Allow functions of SCADA system 'Complex1' for compatibility.";
}

//Complex1 functions:
//------------------------------------------------------------------------------------
//DigitBlock
//Support digital blocks: 0-Open;1-Start;2-Enable;3-Norm;4-Block
//------------------------------------------------------------------------------------ 
DigitBlock::DigitBlock() : TFunction("digitBlock")
{
    //Inputs
    ioAdd( new IO("cmdOpen","Command \"Open\"",IO_BOOL,"0") );
    ioAdd( new IO("cmdClose","Command \"Close\"",IO_BOOL,"0") );
    ioAdd( new IO("cmdStop","Command \"Stop\"",IO_BOOL,"0") );

    //Outputs
    ioAdd( new IO("stOpen","Stat \"Opened\"",IO_BOOL|IO_OUT,"0") );
    ioAdd( new IO("stClose","Stat \"Closed\"",IO_BOOL|IO_OUT,"0") );
    ioAdd( new IO("stOpen","Stat \"Opened\"",IO_BOOL|IO_OUT,"0") );
}

string DigitBlock::name()
{
    return "Digital block";
}

string DigitBlock::descr()
{
    return "Digital assemble block.";	//!!!! make full description
}

void DigitBlock::calc( TValFunc *val )
{    

}

//------------------------------------------------------------------------------------
//Simple summator
//Formula: out=in1_1*in1_2+in2_1*in2_2+in3_1*in3_2+in4_1*in4_2+
//    	       in5_1*in5_2+in6_1*in6_2+in7_1*in7_2+in8_1*in8_2;
//------------------------------------------------------------------------------------
Sum::Sum() : TFunction("sum")
{
    char id_buf[10], nm_buf[20];
    
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"1") );
    for( int i_in=1; i_in <= 8; i_in++ )
    {
	snprintf(id_buf,sizeof(id_buf),"in%d_1",i_in);
	snprintf(nm_buf,sizeof(nm_buf),"Input %d.1",i_in);
	ioAdd( new IO(id_buf,nm_buf,IO_REAL,"0") );
	snprintf(id_buf,sizeof(id_buf),"in%d_2",i_in);
	snprintf(nm_buf,sizeof(nm_buf),"Input %d.2",i_in);
	ioAdd( new IO(id_buf,nm_buf,IO_REAL,"0") );
    }
}

string Sum::name()
{
    return "Simple summator";
}

string Sum::descr()
{
    return "Simple summator per formula:\n"
	"out=in1_1*in1_2+in2_1*in2_2+in3_1*in3_2+in4_1*in4_2+\n"
	"    in5_1*in5_2+in6_1*in6_2+in7_1*in7_2+in8_1*in8_2;";
}

void Sum::calc( TValFunc *val )
{
    val->setR(0,
	    val->getR(1)*val->getR(2)+
	    val->getR(3)*val->getR(4)+
	    val->getR(5)*val->getR(6)+
	    val->getR(7)*val->getR(8)+
	    val->getR(9)*val->getR(10)+
	    val->getR(11)*val->getR(12)+
	    val->getR(13)*val->getR(14)+
	    val->getR(15)*val->getR(16));
}

//------------------------------------------------------------------------------------
//Simple moltiplicator
//Formula: out=(in1_1*in1_2*in1_3*in1_4*in1_5*in1_6)/(in2_1*in2_2*in2_3*in2_4);
//------------------------------------------------------------------------------------
Mult::Mult() : TFunction("mult")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"1") );
    for( int i_c = 1; i_c <= 6; i_c++ )
        ioAdd( new IO("in1_"+TSYS::int2str(i_c),"Input 1."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
}

string Mult::name()
{
    return "Simple multiplicator";
}

string Mult::descr()
{
    return "Simple moltiplicator per formula:\n"
	"out=(in1_1*in1_2*in1_3*in1_4*in1_5*in1_6)/(in2_1*in2_2*in2_3*in2_4);";
}

void Mult::calc( TValFunc *val )
{
    val->setR(0,
	    (val->getR(1)*val->getR(2)*val->getR(3)*val->getR(4)*val->getR(5)*val->getR(6))/
	    (val->getR(7)*val->getR(8)*val->getR(9)*val->getR(10)));
}

//------------------------------------------------------------------------------------
//Sumator + moltiplicator
//Formula: out=(in1_1*in1_2*in1_3*in1_4*in1_5*(in2_1*in2_2*in2_3*in2_4*in2_5+"
//			(in3_1*in3_2*in3_3*in3_4*in3_5)/(in4_1*in4_2*in4_3*in4_4*in4_5)))
//------------------------------------------------------------------------------------
MultDiv::MultDiv() : TFunction("multDiv")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in1_"+TSYS::int2str(i_c),"Input 1."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in3_"+TSYS::int2str(i_c),"Input 3."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
}

string MultDiv::name()
{
    return "Multiplicator+divider";
}

string MultDiv::descr()
{
    return "Multiplicator+divider per formula:\n"
	"out=in1_1*in1_2*in1_3*in1_4*in1_5*(in2_1*in2_2*in2_3*in2_4*in2_5+\n"
	"          (in3_1*in3_2*in3_3*in3_4*in3_5)/(in4_1*in4_2*in4_3*in4_4*in4_5));";
}

void MultDiv::calc( TValFunc *val )
{
    double tmp1 = val->getR(16)*val->getR(17)*val->getR(18)*val->getR(19)*val->getR(20);
    double tmp2 = val->getR(1)*val->getR(2)*val->getR(3)*val->getR(4)*val->getR(5);
    double tmp3 = val->getR(6)*val->getR(7)*val->getR(8)*val->getR(9)*val->getR(10);
    double tmp4 = val->getR(11)*val->getR(12)*val->getR(13)*val->getR(14)*val->getR(15);
    val->setR(0,tmp2*(tmp3+tmp4/tmp1));
}

//------------------------------------------------------------------------------------
//Expanenta
//Formula: out=exp (in1_1*in1_2*in1_3*in1_4*in1_5 + 
//                  (in2_1*in2_2*in2_3*in2_4*in2_5+in3) / (in4_1*in4_2*in4_3*in4_4*in4_5+in5) )
//------------------------------------------------------------------------------------
Exp::Exp() : TFunction("exp")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in1_"+TSYS::int2str(i_c),"Input 1."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    ioAdd( new IO("in3","Input 3",IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
    ioAdd( new IO("in5","Input 5",IO_REAL,"1") );
}

string Exp::name()
{
    return "Expanenta";
}

string Exp::descr()
{
    return "Expanenta per formula:\n"
	"out=exp (in1_1*in1_2*in1_3*in1_4*in1_5 +\n"
	"         (in2_1*in2_2*in2_3*in2_4*in2_5+in3) / (in4_1*in4_2*in4_3*in4_4*in4_5+in5) );";
}

void Exp::calc( TValFunc *v )
{
    double tmp1=v->getR(12)*v->getR(13)*v->getR(14)*v->getR(15)*v->getR(16)+v->getR(17);
    double tmp2=v->getR(1)*v->getR(2)*v->getR(3)*v->getR(4)*v->getR(5);
    double tmp3=v->getR(6)*v->getR(7)*v->getR(8)*v->getR(9)*v->getR(10)+v->getR(11);
    v->setR(0,exp(tmp2+tmp3/tmp1));
}

//------------------------------------------------------------------------------------
//Power
//Formula: out=(in1_1*in1_2*in1_3*in1_4*in1_5)^(in2_1*in2_2*in2_3*in2_4*in2_5 +
//    			(in3_1*in3_2*in3_3*in3_4*in3_5)/(in4_1*in4_2*in4_3*in4_4*in4_5))
//------------------------------------------------------------------------------------
Pow::Pow() : TFunction("pow")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in1_"+TSYS::int2str(i_c),"Input 1."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in3_"+TSYS::int2str(i_c),"Input 3."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
}

string Pow::name()
{
    return "Power";
}

string Pow::descr()
{
    return "Power per formula:\n"
	"out=(in1_1*in1_2*in1_3*in1_4*in1_5)^(in2_1*in2_2*in2_3*in2_4*in2_5 +\n"
	"	(in3_1*in3_2*in3_3*in3_4*in3_5)/(in4_1*in4_2*in4_3*in4_4*in4_5));";
}

void Pow::calc( TValFunc *v )
{
    double tmp1=v->getR(16)*v->getR(17)*v->getR(18)*v->getR(19)*v->getR(20);
    double tmp2=v->getR(6)*v->getR(7)*v->getR(8)*v->getR(9)*v->getR(10);
    double tmp3=v->getR(11)*v->getR(12)*v->getR(13)*v->getR(14)*v->getR(15);
    tmp2=tmp2+tmp3/tmp1;
    tmp3=v->getR(1)*v->getR(2)*v->getR(3)*v->getR(4)*v->getR(5);
    v->setR(0,pow(tmp3,tmp2));
}

//------------------------------------------------------------------------------------
//Condition <
//Formula: out=if( in1<(in2_1*in2_2*in2_3*in2_4) ) then in3_1*in3_2*in3_3*in3_4;
//             else in4_1*in4_2*in4_3*in4_4;
//------------------------------------------------------------------------------------
Cond1::Cond1() : TFunction("cond <")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    ioAdd( new IO("in1","Input 1",IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in3_"+TSYS::int2str(i_c),"Input 3."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
}

string Cond1::name()
{
    return "Condition '<'";
}

string Cond1::descr()
{
    return "Condition '<' per formula:\n"
	"out=if( in1<(in2_1*in2_2*in2_3*in2_4) ) then in3_1*in3_2*in3_3*in3_4;\n"
	"    else in4_1*in4_2*in4_3*in4_4;";
}

void Cond1::calc( TValFunc *v )
{
    double tmp1=v->getR(2)*v->getR(3)*v->getR(4)*v->getR(5);
    if( v->getR(1)<tmp1 ) 	v->setR(0,v->getR(6)*v->getR(7)*v->getR(8)*v->getR(9));
    else 			v->setR(0,v->getR(10)*v->getR(11)*v->getR(12)*v->getR(13));
}

//------------------------------------------------------------------------------------
//Condition >
//Formula: out=if( in1>(in2_1*in2_2*in2_3*in2_4) ) then in3_1*in3_2*in3_3*in3_4;
//             else in4_1*in4_2*in4_3*in4_4;
//------------------------------------------------------------------------------------
Cond2::Cond2() : TFunction("cond >")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    ioAdd( new IO("in1","Input 1",IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in3_"+TSYS::int2str(i_c),"Input 3."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
}

string Cond2::name()
{
    return "Condition '>'";
}

string Cond2::descr()
{
    return "Condition '>' per formula:\n"
	"out=if( in1>(in2_1*in2_2*in2_3*in2_4) ) then in3_1*in3_2*in3_3*in3_4;\n"
	"    else in4_1*in4_2*in4_3*in4_4;";    
}

void Cond2::calc( TValFunc *v )
{
    double tmp1=v->getR(2)*v->getR(3)*v->getR(4)*v->getR(5);
    if( v->getR(1)>tmp1 ) 	v->setR(0,v->getR(6)*v->getR(7)*v->getR(8)*v->getR(9));
    else 			v->setR(0,v->getR(10)*v->getR(11)*v->getR(12)*v->getR(13));
}

//------------------------------------------------------------------------------------
//Condition x>, x<, >x>
//Formula: out = if( in1<(in2_1*in2_2*in2_3*in2_4) )	then in3_1*in3_2*in3_3*in3_4;
//	         else if( in1>(in4_1*in4_2*in4_3*in4_4)	then in5_1*in5_2*in5_3*in5_4;
//	         else in6_1*in6_2*in6_3*in6_4;
//------------------------------------------------------------------------------------
Cond3::Cond3() : TFunction("cond_full")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    ioAdd( new IO("in1","Input 1",IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
	ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in3_"+TSYS::int2str(i_c),"Input 3."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in5_"+TSYS::int2str(i_c),"Input 5."+TSYS::int2str(i_c),IO_REAL,"1") );	
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in6_"+TSYS::int2str(i_c),"Input 6."+TSYS::int2str(i_c),IO_REAL,"1") );	
}

string Cond3::name()
{
    return "Full condition";
}

string Cond3::descr()
{
    return "Full condition per formula:\n"
	"out = if( in1<(in2_1*in2_2*in2_3*in2_4) )    then in3_1*in3_2*in3_3*in3_4;\n"
	"      else if( in1>(in4_1*in4_2*in4_3*in4_4) then in5_1*in5_2*in5_3*in5_4;\n"
	"      else in6_1*in6_2*in6_3*in6_4;";
}

void Cond3::calc( TValFunc *v )
{
    double tmp1=v->getR(2)*v->getR(3)*v->getR(4)*v->getR(5);
    double tmp2=v->getR(10)*v->getR(11)*v->getR(12)*v->getR(13);
    double tmp3=v->getR(6)*v->getR(7)*v->getR(8)*v->getR(9);
    double tmp4=v->getR(18)*v->getR(19)*v->getR(20)*v->getR(21);
    double tmp5=v->getR(14)*v->getR(15)*v->getR(16)*v->getR(17);
    if(v->getR(1)<tmp1) 	v->setR(0,tmp3);
    else if( v->getR(1)>tmp2)	v->setR(0,tmp5);
    else			v->setR(0,tmp4);
}

//------------------------------------------------------------------------------------
//Selector
//Formula: out = if( sel = 1 )	then in1_1*in1_2*in1_3*in1_4;
//		 if( sel = 2 )	then in2_1*in2_2*in2_3*in2_4;
//		 if( sel = 3 )	then in3_1*in3_2*in3_3*in3_4;
//		 if( sel = 4 )	then in4_1*in4_2*in4_3*in4_4;
//------------------------------------------------------------------------------------
Select::Select() : TFunction("select")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    ioAdd( new IO("sel","Select",IO_INT,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in1_"+TSYS::int2str(i_c),"Input 1."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in3_"+TSYS::int2str(i_c),"Input 3."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 4; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
}

string Select::name()
{
    return "Selector";
}

string Select::descr()
{
    return "Selector per formula:\n"
	"out = if( sel = 1 )  then in1_1*in1_2*in1_3*in1_4;\n"
        "      if( sel = 2 )  then in2_1*in2_2*in2_3*in2_4;\n"
        "      if( sel = 3 )  then in3_1*in3_2*in3_3*in3_4;\n"
        "      if( sel = 4 )  then in4_1*in4_2*in4_3*in4_4;";
}

void Select::calc( TValFunc *v )
{
    switch(v->getI(1))
    {
	case  1: v->setR(0,v->getR(2)*v->getR(3)*v->getR(4)*v->getR(5)); break;
	case  2: v->setR(0,v->getR(6)*v->getR(7)*v->getR(8)*v->getR(9)); break;
	case  3: v->setR(0,v->getR(10)*v->getR(11)*v->getR(12)*v->getR(13)); break;
	case  4: v->setR(0,v->getR(14)*v->getR(15)*v->getR(16)*v->getR(17)); break;
	default: v->setR(0,0.0);
    }
}

//------------------------------------------------------------------------------------
//Incrementator
//Formula: out = if( in > prev )then prev + k_pos*(in-prev); else prev - k_neg*(prev-in);
//------------------------------------------------------------------------------------
Increm::Increm() : TFunction("increment")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );
    ioAdd( new IO("in","Input",IO_REAL,"1") );
    ioAdd( new IO("prev","Previous",IO_REAL,"1") );
    ioAdd( new IO("k+","Positive koef",IO_REAL,"0.1") );
    ioAdd( new IO("k-","Negative koef",IO_REAL,"0.1") );
}

string Increm::name()
{
    return "Incrementator";
}

string Increm::descr()
{
    return "Incrementator per formula:\n"
	"out = if( in1 > in2 )then in2 + in3*(in1-in2); else in2 - in4*(in2-in1);";
}

void Increm::calc( TValFunc *v )
{
    if( v->getR(1) > v->getR(2) )
	v->setR(0,v->getR(2) + v->getR(3)*(v->getR(1) - v->getR(2)));
    else v->setR(0,v->getR(2) - v->getR(4)*(v->getR(2) - v->getR(1)));
}

//------------------------------------------------------------------------------------
//Divider
//Formula: out = (in1_1*in1_2*in1_3*in1_4*in1_5 + in2_1*in2_2*in2_3*in2_4*in2_5 + in3) / 
//		 (in4_1*in4_2*in4_3*in4_4*in4_5 + in5_1*in5_2*in5_3*in5_4*in5_5 + in6);
//------------------------------------------------------------------------------------
Divider::Divider() : TFunction("div")
{
    ioAdd( new IO("out","Output",IO_REAL|IO_RET,"0") );

    for( int i_c = 1; i_c <= 5; i_c++ )
	ioAdd( new IO("in1_"+TSYS::int2str(i_c),"Input 1."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in2_"+TSYS::int2str(i_c),"Input 2."+TSYS::int2str(i_c),IO_REAL,"1") );
    ioAdd( new IO("in3","Input 3",IO_REAL,"1") );	
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in4_"+TSYS::int2str(i_c),"Input 4."+TSYS::int2str(i_c),IO_REAL,"1") );
    for( int i_c = 1; i_c <= 5; i_c++ )
        ioAdd( new IO("in5_"+TSYS::int2str(i_c),"Input 5."+TSYS::int2str(i_c),IO_REAL,"1") );
    ioAdd( new IO("in6","Input 6",IO_REAL,"1") );
}

string Divider::name()
{
    return "Divider";
}

string Divider::descr()
{
    return "Divider per formula:\n"
	"out = (in1_1*in1_2*in1_3*in1_4*in1_5 + in2_1*in2_2*in2_3*in2_4*in2_5 + in3) /\n"
	"      (in4_1*in4_2*in4_3*in4_4*in4_5 + in5_1*in5_2*in5_3*in5_4*in5_5 + in6);";
}

void Divider::calc( TValFunc *v )
{
    double t1 =	v->getR(1)*v->getR(2)*v->getR(3)*v->getR(4)*v->getR(5) + 
		v->getR(6)*v->getR(7)*v->getR(8)*v->getR(9)*v->getR(10) + v->getR(11);
    double t2 = v->getR(12)*v->getR(13)*v->getR(14)*v->getR(15)*v->getR(16) +
	                v->getR(17)*v->getR(18)*v->getR(19)*v->getR(20)*v->getR(21) + v->getR(22);
    v->setR(0,t1/t2);
}

//------------------------------------------------------------------------------------
//PID
//Inputs:
//  0:val  - Value
//  1:sp   - Setpoint
//  2:max  - Maximum scale
//  3:min  - Minimum scale
//  4:out  - Output
//  5:auto - Automatic mode
//  6:casc - Cascade mode
//  7:in1  - Add input 1
//  8:in2  - Add input 2
//  9:in3  - Add input 3
//  10:in4  - Add input 4
//Koefficients:
//  11:Kp    - Gain
//  12:Ti   - Integral time (ms)
//  13:Td   - Differencial time (ms)
//  14:Tf   - Filter (lag) time (ms)
//  15:Hup  - Up output limit (%)
//  16:Hdwn - Down output limit (%)
//  17:Zi   - Insensibility zone (%)
//  18:K1   - Scale input 1
//  19:K2   - Scale input 2
//  20:K3   - Scale input 3
//  21:K4   - Scale input 4
//  22:cycle- Calc cycle (ms)
//Internal data:
//  23:#int - Curent integral value
//  24:#dif - Curent differencial value
//  25:#lag - Curent lag value 
//------------------------------------------------------------------------------------
PID::PID() : TFunction("pid")
{
    //Inputs
    ioAdd( new IO("val","Value",IO_REAL,"0") );
    ioAdd( new IO("sp","Setpoint",IO_REAL,"0") );
    ioAdd( new IO("max","Max scale",IO_REAL,"100") );
    ioAdd( new IO("min","Min scale",IO_REAL,"0") );
    ioAdd( new IO("out","Output (%)",IO_REAL|IO_RET,"0") );    
    ioAdd( new IO("auto","Auto mode",IO_BOOL,"0") );
    ioAdd( new IO("casc","Cascade mode",IO_BOOL,"0") );
    ioAdd( new IO("in1","Input 1",IO_REAL,"0") );
    ioAdd( new IO("in2","Input 2",IO_REAL,"0") );
    ioAdd( new IO("in3","Input 3",IO_REAL,"0") );
    ioAdd( new IO("in4","Input 4",IO_REAL,"0") );
    
    //Koefficients
    ioAdd( new IO("Kp","Kp",IO_REAL,"1") );
    ioAdd( new IO("Ti","Ti (ms)",IO_INT,"1000") );    
    ioAdd( new IO("Td","Td (ms)",IO_INT,"0") );    
    ioAdd( new IO("Tf","Tf-lag (ms)",IO_INT,"0") );
    ioAdd( new IO("Hup","Out up limit (%)",IO_REAL,"100") );
    ioAdd( new IO("Hdwn","Out down limit (%)",IO_REAL,"0") );
    ioAdd( new IO("Zi","Insensibility (%)",IO_REAL,"1") );    
    ioAdd( new IO("K1","K input 1",IO_REAL,"0") );
    ioAdd( new IO("K2","K input 2",IO_REAL,"0") );
    ioAdd( new IO("K3","K input 3",IO_REAL,"0") );
    ioAdd( new IO("K4","K input 4",IO_REAL,"0") );
    ioAdd( new IO("cycle","Calc cycle (ms)",IO_INT,"1000") );
    
    //Internal data:
    ioAdd( new IO("int","Integral value",IO_REAL|IO_OUT|IO_HIDE,"0") );
    ioAdd( new IO("dif","Differencial value",IO_REAL|IO_OUT|IO_HIDE,"0") );
    ioAdd( new IO("lag","Lag value",IO_REAL|IO_OUT|IO_HIDE,"0") );
}

string PID::name()
{
    return "PID regulator";
}

string PID::descr()
{
    return "PID regulator";
}

void PID::calc( TValFunc *v )
{
    double	val	= v->getR(0),
		sp	= v->getR(1),
		max 	= v->getR(2),
    		min	= v->getR(3),
    		out	= v->getR(4),
    		in1	= v->getR(7),
    		in2	= v->getR(8),
    		in3	= v->getR(9),
    		in4	= v->getR(10),
		kp	= v->getR(11),
		h_up	= v->getR(15),
		h_dwn	= v->getR(16),
		zi	= v->getR(17),
		k1	= v->getR(18),
		k2 	= v->getR(19),
		k3 	= v->getR(20),
		k4 	= v->getR(21),
	        cycle   = v->getI(22),
	        integ   = v->getR(23),
		difer	= v->getR(24),
		lag     = v->getR(25);
    
    double    	Kf	= (v->getI(14)>cycle)?cycle/v->getI(14):1.;
    double	Kint	= (v->getI(12)>cycle)?cycle/v->getI(12):1.;
    double	Kdif	= (v->getI(13)>cycle)?cycle/v->getI(13):1.;

    //Scale error
    if( max <= min )	return;
    
    //Prepare values
    sp = 100.*(sp+min)/(max-min);
    val = 100.*(val+min)/(max-min);
    val += k1*in1 + k2*in2;
    if(val >  100.) val = 100.;
    if(val < -100.) val = -100.;
    
    //Error
    double err = sp - val;
    
    //Insensibility
    if( fabs(err) < zi )	err = 0.;
    else
    {
	if( err>0. )	err-=zi;
	else		err+=zi;    
    }
    
    //Gain
    err*=kp;
    if(err >  100.) err = 100.;
    if(err < -100.) err = -100.;
    
    //Input filter lag    
    lag+=Kf*(err-lag);
    
    //Automatic mode enabled
    if(v->getB(5))
    {
	integ+=Kint*lag;		//Integral
	difer-=Kdif*(difer-lag);	//Differecial lag
	
	out = (2.*lag + integ - difer) + k3*in3 + k4*in4;
	if(out > h_up || out < h_dwn )
	{
	    if( out > h_up )	out = h_up;
	    if( out < h_dwn ) 	out = h_dwn;
	    //Fix integral
	    integ = out - 2.*lag + difer - k3*in3 + k4*in4;
	}
    }     
    
    //Write outputs
    v->setR(4,out);
    v->setR(23,integ);
    v->setR(24,difer);
    v->setR(25,lag);
}

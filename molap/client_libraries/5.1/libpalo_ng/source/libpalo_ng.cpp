/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * \author Florian Schaper <florian.schaper@jedox.com>
 * 
 *
 */

#ifdef BUILD_SANDBOX

#include <iostream>
#include <vector>

#include "libpalo_ng/Network/NetInitialisation.h"
#include "libpalo_ng/Palo/ServerPool.h"
#include "libpalo_ng/Palo/Database.h"
#include "libpalo_ng/Palo/Cube.h"

using namespace jedox::palo;

void printChildren(string prefix, const Dimension &dim, long parentid)
{
    std::list<ELEMENT_INFO> elems = dim.getElements(parentid);
    for (std::list<ELEMENT_INFO>::iterator it = elems.begin(); it != elems.end(); ++it) {
        std::cout << prefix << it->nelement << endl;
        if (it->number_children) {
            printChildren(prefix + "\t", dim, it->element);
        }
    }
}

void printChildrenLimit(string prefix, const Dimension &dim, long parentid, long limit)
{
    long pos = 0;
    for (;;) {
        std::list<ELEMENT_INFO> elems = dim.getElements(parentid, pos, limit);
        if (elems.empty()) {
            break;
        }
        for (std::list<ELEMENT_INFO>::iterator it = elems.begin(); it != elems.end(); ++it) {
            std::cout << prefix << it->nelement << endl;
            if (it->number_children) {
                printChildrenLimit(prefix + "\t", dim, it->element, limit);
            }
        }
        pos += elems.size();
    }
}




int main( int argc, char* argv[] ) {

	//  All Strings are UTF8.

	NetInitialisation& NetInit = NetInitialisation::instance();
	//	NetInit.initSSL("server.pem");

	std::string key;
	ServerPool& serverpool = ServerPool::getInstance();

	string serverurl("localhost");
	int port = 7777;
	string user("smith");
	string pass("smith");
	string delim("-----------");

	//// choose wether to use http or https is magically done in the server object itself
	//ServerSPtr myServer = serverpool.getServer("localhost", 1111, "admin", "admin", key );

	//std::vector<std::string> coordinates;
	//coordinates.push_back( "All Products" );
	//coordinates.push_back( "Europe" );
	//coordinates.push_back( "Year" );
	//coordinates.push_back( "2002" );
	//
	//coordinates.push_back( "Variance" );
	//coordinates.push_back( "Units" );
	//int cnt = (int)coordinates.size();
	//CELL_VALUE value = ( *myServer )["Demo"].cube["Sales"].CellValue( coordinates );
	//printf("Variance Units: %f\n", value.val.d);

	//coordinates[cnt-2]= "Budget";
	//coordinates[cnt-1]= "Gross Profit";
	//CELL_VALUE value2 = ( *myServer )["Demo"].cube["Sales"].CellValue( coordinates );
	//printf("Budget GrossProfit: %f\n", value2.val.d);

	//coordinates[cnt-2]= "Variance";
	//coordinates[cnt-1]= "Gross Profit";
	//CELL_VALUE value3 = ( *myServer )["Demo"].cube["Sales"].CellValue( coordinates );
	//printf("Variance GrossProfit: %f\n", value3.val.d);

	//AliasFilterSettings afs;
	//FieldFilterSettings ffs;
	//DataFilterSettings dfs;
	//SortingFilterSettings sfs;
	//StructuralFilterSettings stfs;
	//TextFilterSettings tfs;
	//BasicFilterSettings bfs;

	//tfs.active = true;
	//tfs.regexps.push_back("*X*");

	////test1
	///*std::vector<long> eids; eids.push_back(0); eids.push_back(7); eids.push_back(8);
	//(*myServer)["ttt"].dimension["a1"].bulkDeleteElements(eids);*/

	////test2
	///*std::vector<std::string> enames; enames.push_back("eeee"); enames.push_back("aaaa");
	//std::vector<std::vector<long> > c;
	//std::vector<std::vector<double> > w;
	//(*myServer)["ttt"].dimension["a1"].bulkCreateElements(enames, ELEMENT_INFO::NUMERIC, c, w);*/


	////test3
	///*std::vector<std::string> enames; enames.push_back("ee ee1"); enames.push_back("aaaa1");
	//std::vector<std::vector<long> > c;
	//std::vector<long> c1; c1.push_back(11l); c1.push_back(12l);
	//std::vector<long> c2; c2.push_back(9l); c2.push_back(10l);
	//c.push_back(c1); c.push_back(c2);

	//std::vector<std::vector<double> > w;
	//std::vector<double> w1; w1.push_back(7.5); w1.push_back(8);
	//std::vector<double> w2; w2.push_back(9.5); w2.push_back(-10);
	//w.push_back(w1); w.push_back(w2);
	//
	//(*myServer)["ttt"].dimension["a1"].bulkCreateElements(enames, ELEMENT_INFO::CONSOLIDATED, c, w);*/

	//ElementExList els = (*myServer)["Demo"].dimension["Products"].subset(myServer.get(), afs, ffs, bfs, dfs, sfs, stfs, tfs);

	//ElementExList::iterator it;
	//for (it = els.begin(); it!=els.end(); it++)
	//	std::cout<<it->get_name()<<std::endl;

	// choose wether to use http or https is magically done in the server object itself


	ServerSPtr myServer = serverpool.getServer(serverurl, port, user, pass, key );
	/*
	 AliasFilterSettings afs;
	 FieldFilterSettings ffs;
	 DataFilterSettings dfs;
	 SortingFilterSettings sfs;
	 StructuralFilterSettings stfs;
	 TextFilterSettings tfs;
	 BasicFilterSettings bfs;

	 ffs.active = true;
	 std::vector<string> f;
	 f.push_back("Color");
	 f.push_back("yell*");
	 ffs.advanced.push_back(f);

	 ElementExList els = (*myServer)["AFILTER"].dimension["Fruit"].subset(myServer.get(), afs, ffs, bfs, dfs, sfs, stfs, tfs);

	 ElementExList::iterator it;
	 for (it = els.begin(); it!=els.end(); it++)
	 std::cout<<it->get_name()<<std::endl;

	 */

	// SUPERVISION_SERVER_INFO value = ( *myServer ).getSVSInfo();

	cout << "Begin for " << user << endl << delim << endl;

	Dimension products = (*myServer)["Demo"].dimension["Products"];
    printChildren("",  products, -1);
	cout << delim << endl << "End for " << user << endl;

    myServer->logout();

	system("PAUSE");
}

#endif

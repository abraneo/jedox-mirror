Doxygen
Please use Doxygen to generate documentation.
Use the file Doxyfile in the Palo directory by
simply running doxygen in that directory. 

The following header files are documented.
Other headers are not needed by someone
just using libpalo_ng. (Directory: Palo)

Cube.h                             
Cubes.h                           
Database.h                         
Dimension.h                        
Server.h
Dimensions.h                       
Element.h                          
ServerPool.h

Additionally, the Exception-headers are documented.

Architecture

Libpalo_ng's architecture leaves the client unaware of 
concrete HTTP-connections or HTTP-clients. A minimal application
that connects to the PALO-Server and renames an Element follows:

ServerPool serverpool = ServerPool::getInstance();
Server* palo =  serverpool.getServer( host, port, user, password, key); //key is out-parameter, others are in
//get a Database-Object
Database db = (*palo)["NameOfDB"];
//get a specific Dimension from all Dimensions of the Database
Dimension dim = db.Dimensions["NameOfDim"];
//get a specific Element from the dimension and rename it
dim["NameOfElem"].rename("newName");
//Now the name of the Element has been changed on the Server

Accessing different levels

By using [] and "." ,
starting with a server object (Type: jedox::palo:Server),  
all kinds of Database-objects can be accessed and changed.
The follwing hierarchy exists:

Server

Database

Dimension/Cube

Element

We need several applications 
of [] and ".", depending on the level in the hierarchy.

//Deleting a Cube
server["NameOfDB"].Cubes["NameOfCube"].destroy();

//Creating a new Element
server["NameOfDB"].Dimensions["NameOfDimension"].createElement("NameOfElem",...);

//Renaming a Dimension 
server["NameOfDB"].Dimensions["NameOfDimension"].rename();

//Adding a Database (highest level "server")
server.createDatabase("NameOfDB");


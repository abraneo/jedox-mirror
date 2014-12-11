var server = "localhost";
var port = "7778";
var user = "admin";
//var password = "\t\1\t21232f297a57a5a743894a0e4a801fc3";
var password = "21232f297a57a5a743894a0e4a801fc3";
var sid;

function ajaxFunction() {
	try{
		// Opera 8.0+, Firefox, Safari
		ajaxRequest = new XMLHttpRequest();
	} catch (e){
		alert("Your browser broke!");
		return false;
	}
	// Create a function that will receive data sent from the server
	ajaxRequest.onreadystatechange = function(){
//		document.body.innerHTML += "response: "+ajaxRequest.responseText+"</br>";
//		document.body.innerHTML += "state: "+ajaxRequest.readyState+"</br>";
//		document.body.innerHTML += "header: "+JSON.stringify(ajaxRequest)+"</br>";
		if(ajaxRequest.readyState == 4){
			//alert(ajaxRequest.responseText);
			var result = ajaxRequest.responseText.split(";",2);
			sid = result[0];
			document.body.innerHTML += "SID: "+sid+"</br>";
		}
	};
	requrl = "http://"+server+":"+port+"/server/login?user="+user+"&password="+password;
	document.body.innerHTML += "login: "+requrl+"</br>";
	ajaxRequest.open("GET", requrl, true);
	ajaxRequest.send(null); 	
}

if (window.performance.now) {
    console.log("Using high performance timer");
    getTimestamp = function() { return window.performance.now(); };
} else {
    if (window.performance.webkitNow) {
        console.log("Using webkit high performance timer");
        getTimestamp = function() { return window.performance.webkitNow(); };
    } else {
        console.log("Using low performance timer");
        getTimestamp = function() { return new Date().getTime(); };
    }
}

function benchmark(fn, print)
{
	var start = new Date().getTime();

	for (i = 0; i < 1000000; ++i) {
		fn(i);
	}

	var end = new Date().getTime();
	var time = end - start;
	if (print) {
		alert('Execution time: ' + time + ' x='+x);
	} else {
	}
	return time; 
}

function Dimension(database, line)
{
	var items = line.split(";");
	this.id = parseInt(items[0]); 
	this.name = items[1].substring(1,items[1].length-1);
	this.elementsCount = parseInt(items[2]);
	this.maxLevel = parseInt(items[3]);
	this.maxIndent = parseInt(items[4]);
	this.maxDepth = parseInt(items[5]);
	this.type = parseInt(items[6]);
	this.attributesDimension = parseInt(items[7]);
	this.attributesCube = parseInt(items[8]);
	this.rightsCube = parseInt(items[9]);
	this.token = parseInt(items[10]);
}

function Cube(database, line)
{
	var items = line.split(";");
	this.id = parseInt(items[0]); 
	this.name = items[1].substring(1,items[1].length-1);
	this.dimensionsCount = parseInt(items[2]);
	var dimensionsStrings = items[3].split(",");
	this.dimensions = [];
	for (dim in dimensionsStrings) this.dimensions.push(parseInt(dimensionsStrings[dim]));
	this.cellsCount = parseFloat(items[4]);
	this.valuesCount = parseInt(items[5]);
	this.status = parseInt(items[6]);
	this.type = parseInt(items[7]);
	this.token = parseInt(items[8]);
	this.database = database;
}

Cube.prototype.request = function(url) {
	url += "&cube="+this.id;
	return this.database.request(url);
}

Cube.prototype.ruleParse = function(rule) {
	requrl = "/rule/parse?definition="+escape(rule);
	var result = this.request(requrl);
	return result;
}

function Database(server, line)
{
	var items = line.split(";");
	this.id = parseInt(items[0]); 
	this.name = items[1].substring(1,items[1].length-1);
	this.dimensionsCount = parseInt(items[2]);
	this.cubesCount = parseInt(items[3]);
	this.status = parseInt(items[4]);
	this.type = parseInt(items[5]);
	this.token = parseInt(items[6]);
	this.server = server;
}

Database.prototype.toString = function() {
	return "Database(id="+this.id+", name="+this.name+", type="+this.type+")";
};

Database.prototype.request = function(url) {
	url += "&database="+this.id;
	return this.server.request(url);
}

Database.prototype.getCubes = function() {
	var result = this.request("/database/cubes?database="+this.id); 
	var resultLines = result.split("\n");
	var cubes = new Object();
	for (lineNr in resultLines) {
		if (resultLines[lineNr].length) {
			var cube = new Cube(this, resultLines[lineNr]);
			cubes[cube.id] = cube;
		}
	}
	return cubes; 
};

function Server(user, password)
{
	this.user = user;
	this.password = password;
	requrl = "/server/login?user="+user+"&password="+password;
	var result = this.request(requrl).split(";",2);
	this.sid = result[0];
	document.body.innerHTML += "Connected as user:"+user+" sid: "+this.sid+"<br/>";
}

Server.prototype.toString = function() {
	return "Server(sid="+this.sid+")";
};

Server.prototype.request = function(url) {
	var AJAX = new XMLHttpRequest();
	if (this.sid != null) {
		url += "&sid="+this.sid;
	}
    AJAX.open("GET", url, false);                             
    console.log("request: "+url);
    AJAX.send(null);
    var result = AJAX.responseText;
    //alert("result: "+result);
    return result; 		
};

Server.prototype.getDatabases = function() {
	var result = this.request("/server/databases?sid="+this.sid); 
	var resultLines = result.split("\n");
	var databases = new Object();
	for (lineNr in resultLines) {
		if (resultLines[lineNr].length) {
			var database = new Database(this, resultLines[lineNr]);
			databases[database.id] = database;
		}
	}
	return databases; 
};

$(document).ready( function(){
    var elem=document.createElement("DIV");
    //elem.appendChild(document.createTextNode("server"));
    elem = document.body.appendChild(elem);
    //document.body 
    //document.write(server);
    //ajaxFunction();
    srv = new Server(user, password);
    //document.body.innerHTML += JSON.stringify(srv)+"<br/>";
    var databases = srv.getDatabases();
    //document.body.innerHTML += JSON.stringify(databases)+"<br/>";
    
	var selectedDatabase;
	var cubes;
	for (var dbNr in databases) {
	    $('#DatabasesSelect').append($("<option/>", {
	        value: dbNr,
	        text: databases[dbNr].name
	    }));
	}
	$("#DatabasesSelect").change(function(){
		selectedDatabase = databases[$(this).val()]; 
	    cubes = selectedDatabase.getCubes();
	    $('#CubeSelect').empty();
		for (var cube in cubes) {
			$('#CubeSelect').append($("<option/>", {
		        value: cube,
		        text: cubes[cube].name
		    }));
		}
	});
	var selectedCube;
	$("#CubeSelect").change(function(){
		selectedCube = cubes[$(this).val()];
	});
	$("#Parse").click(function(){
		if (selectedCube != null) {
			rulexml = selectedCube.ruleParse($("#PlainRule").val());
			$('<div>').appendTo($(document.body)).text(rulexml);
		}
	});
});


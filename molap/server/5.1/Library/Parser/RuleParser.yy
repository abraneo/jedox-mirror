%skeleton "lalr1.cc"                          /*  -*- C++ -*- */
%defines
%define "parser_class_name" "RuleParser"
%code requires {
	class RuleParserDriver;
}
     
%{
# include <string>
# include <vector>
# include <utility>
class RuleParserDriver;

#include "Parser/DestinationNode.h"
#include "Parser/ExprNode.h"
#include "Parser/Node.h"
#include "Parser/SourceNode.h"
#include "Parser/VariableNode.h"
using namespace palo;
using namespace std;

string errorString;
%}

// The parsing context.
%parse-param { RuleParserDriver& driver }
%lex-param   { RuleParserDriver& driver }

%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.ruleString;
};
%debug
%error-verbose
// Symbols.
%union
{
  int                               ival;
  double                            dval;
  std::string                       *sval;
  DestinationNode                   *dnval;
  ExprNode                          *enval;
  Node                              *nval;
  RPVpsval							*vpsval;
  RPVpival							*vpival;
  RPPsval            				*psval;
  RPPival		                    *pival;
  vector< Node* >                   *vpnval;
};

%{
# include "RuleParserDriver.h"
%}

%token                 END      0       "end of file"
%token                 ASSIGN           "="
%token                 CONS_FLAG        
%token                 BASE_FLAG
%token                 GE 
%token                 LE 
%token                 EQ 
%token                 NE 
%token                 GT 
%token                 LT
%token                 MARKER_OPEN
%token                 MARKER_CLOSE
%token                 SMARKER_OPEN
%token                 SMARKER_CLOSE

%token   <sval>        STRING           "string"
%token   <sval>        EL_STRING        "element_string"
%token   <sval>        VAR_STRING       "variable_string"
%token   <sval>        FUNCTION         "function"
%token   <ival>        INTEGER          "integer"
%token   <dval>        DOUBLE           "double"

%type    <enval>       exp
%type    <dnval>       destination
%type    <enval>       source
%type    <enval>       marker
%type    <nval>        rule

%type    <psval>       element          
%type    <psval>       simpleElement          
%type    <pival>       elementId          
%type    <pival>       simpleElementId          
%type    <vpsval>      elements         
%type    <vpival>      elementsIds         
%type    <vpsval>      markerElements         
%type    <vpival>      markerElementsIds         
%type    <vpnval>      parameters      
%type    <vpnval>      markers

%printer    { debug_stream() << *$$; } "string"

%destructor { /* delete string */
              if ($$) {
                delete $$;
              }
            }                           STRING EL_STRING FUNCTION

%destructor { /* delete node */
              if ($$) {
                delete $$;
              }
            }                           exp destination source marker rule

%destructor { /* delete pair */
              if ($$) {
                if ($$->first) {
                  delete $$->first;
                }
                if ($$->second.first) {
                  delete $$->second.first;
                }
                delete $$;
              }
            }                           element simpleElement

%destructor { /* delete pair */
              if ($$) {
                delete $$;
              }
            }                           elementId simpleElementId

%destructor { /* delete vector of pairs */
              if ($$) {
                for (RPVpsval::iterator i = $$->begin(); i != $$->end(); i++) {
                  if ( *i ) {
                    if ((*i)->first) {
                      delete (*i)->first;
                    }
                    if ((*i)->second.first) {
                      delete (*i)->second.first;
                    }
                    delete (*i);
                  }
                }
                delete $$;
              }
            }                           elements

%destructor { /* delete vector of pairs */
              if ($$) {
                for (RPVpival::iterator i = $$->begin(); i != $$->end(); i++) {
                  if ( *i ) {
                    delete (*i);
                  }
                }
                delete $$;
              }
            }                           elementsIds

%printer    { debug_stream() << $$; }  "integer" exp


%%
%start rule;


rule:
  destination ASSIGN exp END {
                                      driver.setResult(new RuleNode(RuleNode::NONE, $1, $3));
                                      $$ = 0;
                                    }
  | destination ASSIGN CONS_FLAG exp END {
                                      driver.setResult(new RuleNode(RuleNode::CONSOLIDATION, $1, $4));
                                      $$ = 0;
                                    }
  | destination ASSIGN BASE_FLAG exp END {
                                      driver.setResult(new RuleNode(RuleNode::BASE, $1, $4));
                                      $$ = 0;
                                    }
  | destination ASSIGN BASE_FLAG exp '@' markers END {
                                      driver.setResult(new RuleNode(RuleNode::BASE, $1, $4, $6));
                                      $$ = 0;
                                    }
  | destination ASSIGN exp '@' markers END {
                                      driver.setResult(new RuleNode(RuleNode::NONE, $1, $3, $5));
                                      $$ = 0;
                                    }
  ;

%left GE LE EQ NE GT LT;
%left '+' '-';
%left '*' '/';
exp:
  source                            { $$ = $1; }
  | '-' source                      { 
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back($2);
                                      $$ = driver.createFunction("*", v);
  									}
  | marker                          { $$ = $1; }
  | '-' marker                      { 
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back($2);
                                      $$ = driver.createFunction("*", v);
  									}
  | INTEGER                         { $$ = new DoubleNode($1 * 1.0); }
  | DOUBLE                          { $$ = new DoubleNode($1); }
  | '+' INTEGER                     { $$ = new DoubleNode($2 * 1.0); }
  | '+' DOUBLE                      { $$ = new DoubleNode($2); }
  | '-' INTEGER                     { $$ = new DoubleNode($2 * -1.0); }
  | '-' DOUBLE                      { $$ = new DoubleNode($2 * -1.0); }
  | STRING                          { $$ = new StringNode($1); }
  | VAR_STRING                      { $$ = new VariableNode($1); }
  | exp '+' exp                     {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("+", v);
                                    }
  | exp '-' exp                     {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("-", v);
                                    }
  | exp '*' exp                     {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("*", v);
                                    }
  | exp '/' exp                     {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("/", v);
                                    }
  | exp GE exp                      {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction(">=", v);
                                    }
  | exp LE exp                      {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("<=", v);
                                    }
  | exp EQ exp                      {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("==", v);
                                    }
  | exp NE exp                      {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("!=", v);
                                    }
  | exp GT exp                      {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction(">", v);
                                    }
  | exp LT exp                      {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($1);
                                      v->push_back($3);
                                      $$ = driver.createFunction("<", v);
                                    }
  | '-' '(' exp ')'                 {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back($3);
                                      $$ = driver.createFunction("*", v);
                                    }
  | '(' exp ')'                     {
                                      $$ = $2;
                                    }
  | '-' FUNCTION parameters ')'     {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back(driver.createFunction(*($2), $3, &errorString));
                                      delete $2;
                                      if (!(v->at(1))) {
                                        error(@2, errorString);
                                       	YYERROR;
                                      }
                                      $$ = driver.createFunction("*", v);
                                    }
  | FUNCTION parameters ')'         {
                                      $$ = driver.createFunction(*($1), $2, &errorString);
                                      delete $1;
                                      if (!($$)) {
                                        error(@1, errorString);
                                       	YYERROR;
                                      }
                                    }
  ;

parameters:
  '(' exp                           {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back($2);
                                      $$ = v; 
                                    }
  | '('                             {
                                      vector<Node*> *v = new vector<Node*>;
                                      $$ = v; 
                                    }
  | parameters ',' exp              {
                                      $1->push_back($3);
                                      $$ = $1; 
                                    }
  ;

destination:
  elements ']'                      {
                                      $$ = new DestinationNode($1, 0);
                                    }
  | elementsIds '}'                 {
                                      $$ = new DestinationNode(0, $1);
                                    }
  ;

source:
  elements ']'                      {
                                      $$ = new SourceNode($1, 0);
                                    }
  | elementsIds '}'                 {
                                      $$ = new SourceNode(0, $1);
                                    }
  ;


markers:
  marker                                {
                                          vector<Node*> *v = new vector<Node*>;
                                          $$ = v;
                                          $$->push_back($1);
                                        }
  | FUNCTION parameters ')'             {
                                          Node* func = driver.createFunction(*($1), $2, &errorString);
                                          delete $1;

	                                      if (!func) {
	                                        error(@1, errorString);
	                                       	YYERROR;
	                                      }

                                          vector<Node*> *v = new vector<Node*>;
                                          $$ = v;
                                          $$->push_back(func);
                                        }
  | markers ',' marker                  {
                                          $$ = $1;
                                          $$->push_back($3);
                                        }
  | markers ',' FUNCTION parameters ')' {
                                          Node* func = driver.createFunction(*($3), $4, &errorString);
                                          delete $3;

	                                      if (!func) {
	                                        error(@3, errorString);
	                                       	YYERROR;
	                                      }

                                          $$ = $1;
                                          $$->push_back(func);
                                        }
  ;

marker:
  markerElements MARKER_CLOSE       {
                                      $$ = driver.createMarker($1, 0, &errorString);
                                      if (!($$)) {
                                        error(@1, errorString);
                                       	YYERROR;
                                      }
                                    }
  | markerElementsIds SMARKER_CLOSE {
                                      $$ = driver.createMarker(0, $1, &errorString);
                                      if (!($$)) {
                                        error(@1, errorString);
                                       	YYERROR;
                                      }
                                    }
  ;


elements:
  '[' element                       {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back($2);
                                      $$ = v;
                                    }
  | '['                             {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back((RPPsval*)0);
                                      $$ = v;
                                    }
  | elements ',' element            {
                                      $1->push_back($3);
                                      $$ = $1;
                                    }
  | elements ','                    {
                                      $1->push_back((RPPsval*)0);
                                      $$ = $1;
                                    }
  ;

markerElements:
  MARKER_OPEN simpleElement         {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back($2);
                                      $$ = v;
                                    }
  | MARKER_OPEN                     {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back((RPPsval*)0);
                                      $$ = v;
                                    }
  | markerElements ',' simpleElement {
                                      $1->push_back($3);
                                      $$ = $1;
                                    }
  | markerElements ','              {
                                      $1->push_back((RPPsval*)0);
                                      $$ = $1;
                                    }
  ;

simpleElement:
  EL_STRING                         {
                                      RPPsval *p = new RPPsval;
                                      p->first = 0;
                                      p->second.first= $1;
                                      p->second.second= 0;
                                      //cout << "elem pos: " << @1.begin.line << ":" << @1.begin.column << endl;
                                      $$ = p;
                                    }
  | EL_STRING ':' EL_STRING         {
                                      RPPsval *p = new RPPsval;
                                      p->first = $1;
                                      p->second.first= $3;
                                      p->second.second= 0;
                                      //cout << "dim pos: " << @1.begin.line << ":" << @1.begin.column << endl;
                                      $$ = p;
                                    }
  ;

element:
  EL_STRING                         {
                                      RPPsval *p = new RPPsval;
                                      p->first = 0;
                                      p->second.first= $1;
                                      p->second.second= 0;
                                      //cout << "elem pos: " << @1.begin.line << ":" << @1.begin.column << endl;
                                      $$ = p;
                                    }
  | EL_STRING ':' EL_STRING         {
                                      RPPsval *p = new RPPsval;
                                      p->first = $1;
                                      p->second.first= $3;
                                      p->second.second= 0;
                                      //cout << "dim pos: " << @1.begin.line << ":" << @1.begin.column << endl;
                                      $$ = p;
                                    }
  | EL_STRING ':' FUNCTION '(' INTEGER ')' {
                                      $$ = RPPsvalFromParam($1, $3, $5, errorString);
                                      if (!($$)) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
  | EL_STRING ':' FUNCTION '(' '+' INTEGER ')' {
                                      $$ = RPPsvalFromParam($1, $3, $6, errorString);
                                      if (!($$)) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
  | EL_STRING ':' FUNCTION '(' '-' INTEGER ')' {
                                      $$ = RPPsvalFromParam($1, $3, -($6), errorString);
                                      if (!($$)) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
  ;

elementsIds:
  '{' elementId                     {
                                      RPVpival *v = new RPVpival;
                                      v->push_back($2);
                                      $$ = v;
                                    }
  | '{'                             {
                                      RPVpival *v = new RPVpival;
                                      v->push_back((RPPival*)0);
                                      $$ = v;
                                    }
  | elementsIds ',' elementId       {
                                      $1->push_back($3);
                                      $$ = $1;
                                    }
  | elementsIds ','                 {
                                      $1->push_back((RPPival*)0);
                                      $$ = $1;
                                    }
  ;

markerElementsIds:
  SMARKER_OPEN simpleElementId      {
                                      RPVpival *v = new RPVpival;
                                      v->push_back($2);
                                      $$ = v;
                                    }
  | SMARKER_OPEN                    {
                                      RPVpival *v = new RPVpival;
                                      v->push_back((RPPival*)0);
                                      $$ = v;
                                    }
  | markerElementsIds ',' simpleElementId {
                                      $1->push_back($3);
                                      $$ = $1;
                                    }
  | markerElementsIds ','           {
                                      $1->push_back((RPPival*)0);
                                      $$ = $1;
                                    }
  ;

simpleElementId:
  INTEGER ':' INTEGER               {
                                      RPPival *p = new RPPival;
                                      p->first = $1;
                                      p->second.first = $3;
                                      p->second.second = 0;
                                      $$ = p;
                                    }
  | INTEGER '@' INTEGER             {
                                      RPPival *p = new RPPival;
                                      p->first = -(($1) + 1);
                                      p->second.first = $3;
                                      p->second.second = 0;
                                      $$ = p;
                                    }
  ;
  
elementId:
  INTEGER ':' INTEGER               {
                                      RPPival *p = new RPPival;
                                      p->first = $1;
                                      p->second.first = $3;
                                      p->second.second = 0;
                                      $$ = p;
                                    }
  | INTEGER '@' INTEGER             {
                                      RPPival *p = new RPPival;
                                      p->first = -(($1) + 1);
                                      p->second.first = $3;
                                      p->second.second = 0;
                                      $$ = p;
                                    }
  | INTEGER ':' FUNCTION '(' INTEGER ')' {
                                      $$ = RPPivalFromParam($1, $3, $5, errorString);
                                      if (!($$)) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
  | INTEGER ':' FUNCTION '(' '+' INTEGER ')' {
                                      $$ = RPPivalFromParam($1, $3, $6, errorString);
                                      if (!($$)) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
  | INTEGER ':' FUNCTION '(' '-' INTEGER ')' {
                                      $$ = RPPivalFromParam($1, $3, -($6), errorString);
                                      if (!($$)) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
  ;


%%

void  yy::RuleParser::error (const yy::RuleParser::location_type& l, const string& m) {
  driver.error(l, m);
}

/* A Bison parser, made by GNU Bison 2.5.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* First part of user declarations.  */


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




#include "RuleParser.hpp"

/* User implementation prologue.  */


# include "RuleParserDriver.h"



#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                               \
 do                                                                    \
   if (N)                                                              \
     {                                                                 \
       (Current).begin = YYRHSLOC (Rhs, 1).begin;                      \
       (Current).end   = YYRHSLOC (Rhs, N).end;                        \
     }                                                                 \
   else                                                                \
     {                                                                 \
       (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;        \
     }                                                                 \
 while (false)
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  RuleParser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  RuleParser::RuleParser (RuleParserDriver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg)
  {
  }

  RuleParser::~RuleParser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  RuleParser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    switch (yytype)
      {
        case 16: /* "\"string\"" */

	{ debug_stream() << *(yyvaluep->sval); };

	break;
      case 20: /* "\"integer\"" */

	{ debug_stream() << (yyvaluep->ival); };

	break;
      case 37: /* "exp" */

	{ debug_stream() << (yyvaluep->enval); };

	break;
       default:
	  break;
      }
  }


  void
  RuleParser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  RuleParser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
        case 16: /* "\"string\"" */

	{ /* delete string */
              if ((yyvaluep->sval)) {
                delete (yyvaluep->sval);
              }
            };

	break;
      case 17: /* "\"element_string\"" */

	{ /* delete string */
              if ((yyvaluep->sval)) {
                delete (yyvaluep->sval);
              }
            };

	break;
      case 19: /* "\"function\"" */

	{ /* delete string */
              if ((yyvaluep->sval)) {
                delete (yyvaluep->sval);
              }
            };

	break;
      case 36: /* "rule" */

	{ /* delete node */
              if ((yyvaluep->nval)) {
                delete (yyvaluep->nval);
              }
            };

	break;
      case 37: /* "exp" */

	{ /* delete node */
              if ((yyvaluep->enval)) {
                delete (yyvaluep->enval);
              }
            };

	break;
      case 39: /* "destination" */

	{ /* delete node */
              if ((yyvaluep->dnval)) {
                delete (yyvaluep->dnval);
              }
            };

	break;
      case 40: /* "source" */

	{ /* delete node */
              if ((yyvaluep->enval)) {
                delete (yyvaluep->enval);
              }
            };

	break;
      case 42: /* "marker" */

	{ /* delete node */
              if ((yyvaluep->enval)) {
                delete (yyvaluep->enval);
              }
            };

	break;
      case 43: /* "elements" */

	{ /* delete vector of pairs */
              if ((yyvaluep->vpsval)) {
                for (RPVpsval::iterator i = (yyvaluep->vpsval)->begin(); i != (yyvaluep->vpsval)->end(); i++) {
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
                delete (yyvaluep->vpsval);
              }
            };

	break;
      case 45: /* "simpleElement" */

	{ /* delete pair */
              if ((yyvaluep->psval)) {
                if ((yyvaluep->psval)->first) {
                  delete (yyvaluep->psval)->first;
                }
                if ((yyvaluep->psval)->second.first) {
                  delete (yyvaluep->psval)->second.first;
                }
                delete (yyvaluep->psval);
              }
            };

	break;
      case 46: /* "element" */

	{ /* delete pair */
              if ((yyvaluep->psval)) {
                if ((yyvaluep->psval)->first) {
                  delete (yyvaluep->psval)->first;
                }
                if ((yyvaluep->psval)->second.first) {
                  delete (yyvaluep->psval)->second.first;
                }
                delete (yyvaluep->psval);
              }
            };

	break;
      case 47: /* "elementsIds" */

	{ /* delete vector of pairs */
              if ((yyvaluep->vpival)) {
                for (RPVpival::iterator i = (yyvaluep->vpival)->begin(); i != (yyvaluep->vpival)->end(); i++) {
                  if ( *i ) {
                    delete (*i);
                  }
                }
                delete (yyvaluep->vpival);
              }
            };

	break;
      case 49: /* "simpleElementId" */

	{ /* delete pair */
              if ((yyvaluep->pival)) {
                delete (yyvaluep->pival);
              }
            };

	break;
      case 50: /* "elementId" */

	{ /* delete pair */
              if ((yyvaluep->pival)) {
                delete (yyvaluep->pival);
              }
            };

	break;

	default:
	  break;
      }
  }

  void
  RuleParser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  RuleParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  RuleParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  RuleParser::debug_level_type
  RuleParser::debug_level () const
  {
    return yydebug_;
  }

  void
  RuleParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  inline bool
  RuleParser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  RuleParser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  RuleParser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    semantic_type yylval;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* User initialization code.  */
    
{
  // Initialize the initial location.
  yylloc.begin.filename = yylloc.end.filename = &driver.ruleString;
}


    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
	YYCDEBUG << "Reading a token: ";
	yychar = yylex (&yylval, &yylloc, driver);
      }


    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
	  case 2:

    {
                                      driver.setResult(new RuleNode(RuleNode::NONE, (yysemantic_stack_[(4) - (1)].dnval), (yysemantic_stack_[(4) - (3)].enval)));
                                      (yyval.nval) = 0;
                                    }
    break;

  case 3:

    {
                                      driver.setResult(new RuleNode(RuleNode::CONSOLIDATION, (yysemantic_stack_[(5) - (1)].dnval), (yysemantic_stack_[(5) - (4)].enval)));
                                      (yyval.nval) = 0;
                                    }
    break;

  case 4:

    {
                                      driver.setResult(new RuleNode(RuleNode::BASE, (yysemantic_stack_[(5) - (1)].dnval), (yysemantic_stack_[(5) - (4)].enval)));
                                      (yyval.nval) = 0;
                                    }
    break;

  case 5:

    {
                                      driver.setResult(new RuleNode(RuleNode::BASE, (yysemantic_stack_[(7) - (1)].dnval), (yysemantic_stack_[(7) - (4)].enval), (yysemantic_stack_[(7) - (6)].vpnval)));
                                      (yyval.nval) = 0;
                                    }
    break;

  case 6:

    {
                                      driver.setResult(new RuleNode(RuleNode::NONE, (yysemantic_stack_[(6) - (1)].dnval), (yysemantic_stack_[(6) - (3)].enval), (yysemantic_stack_[(6) - (5)].vpnval)));
                                      (yyval.nval) = 0;
                                    }
    break;

  case 7:

    { (yyval.enval) = (yysemantic_stack_[(1) - (1)].enval); }
    break;

  case 8:

    { 
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back((yysemantic_stack_[(2) - (2)].enval));
                                      (yyval.enval) = driver.createFunction("*", v);
  									}
    break;

  case 9:

    { (yyval.enval) = (yysemantic_stack_[(1) - (1)].enval); }
    break;

  case 10:

    { 
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back((yysemantic_stack_[(2) - (2)].enval));
                                      (yyval.enval) = driver.createFunction("*", v);
  									}
    break;

  case 11:

    { (yyval.enval) = new DoubleNode((yysemantic_stack_[(1) - (1)].ival) * 1.0); }
    break;

  case 12:

    { (yyval.enval) = new DoubleNode((yysemantic_stack_[(1) - (1)].dval)); }
    break;

  case 13:

    { (yyval.enval) = new DoubleNode((yysemantic_stack_[(2) - (2)].ival) * 1.0); }
    break;

  case 14:

    { (yyval.enval) = new DoubleNode((yysemantic_stack_[(2) - (2)].dval)); }
    break;

  case 15:

    { (yyval.enval) = new DoubleNode((yysemantic_stack_[(2) - (2)].ival) * -1.0); }
    break;

  case 16:

    { (yyval.enval) = new DoubleNode((yysemantic_stack_[(2) - (2)].dval) * -1.0); }
    break;

  case 17:

    { (yyval.enval) = new StringNode((yysemantic_stack_[(1) - (1)].sval)); }
    break;

  case 18:

    { (yyval.enval) = new VariableNode((yysemantic_stack_[(1) - (1)].sval)); }
    break;

  case 19:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("+", v);
                                    }
    break;

  case 20:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("-", v);
                                    }
    break;

  case 21:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("*", v);
                                    }
    break;

  case 22:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("/", v);
                                    }
    break;

  case 23:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction(">=", v);
                                    }
    break;

  case 24:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("<=", v);
                                    }
    break;

  case 25:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("==", v);
                                    }
    break;

  case 26:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("!=", v);
                                    }
    break;

  case 27:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction(">", v);
                                    }
    break;

  case 28:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(3) - (1)].enval));
                                      v->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("<", v);
                                    }
    break;

  case 29:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back((yysemantic_stack_[(4) - (3)].enval));
                                      (yyval.enval) = driver.createFunction("*", v);
                                    }
    break;

  case 30:

    {
                                      (yyval.enval) = (yysemantic_stack_[(3) - (2)].enval);
                                    }
    break;

  case 31:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back(new DoubleNode(-1.0));
                                      v->push_back(driver.createFunction(*((yysemantic_stack_[(4) - (2)].sval)), (yysemantic_stack_[(4) - (3)].vpnval), &errorString));
                                      delete (yysemantic_stack_[(4) - (2)].sval);
                                      if (!(v->at(1))) {
                                        error((yylocation_stack_[(4) - (2)]), errorString);
                                       	YYERROR;
                                      }
                                      (yyval.enval) = driver.createFunction("*", v);
                                    }
    break;

  case 32:

    {
                                      (yyval.enval) = driver.createFunction(*((yysemantic_stack_[(3) - (1)].sval)), (yysemantic_stack_[(3) - (2)].vpnval), &errorString);
                                      delete (yysemantic_stack_[(3) - (1)].sval);
                                      if (!((yyval.enval))) {
                                        error((yylocation_stack_[(3) - (1)]), errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 33:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      v->push_back((yysemantic_stack_[(2) - (2)].enval));
                                      (yyval.vpnval) = v; 
                                    }
    break;

  case 34:

    {
                                      vector<Node*> *v = new vector<Node*>;
                                      (yyval.vpnval) = v; 
                                    }
    break;

  case 35:

    {
                                      (yysemantic_stack_[(3) - (1)].vpnval)->push_back((yysemantic_stack_[(3) - (3)].enval));
                                      (yyval.vpnval) = (yysemantic_stack_[(3) - (1)].vpnval); 
                                    }
    break;

  case 36:

    {
                                      (yyval.dnval) = new DestinationNode((yysemantic_stack_[(2) - (1)].vpsval), 0);
                                    }
    break;

  case 37:

    {
                                      (yyval.dnval) = new DestinationNode(0, (yysemantic_stack_[(2) - (1)].vpival));
                                    }
    break;

  case 38:

    {
                                      (yyval.enval) = new SourceNode((yysemantic_stack_[(2) - (1)].vpsval), 0);
                                    }
    break;

  case 39:

    {
                                      (yyval.enval) = new SourceNode(0, (yysemantic_stack_[(2) - (1)].vpival));
                                    }
    break;

  case 40:

    {
                                          vector<Node*> *v = new vector<Node*>;
                                          (yyval.vpnval) = v;
                                          (yyval.vpnval)->push_back((yysemantic_stack_[(1) - (1)].enval));
                                        }
    break;

  case 41:

    {
                                          Node* func = driver.createFunction(*((yysemantic_stack_[(3) - (1)].sval)), (yysemantic_stack_[(3) - (2)].vpnval), &errorString);
                                          delete (yysemantic_stack_[(3) - (1)].sval);

	                                      if (!func) {
	                                        error((yylocation_stack_[(3) - (1)]), errorString);
	                                       	YYERROR;
	                                      }

                                          vector<Node*> *v = new vector<Node*>;
                                          (yyval.vpnval) = v;
                                          (yyval.vpnval)->push_back(func);
                                        }
    break;

  case 42:

    {
                                          (yyval.vpnval) = (yysemantic_stack_[(3) - (1)].vpnval);
                                          (yyval.vpnval)->push_back((yysemantic_stack_[(3) - (3)].enval));
                                        }
    break;

  case 43:

    {
                                          Node* func = driver.createFunction(*((yysemantic_stack_[(5) - (3)].sval)), (yysemantic_stack_[(5) - (4)].vpnval), &errorString);
                                          delete (yysemantic_stack_[(5) - (3)].sval);

	                                      if (!func) {
	                                        error((yylocation_stack_[(5) - (3)]), errorString);
	                                       	YYERROR;
	                                      }

                                          (yyval.vpnval) = (yysemantic_stack_[(5) - (1)].vpnval);
                                          (yyval.vpnval)->push_back(func);
                                        }
    break;

  case 44:

    {
                                      (yyval.enval) = driver.createMarker((yysemantic_stack_[(2) - (1)].vpsval), 0, &errorString);
                                      if (!((yyval.enval))) {
                                        error((yylocation_stack_[(2) - (1)]), errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 45:

    {
                                      (yyval.enval) = driver.createMarker(0, (yysemantic_stack_[(2) - (1)].vpival), &errorString);
                                      if (!((yyval.enval))) {
                                        error((yylocation_stack_[(2) - (1)]), errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 46:

    {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back((yysemantic_stack_[(2) - (2)].psval));
                                      (yyval.vpsval) = v;
                                    }
    break;

  case 47:

    {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back((RPPsval*)0);
                                      (yyval.vpsval) = v;
                                    }
    break;

  case 48:

    {
                                      (yysemantic_stack_[(3) - (1)].vpsval)->push_back((yysemantic_stack_[(3) - (3)].psval));
                                      (yyval.vpsval) = (yysemantic_stack_[(3) - (1)].vpsval);
                                    }
    break;

  case 49:

    {
                                      (yysemantic_stack_[(2) - (1)].vpsval)->push_back((RPPsval*)0);
                                      (yyval.vpsval) = (yysemantic_stack_[(2) - (1)].vpsval);
                                    }
    break;

  case 50:

    {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back((yysemantic_stack_[(2) - (2)].psval));
                                      (yyval.vpsval) = v;
                                    }
    break;

  case 51:

    {
                                      RPVpsval *v = new RPVpsval;
                                      v->push_back((RPPsval*)0);
                                      (yyval.vpsval) = v;
                                    }
    break;

  case 52:

    {
                                      (yysemantic_stack_[(3) - (1)].vpsval)->push_back((yysemantic_stack_[(3) - (3)].psval));
                                      (yyval.vpsval) = (yysemantic_stack_[(3) - (1)].vpsval);
                                    }
    break;

  case 53:

    {
                                      (yysemantic_stack_[(2) - (1)].vpsval)->push_back((RPPsval*)0);
                                      (yyval.vpsval) = (yysemantic_stack_[(2) - (1)].vpsval);
                                    }
    break;

  case 54:

    {
                                      RPPsval *p = new RPPsval;
                                      p->first = 0;
                                      p->second.first= (yysemantic_stack_[(1) - (1)].sval);
                                      p->second.second= 0;
//                                      cout << "elem pos: " << (yylocation_stack_[(1) - (1)]).begin.line << ":" << (yylocation_stack_[(1) - (1)]).begin.column << endl;
                                      (yyval.psval) = p;
                                    }
    break;

  case 55:

    {
                                      RPPsval *p = new RPPsval;
                                      p->first = (yysemantic_stack_[(3) - (1)].sval);
                                      p->second.first= (yysemantic_stack_[(3) - (3)].sval);
                                      p->second.second= 0;
//                                      cout << "dim pos: " << (yylocation_stack_[(3) - (1)]).begin.line << ":" << (yylocation_stack_[(3) - (1)]).begin.column << endl;
                                      (yyval.psval) = p;
                                    }
    break;

  case 56:

    {
                                      RPPsval *p = new RPPsval;
                                      p->first = 0;
                                      p->second.first= (yysemantic_stack_[(1) - (1)].sval);
                                      p->second.second= 0;
//                                      cout << "elem pos: " << (yylocation_stack_[(1) - (1)]).begin.line << ":" << (yylocation_stack_[(1) - (1)]).begin.column << endl;
                                      (yyval.psval) = p;
                                    }
    break;

  case 57:

    {
                                      RPPsval *p = new RPPsval;
                                      p->first = (yysemantic_stack_[(3) - (1)].sval);
                                      p->second.first= (yysemantic_stack_[(3) - (3)].sval);
                                      p->second.second= 0;
//                                      cout << "dim pos: " << (yylocation_stack_[(3) - (1)]).begin.line << ":" << (yylocation_stack_[(3) - (1)]).begin.column << endl;
                                      (yyval.psval) = p;
                                    }
    break;

  case 58:

    {
                                      (yyval.psval) = RPPsvalFromParam((yysemantic_stack_[(6) - (1)].sval), (yysemantic_stack_[(6) - (3)].sval), (yysemantic_stack_[(6) - (5)].ival), errorString);
                                      if (!((yyval.psval))) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 59:

    {
                                      (yyval.psval) = RPPsvalFromParam((yysemantic_stack_[(7) - (1)].sval), (yysemantic_stack_[(7) - (3)].sval), (yysemantic_stack_[(7) - (6)].ival), errorString);
                                      if (!((yyval.psval))) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 60:

    {
                                      (yyval.psval) = RPPsvalFromParam((yysemantic_stack_[(7) - (1)].sval), (yysemantic_stack_[(7) - (3)].sval), -((yysemantic_stack_[(7) - (6)].ival)), errorString);
                                      if (!((yyval.psval))) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 61:

    {
                                      RPVpival *v = new RPVpival;
                                      v->push_back((yysemantic_stack_[(2) - (2)].pival));
                                      (yyval.vpival) = v;
                                    }
    break;

  case 62:

    {
                                      RPVpival *v = new RPVpival;
                                      v->push_back((RPPival*)0);
                                      (yyval.vpival) = v;
                                    }
    break;

  case 63:

    {
                                      (yysemantic_stack_[(3) - (1)].vpival)->push_back((yysemantic_stack_[(3) - (3)].pival));
                                      (yyval.vpival) = (yysemantic_stack_[(3) - (1)].vpival);
                                    }
    break;

  case 64:

    {
                                      (yysemantic_stack_[(2) - (1)].vpival)->push_back((RPPival*)0);
                                      (yyval.vpival) = (yysemantic_stack_[(2) - (1)].vpival);
                                    }
    break;

  case 65:

    {
                                      RPVpival *v = new RPVpival;
                                      v->push_back((yysemantic_stack_[(2) - (2)].pival));
                                      (yyval.vpival) = v;
                                    }
    break;

  case 66:

    {
                                      RPVpival *v = new RPVpival;
                                      v->push_back((RPPival*)0);
                                      (yyval.vpival) = v;
                                    }
    break;

  case 67:

    {
                                      (yysemantic_stack_[(3) - (1)].vpival)->push_back((yysemantic_stack_[(3) - (3)].pival));
                                      (yyval.vpival) = (yysemantic_stack_[(3) - (1)].vpival);
                                    }
    break;

  case 68:

    {
                                      (yysemantic_stack_[(2) - (1)].vpival)->push_back((RPPival*)0);
                                      (yyval.vpival) = (yysemantic_stack_[(2) - (1)].vpival);
                                    }
    break;

  case 69:

    {
                                      RPPival *p = new RPPival;
                                      p->first = (yysemantic_stack_[(3) - (1)].ival);
                                      p->second.first = (yysemantic_stack_[(3) - (3)].ival);
                                      p->second.second = 0;
                                      (yyval.pival) = p;
                                    }
    break;

  case 70:

    {
                                      RPPival *p = new RPPival;
                                      p->first = -(((yysemantic_stack_[(3) - (1)].ival)) + 1);
                                      p->second.first = (yysemantic_stack_[(3) - (3)].ival);
                                      p->second.second = 0;
                                      (yyval.pival) = p;
                                    }
    break;

  case 71:

    {
                                      RPPival *p = new RPPival;
                                      p->first = (yysemantic_stack_[(3) - (1)].ival);
                                      p->second.first = (yysemantic_stack_[(3) - (3)].ival);
                                      p->second.second = 0;
                                      (yyval.pival) = p;
                                    }
    break;

  case 72:

    {
                                      RPPival *p = new RPPival;
                                      p->first = -(((yysemantic_stack_[(3) - (1)].ival)) + 1);
                                      p->second.first = (yysemantic_stack_[(3) - (3)].ival);
                                      p->second.second = 0;
                                      (yyval.pival) = p;
                                    }
    break;

  case 73:

    {
                                      (yyval.pival) = RPPivalFromParam((yysemantic_stack_[(6) - (1)].ival), (yysemantic_stack_[(6) - (3)].sval), (yysemantic_stack_[(6) - (5)].ival), errorString);
                                      if (!((yyval.pival))) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 74:

    {
                                      (yyval.pival) = RPPivalFromParam((yysemantic_stack_[(7) - (1)].ival), (yysemantic_stack_[(7) - (3)].sval), (yysemantic_stack_[(7) - (6)].ival), errorString);
                                      if (!((yyval.pival))) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
    break;

  case 75:

    {
                                      (yyval.pival) = RPPivalFromParam((yysemantic_stack_[(7) - (1)].ival), (yysemantic_stack_[(7) - (3)].sval), -((yysemantic_stack_[(7) - (6)].ival)), errorString);
                                      if (!((yyval.pival))) {
                                        error(yylloc, errorString);
                                       	YYERROR;
                                      }
                                    }
    break;



	default:
          break;
      }
    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

	if (yychar <= yyeof_)
	  {
	  /* Return failure if at end of input.  */
	  if (yychar == yyeof_)
	    YYABORT;
	  }
	else
	  {
	    yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
	    yychar = yyempty_;
	  }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (!yy_pact_value_is_default_ (yyn))
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystate_stack_.height () != 1)
      {
	yydestruct_ ("Cleanup: popping",
		   yystos_[yystate_stack_[0]],
		   &yysemantic_stack_[0],
		   &yylocation_stack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  RuleParser::yysyntax_error_ (int yystate, int yytoken)
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yychar.
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = 0;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char RuleParser::yypact_ninf_ = -56;
  const short int
  RuleParser::yypact_[] =
  {
       -22,    11,   -18,    32,    45,    33,    16,    82,   -56,    -8,
     -56,   -56,    -1,    11,   -56,   -18,   -56,    55,   110,    58,
     117,   117,   115,   119,   -56,   -56,   107,   -56,   -56,    80,
     167,   117,    60,   -56,   -56,    64,    -5,    85,     6,   -56,
     -56,   -56,   118,   -56,   120,   -56,   102,    81,   113,   -56,
       8,   -56,   117,    89,   -56,   -56,   107,   -56,   -56,   117,
     -56,   -56,   146,   -56,   117,   117,   117,   117,   117,   117,
      15,   117,   117,   117,   117,   -56,   -56,   115,   -56,   -56,
     119,    19,    41,   -56,   -56,    15,   131,   130,   144,   196,
     -56,   117,    91,   152,   -56,    73,    73,    73,    73,    73,
      73,   107,     7,   -56,    96,    96,   -56,   -56,   -56,   -56,
     137,   147,   148,   138,   153,   162,     9,   -56,   -56,   -56,
     196,   -56,   -56,    95,   -56,    61,   -56,   155,   156,   -56,
     157,   161,   -56,   -56,   107,   -56,   -56,   -56,   -56,   -56,
     114,   -56
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  RuleParser::yydefact_[] =
  {
         0,    47,    62,     0,     0,     0,     0,    56,    46,     0,
      61,     1,     0,    49,    36,    64,    37,     0,     0,     0,
       0,     0,    51,    66,    17,    18,     0,    11,    12,     0,
       0,     0,     0,     7,     9,     0,     0,     0,     0,    48,
      63,    57,     0,    72,     0,    71,     0,     0,    54,    50,
       0,    65,    34,     0,    13,    14,     0,    15,    16,     0,
       8,    10,     0,     2,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    38,    44,    53,    39,    45,
      68,     0,     0,     3,     4,     0,     0,     0,     0,    33,
      32,     0,     0,     0,    30,    23,    24,    25,    26,    27,
      28,     0,     0,    40,    19,    20,    21,    22,    52,    67,
       0,     0,     0,     0,     0,     0,     0,    55,    70,    69,
      35,    31,    29,     0,     6,     0,    58,     0,     0,    73,
       0,     0,     5,    41,     0,    42,    59,    60,    74,    75,
       0,    43
  };

  /* YYPGOTO[NTERM-NUM].  */
  const short int
  RuleParser::yypgoto_[] =
  {
       -56,   -56,   -15,   -55,   -56,   160,   106,   -30,   192,   -56,
     116,   182,   197,   -56,   128,   181
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  RuleParser::yydefgoto_[] =
  {
        -1,     3,    32,    53,     4,    33,   102,    34,    35,    36,
      49,     8,    37,    38,    51,    10
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const signed char RuleParser::yytable_ninf_ = -1;
  const unsigned char
  RuleParser::yytable_[] =
  {
        61,    92,     9,    20,    21,    46,    47,   124,    76,   132,
       1,    22,     2,    23,    18,    24,    62,    25,    26,    27,
      28,    79,    29,    30,    77,    19,    31,    22,     7,    23,
      87,     1,    11,     2,   101,    80,   125,    89,   125,   110,
     103,    88,   111,   112,    93,    15,   123,    16,    12,    95,
      96,    97,    98,    99,   100,   103,   104,   105,   106,   107,
      63,   113,    13,    14,   114,   115,    64,    65,    66,    67,
      68,    69,    41,    22,    42,    23,   120,    44,    45,   140,
     134,    84,    70,    71,    72,    73,    74,    64,    65,    66,
      67,    68,    69,    13,    75,   135,    71,    72,    73,    74,
      54,    55,    83,    85,    71,    72,    73,    74,    64,    65,
      66,    67,    68,    69,    15,    17,    78,    90,    91,   121,
      91,    73,    74,   133,    91,    71,    72,    73,    74,    22,
      43,    23,    48,    24,    52,    25,    26,    27,    28,    50,
      29,    30,   141,    91,    31,    81,    86,    82,   117,     1,
     118,     2,    64,    65,    66,    67,    68,    69,    64,    65,
      66,    67,    68,    69,   119,   126,   129,   127,   128,    71,
      72,    73,    74,   130,    94,    71,    72,    73,    74,    22,
     122,    23,   131,   136,   137,   138,    56,    57,    58,   139,
      60,   116,     5,   108,    59,    39,    40,     6,     0,     1,
       0,     2,    64,    65,    66,    67,    68,    69,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    71,
      72,    73,    74
  };

  /* YYCHECK.  */
  const short int
  RuleParser::yycheck_[] =
  {
        30,    56,    20,     4,     5,    20,    21,     0,    13,     0,
      32,    12,    34,    14,    22,    16,    31,    18,    19,    20,
      21,    15,    23,    24,    29,    33,    27,    12,    17,    14,
      22,    32,     0,    34,    19,    29,    29,    52,    29,    20,
      70,    33,    23,    24,    59,    29,   101,    31,     3,    64,
      65,    66,    67,    68,    69,    85,    71,    72,    73,    74,
       0,    20,    29,    30,    23,    24,     6,     7,     8,     9,
      10,    11,    17,    12,    19,    14,    91,    19,    20,   134,
      19,     0,    22,    23,    24,    25,    26,     6,     7,     8,
       9,    10,    11,    29,    30,   125,    23,    24,    25,    26,
      20,    21,     0,    22,    23,    24,    25,    26,     6,     7,
       8,     9,    10,    11,    29,    33,    31,    28,    29,    28,
      29,    25,    26,    28,    29,    23,    24,    25,    26,    12,
      20,    14,    17,    16,    27,    18,    19,    20,    21,    20,
      23,    24,    28,    29,    27,    27,    33,    27,    17,    32,
      20,    34,     6,     7,     8,     9,    10,    11,     6,     7,
       8,     9,    10,    11,    20,    28,    28,    20,    20,    23,
      24,    25,    26,    20,    28,    23,    24,    25,    26,    12,
      28,    14,    20,    28,    28,    28,    19,    20,    21,    28,
      30,    85,     0,    77,    27,    13,    15,     0,    -1,    32,
      -1,    34,     6,     7,     8,     9,    10,    11,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      24,    25,    26
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  RuleParser::yystos_[] =
  {
         0,    32,    34,    36,    39,    43,    47,    17,    46,    20,
      50,     0,     3,    29,    30,    29,    31,    33,    22,    33,
       4,     5,    12,    14,    16,    18,    19,    20,    21,    23,
      24,    27,    37,    40,    42,    43,    44,    47,    48,    46,
      50,    17,    19,    20,    19,    20,    37,    37,    17,    45,
      20,    49,    27,    38,    20,    21,    19,    20,    21,    27,
      40,    42,    37,     0,     6,     7,     8,     9,    10,    11,
      22,    23,    24,    25,    26,    30,    13,    29,    31,    15,
      29,    27,    27,     0,     0,    22,    33,    22,    33,    37,
      28,    29,    38,    37,    28,    37,    37,    37,    37,    37,
      37,    19,    41,    42,    37,    37,    37,    37,    45,    49,
      20,    23,    24,    20,    23,    24,    41,    17,    20,    20,
      37,    28,    28,    38,     0,    29,    28,    20,    20,    28,
      20,    20,     0,    28,    19,    42,    28,    28,    28,    28,
      38,    28
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  RuleParser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,    64,    43,    45,    42,    47,    40,    41,    44,
      93,   125,    91,    58,   123
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  RuleParser::yyr1_[] =
  {
         0,    35,    36,    36,    36,    36,    36,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    38,    38,    38,    39,    39,    40,    40,
      41,    41,    41,    41,    42,    42,    43,    43,    43,    43,
      44,    44,    44,    44,    45,    45,    46,    46,    46,    46,
      46,    47,    47,    47,    47,    48,    48,    48,    48,    49,
      49,    50,    50,    50,    50,    50
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  RuleParser::yyr2_[] =
  {
         0,     2,     4,     5,     5,     7,     6,     1,     2,     1,
       2,     1,     1,     2,     2,     2,     2,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       3,     4,     3,     2,     1,     3,     2,     2,     2,     2,
       1,     3,     3,     5,     2,     2,     2,     1,     3,     2,
       2,     1,     3,     2,     1,     3,     1,     3,     6,     7,
       7,     2,     1,     3,     2,     2,     1,     3,     2,     3,
       3,     3,     3,     6,     7,     7
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const RuleParser::yytname_[] =
  {
    "\"end of file\"", "error", "$undefined", "\"=\"", "CONS_FLAG",
  "BASE_FLAG", "GE", "LE", "EQ", "NE", "GT", "LT", "MARKER_OPEN",
  "MARKER_CLOSE", "SMARKER_OPEN", "SMARKER_CLOSE", "\"string\"",
  "\"element_string\"", "\"variable_string\"", "\"function\"",
  "\"integer\"", "\"double\"", "'@'", "'+'", "'-'", "'*'", "'/'", "'('",
  "')'", "','", "']'", "'}'", "'['", "':'", "'{'", "$accept", "rule",
  "exp", "parameters", "destination", "source", "markers", "marker",
  "elements", "markerElements", "simpleElement", "element", "elementsIds",
  "markerElementsIds", "simpleElementId", "elementId", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const RuleParser::rhs_number_type
  RuleParser::yyrhs_[] =
  {
        36,     0,    -1,    39,     3,    37,     0,    -1,    39,     3,
       4,    37,     0,    -1,    39,     3,     5,    37,     0,    -1,
      39,     3,     5,    37,    22,    41,     0,    -1,    39,     3,
      37,    22,    41,     0,    -1,    40,    -1,    24,    40,    -1,
      42,    -1,    24,    42,    -1,    20,    -1,    21,    -1,    23,
      20,    -1,    23,    21,    -1,    24,    20,    -1,    24,    21,
      -1,    16,    -1,    18,    -1,    37,    23,    37,    -1,    37,
      24,    37,    -1,    37,    25,    37,    -1,    37,    26,    37,
      -1,    37,     6,    37,    -1,    37,     7,    37,    -1,    37,
       8,    37,    -1,    37,     9,    37,    -1,    37,    10,    37,
      -1,    37,    11,    37,    -1,    24,    27,    37,    28,    -1,
      27,    37,    28,    -1,    24,    19,    38,    28,    -1,    19,
      38,    28,    -1,    27,    37,    -1,    27,    -1,    38,    29,
      37,    -1,    43,    30,    -1,    47,    31,    -1,    43,    30,
      -1,    47,    31,    -1,    42,    -1,    19,    38,    28,    -1,
      41,    29,    42,    -1,    41,    29,    19,    38,    28,    -1,
      44,    13,    -1,    48,    15,    -1,    32,    46,    -1,    32,
      -1,    43,    29,    46,    -1,    43,    29,    -1,    12,    45,
      -1,    12,    -1,    44,    29,    45,    -1,    44,    29,    -1,
      17,    -1,    17,    33,    17,    -1,    17,    -1,    17,    33,
      17,    -1,    17,    33,    19,    27,    20,    28,    -1,    17,
      33,    19,    27,    23,    20,    28,    -1,    17,    33,    19,
      27,    24,    20,    28,    -1,    34,    50,    -1,    34,    -1,
      47,    29,    50,    -1,    47,    29,    -1,    14,    49,    -1,
      14,    -1,    48,    29,    49,    -1,    48,    29,    -1,    20,
      33,    20,    -1,    20,    22,    20,    -1,    20,    33,    20,
      -1,    20,    22,    20,    -1,    20,    33,    19,    27,    20,
      28,    -1,    20,    33,    19,    27,    23,    20,    28,    -1,
      20,    33,    19,    27,    24,    20,    28,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned short int
  RuleParser::yyprhs_[] =
  {
         0,     0,     3,     8,    14,    20,    28,    35,    37,    40,
      42,    45,    47,    49,    52,    55,    58,    61,    63,    65,
      69,    73,    77,    81,    85,    89,    93,    97,   101,   105,
     110,   114,   119,   123,   126,   128,   132,   135,   138,   141,
     144,   146,   150,   154,   160,   163,   166,   169,   171,   175,
     178,   181,   183,   187,   190,   192,   196,   198,   202,   209,
     217,   225,   228,   230,   234,   237,   240,   242,   246,   249,
     253,   257,   261,   265,   272,   280
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  RuleParser::yyrline_[] =
  {
         0,   164,   164,   168,   172,   176,   180,   190,   191,   197,
     198,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     218,   224,   230,   236,   242,   248,   254,   260,   266,   272,
     278,   281,   292,   303,   308,   312,   319,   322,   328,   331,
     338,   343,   356,   360,   375,   382,   393,   398,   403,   407,
     414,   419,   424,   428,   435,   443,   454,   462,   470,   477,
     484,   494,   499,   504,   508,   515,   520,   525,   529,   536,
     543,   553,   560,   567,   574,   581
  };

  // Print the state stack on the debug stream.
  void
  RuleParser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  RuleParser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  RuleParser::token_number_type
  RuleParser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      27,    28,    25,    23,    29,    24,     2,    26,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    33,     2,
       2,     2,     2,     2,    22,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    32,     2,    30,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    34,     2,    31,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int RuleParser::yyeof_ = 0;
  const int RuleParser::yylast_ = 222;
  const int RuleParser::yynnts_ = 16;
  const int RuleParser::yyempty_ = -2;
  const int RuleParser::yyfinal_ = 11;
  const int RuleParser::yyterror_ = 1;
  const int RuleParser::yyerrcode_ = 256;
  const int RuleParser::yyntokens_ = 35;

  const unsigned int RuleParser::yyuser_token_number_max_ = 276;
  const RuleParser::token_number_type RuleParser::yyundef_token_ = 2;


} // yy





void  yy::RuleParser::error (const yy::RuleParser::location_type& l, const string& m) {
  driver.error(l, m);
}


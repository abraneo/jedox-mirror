/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloBrowser/RuleBrowserJob.h"

#include "Exceptions/ParameterException.h"

#include "Collections/StringUtils.h"

#include "PaloDocumentation/HtmlFormatter.h"

#include "Olap/Dimension.h"
#include "Olap/Rule.h"

#include "Parser/RuleParserDriver.h"

#include "PaloDocumentation/RuleBrowserDocumentation.h"

namespace palo {

bool RuleBrowserJob::initialize()
{
	bool ok = PaloJob::initialize();

	if (!ok) {
		return false;
	}

	if (jobRequest->action) {
		action = *(jobRequest->action);
	}

	if (!action.empty()) {
		if (action == "delete" || action == "activate") {
			jobType = WRITE_JOB;
		}
	}
	return true;
}

void RuleBrowserJob::compute()
{
	findCube(true, false);

	string message;
	string ruleStringMessage;

	if (jobRequest->action) {
		string action = *(jobRequest->action);
		clear();
		server = Context::getContext()->getServerCopy();
		findDatabase(true, true);
		findCube(true, true);

		if (action == "delete") {
			cube->deleteRule(server, database, jobRequest->rule, PUser(), true);
			message = createHtmlMessage("Info", "rule deleted");
		} else if (action == "activate") {
			findRules(true);
			if (rules[0]->isActive()) {
				cube->activateRules(server, database, rules, INACTIVE, PUser(), NULL, false, true);
				message = createHtmlMessage("Info", "rule deactivated");
			} else {
				string errMsg;
				if (!cube->activateRules(server, database, rules, ACTIVE, PUser(), &errMsg, false, true)) {
					message = createHtmlMessage("Error", errMsg);
				} else {
					message = createHtmlMessage("Info", "rule activated");
				}
			}
        } else if ( action == "reset" ) {
        	vector<PRule> rules = cube->getRules(PUser(), false);
            for ( vector<PRule>::iterator rule = rules.begin(); rule != rules.end(); ++rule ) {
            	(*rule)->resetCounter();
            }
        }
		if (!server->commit()) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
		}
	}

	if (jobRequest->definition) {
		string definition = *(jobRequest->definition);

		ruleStringMessage = StringUtils::escapeHtml(definition);

		if (!ruleStringMessage.empty()) {
			RuleParserDriver driver;

			driver.parse(definition);
			PRuleNode r(driver.getResult());

			if (r) {
				clear();
				server = Context::getContext()->getServerCopy();
				findDatabase(true, true);
				findCube(true, true);

				// validate parse tree
				string errorMsg;
				bool ok = r->validate(server, database, cube, errorMsg);

				if (!ok) {
					message = createHtmlMessage("Error", errorMsg);
				} else {
					string defaultComment = "generated via server browser";
					string &comment = jobRequest->comment ? *jobRequest->comment : defaultComment;
					if (jobRequest->rule == NO_IDENTIFIER) {
						cube->createRule(server, database, r, definition, "PALO", comment, true, PUser(), true, 0, driver.getPosition());
					} else {
						findRules(true);
						if (rules.size()) {
							PRule rule = rules[0];
							double position = rule->getPosition();
							if (jobRequest->dPositions && jobRequest->dPositions->size()) {
								position = jobRequest->dPositions->at(0);
							}
							if (driver.getPosition()) {
								position = driver.getPosition();
							}
							cube->modifyRule(server, database, rule, r, definition, rule->getExternal(), comment, PUser(), ACTIVE, true, position);
						}
					}
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = createHtmlMessage("Info", "added new rule");
				}
			} else {
				message = createHtmlMessage("Error", driver.getErrorMessage());
			}
		}
	}
    if (jobRequest->action || jobRequest->definition) {
    	server->invalidateCache();

        response = new HttpResponse( HttpResponse::FOUND );
        StringBuffer redirectUrl;
        redirectUrl.appendText("/browser/rule?database=");
        redirectUrl.appendInteger(database->getIdentifier());
        redirectUrl.appendText("&cube=");
        redirectUrl.appendInteger(cube->getId());
        redirectUrl.appendText("\r\n");
        response->setHeaderField(pair<string, string>("Location", redirectUrl.c_str()));
        return;
    }

	RuleBrowserDocumentation sbd(database, cube, ruleStringMessage, message, jobRequest->mode, jobRequest->useRules);
	HtmlFormatter hf(templatePath + "/browser_cube_rule.tmpl");

	generateResult(hf.getDocumentation(&sbd));
}

}

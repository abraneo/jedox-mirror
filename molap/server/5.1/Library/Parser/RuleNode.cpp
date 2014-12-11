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

#include "Parser/RuleNode.h"
#include "Collections/DeleteObject.h"
#include "Parser/SourceNode.h"

namespace palo {

RuleNode::RuleNode(RuleOption ruleOption, DestinationNode *destination, ExprNode *expr) :
	destinationNode(destination), exprNode(expr), ruleOption(ruleOption)
{
}

RuleNode::RuleNode(RuleOption ruleOption, DestinationNode *destination, ExprNode *expr, vector<Node*>* markers) :
	destinationNode(destination), exprNode(expr), ruleOption(ruleOption), externalMarkers(*markers)
{
}

RuleNode::~RuleNode()
{
	if (destinationNode) {
		delete destinationNode;
	}

	if (exprNode) {
		delete exprNode;
	}

	for_each(externalMarkers.begin(), externalMarkers.end(), DeleteObject());
}

Node * RuleNode::clone()
{
	RuleNode * cloned = new RuleNode(this->ruleOption, dynamic_cast<DestinationNode*>(this->destinationNode->clone()), dynamic_cast<ExprNode*>(this->exprNode));
	cloned->valid = this->valid;
	return cloned;
}

bool RuleNode::validate(PServer server, PDatabase database, PCube cube, Node* destination, string& message)
{
	if (destinationNode->validate(server, database, cube, 0, message) && exprNode->validate(server, database, cube, destinationNode, message)) {
		valid = true;
	}

	if (valid && !externalMarkers.empty()) {
		for (vector<Node*>::iterator i = externalMarkers.begin(); i != externalMarkers.end(); ++i) {
			Node* node = *i;

			if (!node->validate(server, database, cube, 0, message)) {
				valid = false;
				break;
			}

			if (node->getNodeType() == NODE_SOURCE_NODE) {
				SourceNode* src = dynamic_cast<SourceNode*>(node);

				if (!src->isMarker()) {
					message = "only markered source areas or PALO.MARKER are allowed, got unmarked source area";
					valid = false;
					break;
				}
			} else if (node->getNodeType() != NODE_FUNCTION_PALO_MARKER) {
				message = "only markered source areas or PALO.MARKER are allowed";
				valid = false;
				break;
			}
		}
	}

	if (valid) {
		internalMarkers.clear();
		exprNode->collectMarkers(internalMarkers);
	}

	return valid;
}

bool RuleNode::validate(PServer server, PDatabase database, PCube cube, string& message)
{
	return validate(server, database, cube, 0, message);
}

bool RuleNode::hasElement(CPDimension dimension, IdentifierType element) const
{
	if (destinationNode != 0 && destinationNode->hasElement(dimension, element)) {
		return true;
	}

	if (exprNode != 0 && exprNode->hasElement(dimension, element)) {
		return true;
	}

	for (vector<Node*>::const_iterator i = externalMarkers.begin(); i != externalMarkers.end(); ++i) {
		Node* node = *i;

		if (node->hasElement(dimension, element)) {
			return true;
		}
	}

	return false;
}

void RuleNode::appendXmlRepresentation(StringBuffer* sb, int ident, bool names)
{
	switch (ruleOption) {
	case CONSOLIDATION:
		sb->appendText("<rule path=\"none-base\">");
		break;
	case BASE:
		sb->appendText("<rule path=\"base\">");
		break;
	default:
		sb->appendText("<rule>");
	}
	sb->appendEol();

	destinationNode->appendXmlRepresentation(sb, 1, names);

	identXML(sb, 1);
	sb->appendText("<definition>");
	sb->appendEol();

	exprNode->appendXmlRepresentation(sb, 2, names);

	identXML(sb, 1);
	sb->appendText("</definition>");
	sb->appendEol();

	if (!externalMarkers.empty()) {
		identXML(sb, 1);
		sb->appendText("<external-markers>");
		sb->appendEol();

		for (vector<Node*>::iterator i = externalMarkers.begin(); i != externalMarkers.end(); ++i) {
			(*i)->appendXmlRepresentation(sb, 2, names);
		}

		identXML(sb, 1);
		sb->appendText("</external-markers>");
		sb->appendEol();
	}

	sb->appendText("</rule>");
	sb->appendEol();
}

void RuleNode::appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const
{
	destinationNode->appendRepresentation(sb, db, cube);

	sb->appendText(" = ");

	switch (ruleOption) {
	case CONSOLIDATION:
		sb->appendText("C:");
		break;

	case BASE:
		sb->appendText("B:");
		break;

	default:
		break;
	}

	exprNode->appendRepresentation(sb, db, cube);

	if (!externalMarkers.empty()) {
		sb->appendText(" @ ");

		const char * sep = "";

		for (vector<Node*>::const_iterator i = externalMarkers.begin(); i != externalMarkers.end(); ++i) {
			Node* node = *i;

			sb->appendText(sep);
			node->appendRepresentation(sb, db, cube);
			sep = ",";
		}
	}
}

uint32_t RuleNode::guessType(uint32_t level)
{
	Logger::trace << "guessType " << "level " << level << "node " << "RuleNode" << " type " << "none" << endl;
	destinationNode->guessType(level + 1);
	exprNode->guessType(level + 1);
	return Node::NODE_UNKNOWN_VALUE;
}

bool RuleNode::genCode(bytecode_generator& generator, uint8_t want) const
{
	RulesContext mem_context;
	paloLegacy::EBorder *border;
	uint8_t dim;
	paloLegacy::ERule* erule = generator.get_rule();

	erule->marker_flag = 0;
	erule->ubm_flag = 0;
	erule->ubm_rulesTested = 0;
	erule->ubm_noMoreRules = 0;

	IdentifiersType* destElementIDs = destinationNode->getElementIDs();
	IdentifiersType::const_iterator destElementIDsIter = destElementIDs->begin();
	vector<uint8_t> *destIsRestricted = destinationNode->getRestriction();
	vector<uint8_t>::const_iterator destIsRestrictedIter = destIsRestricted->begin();

	erule->dest_area = new paloLegacy::EArea(erule->cube);

	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(CONST_COMMITABLE_CAST(Cube, erule->cube->acube->shared_from_this())));
	for (dim = 0; dim < erule->cube->nrDimensions; dim++) {

		if (*destIsRestrictedIter) {

			erule->ubm_mask[dim] = 1;

			erule->ubm_dest[dim] = *destElementIDsIter;
			Element *destElm = db->lookupDimension(erule->cube->acube->getDimensions()->at(dim), false)->lookupElement(*destElementIDsIter, false);
			if (destElm) {
				erule->ubm_dest_is_base[dim] = (destElm->getElementType() != Element::CONSOLIDATED);
			} else {
				erule->ubm_dest_is_base[dim] = 0;
			}

			border = new paloLegacy::EBorder(erule->cube->dimensions[dim]);
			if (erule->ubm_dest_is_base[dim]) {
				border->nrBase = 1;
				border->singleBaseId = erule->ubm_dest[dim];
			} else {
				border->nrCons = 1;
				border->singleConsId = erule->ubm_dest[dim];
			}
			border->size = border->nrBase + border->nrCons;
			erule->dest_area->borders[dim] = border;

		} else {

			erule->ubm_mask[dim] = 0;

			erule->ubm_dest[dim] = 0;
			erule->ubm_dest_is_base[dim] = 1;

			border = new paloLegacy::EBorder(erule->cube->dimensions[dim]);
			border->nrCons = border->dimension->nrCons;
			border->pconss = border->dimension->pconss;
			border->nrBase = border->dimension->nrBase;
			border->pbases = border->dimension->pbases;
			border->size = border->nrBase + border->nrCons;
			erule->dest_area->borders[dim] = border;

		}

		++destElementIDsIter;
		++destIsRestrictedIter;

	}

	if (internalMarkers.size() > 0 || externalMarkers.size() > 0) {
		erule->marker_flag = 1;
	}

	/* if we have a unique marker, check whether it is a bijection       */
	/* and set up the ubm_source attributes                            */
	if (internalMarkers.size() == 1 && externalMarkers.size() == 0) {
		erule->ubm_flag = 1;

		/* XXX this code will not work with Palo.Marker XXX */
		AreaNode * sourceNode = dynamic_cast<AreaNode*>(internalMarkers[0]);

		if (sourceNode != 0) {
			IdentifiersType* sourceElementIDs = sourceNode->getElementIDs();
			IdentifiersType::const_iterator sourceElementIDsIter = sourceElementIDs->begin();
			vector<uint8_t> *sourceIsRestricted = sourceNode->getRestriction();
			vector<uint8_t>::const_iterator sourceIsRestrictedIter = sourceIsRestricted->begin();

			for (dim = 0; dim < erule->cube->nrDimensions; dim++) {
				if (*sourceIsRestrictedIter) {

					erule->ubm_source[dim] = *sourceElementIDsIter;
					Element *sourceElm = db->lookupDimension(erule->cube->acube->getDimensions()->at(dim), false)->lookupElement(*sourceElementIDsIter, false);
					if (sourceElm) {
						erule->ubm_source_is_base[dim] = (sourceElm->getElementType() != Element::CONSOLIDATED);
					} else {
						erule->ubm_source_is_base[dim] = 0;
					}

					/* if the source is restricted but the destination is not */
					/* then the marker is not a bijection mapping             */
					if (!erule->ubm_mask[dim]) {
						erule->ubm_flag = 0;
					}

				} else if (erule->ubm_mask[dim]) {
					/* if the destination is restricted but the source is not  */
					/* then the source is effectively restricted anyhow        */
					erule->ubm_source[dim] = erule->ubm_dest[dim];
					erule->ubm_source_is_base[dim] = erule->ubm_dest_is_base[dim];
				} else {
					erule->ubm_source[dim] = 0;
					erule->ubm_source_is_base[dim] = 1;
				}

				++sourceElementIDsIter;
				++sourceIsRestrictedIter;
			}
		} else {
			erule->ubm_flag = 0;
		}
	}

	if (!exprNode->genCode(generator, want))
		return false;

	return generator.EmitHaltCode();

}

}

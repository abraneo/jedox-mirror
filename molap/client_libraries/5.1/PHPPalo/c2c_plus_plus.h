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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * 
 *
 */

#ifndef C2C_PLUS_PLUS_H
#define C2C_PLUS_PLUS_H

#ifdef __cplusplus
extern "C" {
#endif

enum PHPPaloGoalSeekType {
	PHPPalo_GOALSEEK_COMPLETE, PHPPalo_GOALSEEK_EQUAL, PHPPalo_GOALSEEK_RELATIVE
};

void ChangeToNewStartIndex(void);

void InternalPaloWeb(long value);
void ActivateLogStderr(long value);

void PHPInitSSL(const char * trustfile);

long GetGoalSeekType(enum PHPPaloGoalSeekType type);

#ifdef __cplusplus
}
#endif

#endif

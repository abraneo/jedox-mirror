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
 * \author Marko Stijak, Banja Luka, Bosnia and Herzegovina
 * 
 *
 */

#ifndef OLAP_GOALSEEKSOLVER_H
#define OLAP_GOALSEEKSOLVER_H 1

#include <vector>
#include <list>
#include <map>
#include <set>
#include <math.h>

namespace palo {
namespace goalseeksolver {

class CalculationTimeoutException {
};

namespace eps {
const static double eps = 1e-7;
bool equal(const double& a, const double& b);

bool zero(const double& a);
}

//multidimensional matrix
template<class T>
class MDM {
	std::vector<int> ec;
	std::vector<T> el;
public:
	MDM() {
	}
	;
	MDM(std::vector<int> ec) :
		ec(ec) {
		el.resize(count());
	}

	int count() const {
		int c = 1;
		for (int i = 0; i < (int)ec.size(); i++)
			c *= ec[i];
		return c;
	}

	T& operator[](const std::vector<int>& coord) {
		int ind = get_index(coord);
		return el[ind];
	}

	T& operator[](int ind) {
		return el[ind];
	}

	const T& operator[](const std::vector<int>& coord) const {
		int ind = get_index(coord);
		return el[ind];
	}

	const T& operator[](int ind) const {
		return el[ind];
	}

	int get_index(const std::vector<int>& coord) const {
		int ind = 0;
		int l = (int)ec.size() - 1;
		for (int i = 0; i < l; i++) {
			ind += coord[i];
			ind *= ec[i + 1];
		}
		ind += coord[l];
		return ind;
	}

	std::vector<int> get_coords(int ind) const {
		std::vector<int> res;
		res.resize(ec.size());
		int l = (int)ec.size() - 1;
		for (int i = 0; i < l; i++) {
			res[l - i] = ind % ec[l - i];
			ind /= ec[l - i];
		}
		res[0] = ind;
		return res;
	}
};

template<class T> class M2D {
public:
	typedef std::vector<T> RowType;
	typedef std::vector<RowType> RowListType;
public:
	RowListType row;
	int colCount;

public:
	M2D() :
		colCount(0) {
	}
	;
	M2D(int c) :
		colCount(c) {
	}
	;
	M2D(int c, int r) :
		colCount(c) {
		row_count(r);
	}
	;
	int col_count() const {
		return colCount;
	}
	;
	void col_count(int c) {
		colCount = c;
		for (int i = 0; i < (int)row.size(); i++)
			row[i].resize(c);
	}
	int row_count() const {
		return (int)row.size();
	}
	;
	void row_count(int r) {
		int otr = (int)row.size();
		row.resize(r);
		if (otr < r)
			col_count(colCount);
	}

	void append(const RowType& nr) {
		row.push_back(nr);
	}
	;

	RowType& operator[](int index) {
		return row[index];
	}
	;
	const RowType& operator[](int index) const {
		return row[index];
	}
	;

	void swap_rows(int i, int j) {
		std::swap(row[i], row[j]);
	}

	RowType empty_row() {
		RowType r;
		r.resize(colCount);
		return r;
	}

	void insert_row(int at, const RowType& r) {
		row.push_back(r);
		swap_rows(at, (int)row.size() - 1);
	}

	void eliminate_column(int pivot_row, int row, int column) {
		double f = -this->row[row][column] / this->row[pivot_row][column];
		if (eps::zero(f))
			return;
		for (int c = 0; c < colCount; c++)
			this->row[row][c] += f * this->row[pivot_row][c];
	}

	void remove_trailing_zero_rows() {
		while (!row.empty()) {
			const RowType& r = row.back();
			for (int c = 0; c < colCount; c++)
				if (!eps::zero(r[c]))
					return;
			row.pop_back();
		}
	}
};

struct Problem {
	std::vector<std::vector<double> > dimensionElementWeight;
	MDM<double> cellValue;
	std::vector<int> fixedCoord;
	double fixedValue;
};

struct Result {
	bool valid;
	MDM<double> cellValue;
};

Result solve(const Problem& p, int timeoutMiliSec);
}
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////

void PALO_ARG_FUNC(free_arg_palo_dataset_contents)(struct arg_palo_dataset *ds) {
	if (ds != NULL) {
		PALO_ARG_FUNC(free_arg_str_array_contents)(&ds->coordinates);
		PALO_ARG_FUNC(free_arg_palo_value_contents)(&ds->value);
	}
}


void PALO_ARG_FUNC(free_arg_palo_dataset_array_contents)(struct arg_palo_dataset_array *dsa) {
	size_t i;

	if (dsa != NULL) {
		if (dsa->a != NULL) {
			for (i = 0; i < dsa->len; i++) {
				PALO_ARG_FUNC(free_arg_palo_dataset_contents)(dsa->a + i);
			}
			free(dsa->a);
			dsa->a = NULL;
		}
		dsa->len = 0;
	}
}


void PALO_ARG_FUNC(free_arg_str)(CHAR_T *s) {
	free(s);
	s = NULL;
}


void PALO_ARG_FUNC(free_arg_str_array_contents)(struct arg_str_array *sa) {
	size_t i;

	if (sa != NULL) {
		if (sa->a != NULL) {
			for (i = 0; i < sa->len; i++) {
				PALO_ARG_FUNC(free_arg_str)(sa->a[i]);
			}
			free(sa->a);
			sa->a = NULL;
		}
		sa->len = 0;
	}
}


void PALO_ARG_FUNC(free_arg_dim_element_info_contents)(struct arg_dim_element_info *dei) {
	if (dei != NULL) {
		PALO_ARG_FUNC(free_arg_str)(dei->name);
	}
}


void PALO_ARG_FUNC(free_arg_dim_element_info2_raw_contents)(struct arg_dim_element_info2_raw *dei) {
	if (dei != NULL) {
		PALO_ARG_FUNC(free_arg_str)(dei->name);
		if (dei->parents != NULL) {
			free(dei->parents);
			dei->parents = NULL;
		}
		if (dei->children != NULL) {
			free(dei->children);
			dei->children = NULL;
		}
	}
}


void PALO_ARG_FUNC(free_arg_getdata_export_result_contents)(struct arg_getdata_export_result *exres) {
	if (exres != NULL) {
		PALO_ARG_FUNC(free_arg_palo_dataset_array_contents)(&exres->results);
	}
}


void PALO_ARG_FUNC(free_arg_dim_element_info_array_contents)(struct arg_dim_element_info_array *deia) {
	size_t i;

	if (deia != NULL) {
		if (deia->a != NULL) {
			for (i = 0; i < deia->len; i++) {
				PALO_ARG_FUNC(free_arg_dim_element_info_contents)(deia->a + i);
			}
			free(deia->a);
			deia->a = NULL;
		}
		deia->len = 0;
	}
}


void PALO_ARG_FUNC(free_arg_dim_element_info2_raw_array_contents)(struct arg_dim_element_info2_raw_array *deia) {
	size_t i;

	if (deia != NULL) {
		if (deia->a != NULL) {
			for (i = 0; i < deia->len; i++) {
				PALO_ARG_FUNC(free_arg_dim_element_info2_raw_contents)(deia->a + i);
			}
			free(deia->a);
			deia->a = NULL;
		}
		deia->len = 0;
	}
}

void PALO_ARG_FUNC(free_arg_consolidation_element_info_contents)(struct arg_consolidation_element_info *cei) {
	if (cei != NULL) {
		PALO_ARG_FUNC(free_arg_str)(cei->name);
	}
}


void PALO_ARG_FUNC(free_arg_consolidation_element_info_array_contents)(struct arg_consolidation_element_info_array *ceia) {
	size_t i;

	if (ceia != NULL) {
		if (ceia->a != NULL) {
			for (i = 0; i < ceia->len; i++) {
				PALO_ARG_FUNC(free_arg_consolidation_element_info_contents)(ceia->a + i);
			}
			free(ceia->a);
			ceia->a = NULL;
		}
		ceia->len = 0;
	}
}


void PALO_ARG_FUNC(free_arg_palo_value_array_contents)(struct arg_palo_value_array *pva) {
	size_t i;

	if (pva != NULL) {
		if (pva->a != NULL) {
			for (i = 0; i < pva->len; i++) {
				PALO_ARG_FUNC(free_arg_palo_value_contents)(pva->a + i);
			}
			free(pva->a);
			pva->a = NULL;
		}
		pva->len = 0;
	}
}


void PALO_ARG_FUNC(free_arg_str_array_2d_contents)(struct arg_str_array_2d *sa) {
	size_t i;

	if (sa != NULL) {
		if (sa->a != NULL) {
			for (i = 0; i < sa->rows*sa->cols; i++) {
				PALO_ARG_FUNC(free_arg_str)(sa->a[i]);
			}
			free(sa->a);
			sa->a = NULL;
		}
		sa->rows = 0;
		sa->cols = 0;
	}
}


void PALO_ARG_FUNC(free_arg_str_array_array_contents)(struct arg_str_array_array *sa) {
	size_t i;

	if (sa != NULL) {
		if (sa->a != NULL) {
			for (i = 0; i < sa->len; i++) {
				PALO_ARG_FUNC(free_arg_str_array_contents)(sa->a + i);
			}
			free(sa->a);
			sa->a = NULL;
		}
		sa->len = 0;
	}
}


void PALO_ARG_FUNC(free_arg_palo_value_contents)(struct arg_palo_value *pv) {
	if (pv != NULL) {
		if (pv->type == argPaloValueTypeStr) {
			if (pv->val.s != NULL) {
				PALO_ARG_FUNC(free_arg_str)(pv->val.s);
			}
		} else if (pv->type == argPaloValueTypeError) {
			free_arg_error_contents(&pv->val.err);
		}
	}
}


void PALO_ARG_FUNC(free_arg_rule_info_contents)(struct arg_rule_info *ri) {
	if (ri != NULL) {
		PALO_ARG_FUNC(free_arg_str)(ri->definition);
		PALO_ARG_FUNC(free_arg_str)(ri->extern_id);
		PALO_ARG_FUNC(free_arg_str)(ri->comment);
	}
}


void PALO_ARG_FUNC(free_arg_lock_info_contents)(struct arg_lock_info *li) {
	if (li != NULL) {
		PALO_ARG_FUNC(free_arg_str)(li->user);
		PALO_ARG_FUNC(free_arg_str_array_array_contents)(&(li->area));
	}
}


void PALO_ARG_FUNC(free_arg_db_info_contents)(struct arg_db_info *dbi) {
	if (dbi != NULL) {
		PALO_ARG_FUNC(free_arg_str)(dbi->name);
	}
}


void PALO_ARG_FUNC(free_arg_dim_info_contents)(struct arg_dim_info *di) {
	if (di != NULL) {
		PALO_ARG_FUNC(free_arg_str)(di->name);
		PALO_ARG_FUNC(free_arg_str)(di->assoc_dimension);
		PALO_ARG_FUNC(free_arg_str)(di->attribut_cube);
		PALO_ARG_FUNC(free_arg_str)(di->rights_cube);
	}
}

void PALO_ARG_FUNC(free_arg_dim_info_simple_contents)(struct arg_dim_info_simple *di) {
	if (di != NULL) {
		PALO_ARG_FUNC(free_arg_str)(di->name);
	}
}


void PALO_ARG_FUNC(free_arg_cube_info_contents)(struct arg_cube_info *ci) {
	if (ci != NULL) {
		PALO_ARG_FUNC(free_arg_str)(ci->name);
		PALO_ARG_FUNC(free_arg_str_array_contents)(&(ci->dimensions));
	}
}


void PALO_ARG_FUNC(free_arg_rule_info_array_contents)(struct arg_rule_info_array *ria) {
	size_t i;

	if (ria != NULL) {
		if (ria->a != NULL) {
			for (i = 0; i < ria->len; i++) {
				PALO_ARG_FUNC(free_arg_rule_info_contents)(ria->a + i);
			}
			free(ria->a);
			ria->a = NULL;
		}
		ria->len = 0;
	}
}


void PALO_ARG_FUNC(free_arg_lock_info_array_contents)(struct arg_lock_info_array *lia) {
	size_t i;

	if (lia != NULL) {
		if (lia->a != NULL) {
			for (i = 0; i < lia->len; i++) {
				PALO_ARG_FUNC(free_arg_lock_info_contents)(lia->a + i);
			}
			free(lia->a);
			lia->a = NULL;
		}
		lia->len = 0;
	}
}

void PALO_ARG_FUNC(free_arg_subset_result_contents)(struct arg_subset_result *sr) {
	if (sr != NULL) {
		PALO_ARG_FUNC(free_arg_str)(sr->name);
		PALO_ARG_FUNC(free_arg_str)(sr->alias);
		PALO_ARG_FUNC(free_arg_str)(sr->path);
	}
}


void PALO_ARG_FUNC(free_arg_subset_result_array_contents)(struct arg_subset_result_array *sra) {
	size_t i;

	if (sra != NULL) {
		if (sra->a != NULL) {
			for (i = 0; i < sra->len; i++) {
				PALO_ARG_FUNC(free_arg_subset_result_contents)(&(sra->a[i]));
			}
			free(sra->a);
			sra->a = NULL;
		}
		sra->len = 0;
	}
}

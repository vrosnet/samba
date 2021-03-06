/* 
   Unix SMB/CIFS mplementation.

   implement possibleInferiors calculation
   
   Copyright (C) Andrew Tridgell 2009
   Copyright (C) Andrew Bartlett <abartlet@samba.org> 2009

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   
*/
/*
  This module is a C implementation of the logic in the
  dsdb/samdb/ldb_modules/tests/possibleInferiors.py code

  To understand the C code, please see the python code first
 */

#include "includes.h"
#include "dsdb/samdb/samdb.h"


/*
  create the SUPCLASSES() list
 */
static char **schema_supclasses(struct dsdb_schema *schema, struct dsdb_class *schema_class)
{
	char **list;

	if (schema_class->supclasses) {
		return schema_class->supclasses;
	}

	list = str_list_make(schema_class, NULL, NULL);
	if (list == NULL) {
		DEBUG(0,(__location__ " out of memory\n"));
		return NULL;
	}

	/* Cope with 'top SUP top', ie top is subClassOf top */ 
	if (strcmp(schema_class->lDAPDisplayName, schema_class->subClassOf) == 0) {
		schema_class->supclasses = list;
		return list;
	}

	if (schema_class->subClassOf) {
		const char **list2;
		list = str_list_add_const(list, schema_class->subClassOf);

		list2 = schema_supclasses(schema,  
					  discard_const_p(struct dsdb_class, 
							  dsdb_class_by_lDAPDisplayName(schema, 
											schema_class->subClassOf)));
		list = str_list_append_const(list, list2);
	}

	schema_class->supclasses = str_list_unique(list);
	
	return list;
}

/*
  this one is used internally
  matches SUBCLASSES() python function
 */
static char **schema_subclasses(struct dsdb_schema *schema, TALLOC_CTX *mem_ctx, char **oclist)
{
	char **list = str_list_make(mem_ctx, NULL, NULL);
	int i;

	for (i=0; oclist && oclist[i]; i++) {
		struct dsdb_class *schema_class = dsdb_class_by_lDAPDisplayName(schema, oclist[i]);
		list = str_list_append_const(list, schema_class->subclasses);
	}
	return list;
}


/* 
   equivalent of the POSSSUPERIORS() python function
 */
static char **schema_posssuperiors(struct dsdb_schema *schema, 
				   struct dsdb_class *schema_class)
{
	if (schema_class->posssuperiors == NULL) {
		char **list2 = str_list_make(schema_class, NULL, NULL);
		char **list3;
		int i;

		list2 = str_list_append_const(list2, schema_class->systemPossSuperiors);
		list2 = str_list_append_const(list2, schema_class->possSuperiors);
		list3 = schema_supclasses(schema, schema_class);
		for (i=0; list3 && list3[i]; i++) {
			struct dsdb_class *class2 = dsdb_class_by_lDAPDisplayName(schema, list3[i]);
			list2 = str_list_append_const(list2, schema_posssuperiors(schema, class2));
		}
		list2 = str_list_append_const(list2, schema_subclasses(schema, list2, list2));

		schema_class->posssuperiors = str_list_unique(list2);
	}

	return schema_class->posssuperiors;
}

static char **schema_subclasses_recurse(struct dsdb_schema *schema, struct dsdb_class *schema_class)
{
	char **list = str_list_copy_const(schema_class, schema_class->subclasses_direct);
	int i;
	for (i=0;list && list[i]; i++) {
		struct dsdb_class *schema_class2 = dsdb_class_by_lDAPDisplayName(schema, list[i]);
		if (schema_class != schema_class2) {
			list = str_list_append_const(list, schema_subclasses_recurse(schema, schema_class2));
		}
	}
	return list;
}

static void schema_create_subclasses(struct dsdb_schema *schema)
{
	struct dsdb_class *schema_class;

	for (schema_class=schema->classes; schema_class; schema_class=schema_class->next) {
		struct dsdb_class *schema_class2 = dsdb_class_by_lDAPDisplayName(schema, schema_class->subClassOf);
		if (schema_class != schema_class2) {
			if (schema_class2->subclasses_direct == NULL) {
				schema_class2->subclasses_direct = str_list_make(schema_class2, NULL, NULL);
			}
			schema_class2->subclasses_direct = str_list_add_const(schema_class2->subclasses_direct, 
									schema_class->lDAPDisplayName);
		}
	}

	for (schema_class=schema->classes; schema_class; schema_class=schema_class->next) {
		schema_class->subclasses = str_list_unique(schema_subclasses_recurse(schema, schema_class));
	}	
}

static void schema_fill_possible_inferiors(struct dsdb_schema *schema, struct dsdb_class *schema_class)
{
	struct dsdb_class *c2;

	for (c2=schema->classes; c2; c2=c2->next) {
		char **superiors = schema_posssuperiors(schema, c2);
		if (c2->systemOnly == false 
		    && c2->objectClassCategory != 2 
		    && c2->objectClassCategory != 3
		    && str_list_check(superiors, schema_class->lDAPDisplayName)) {
			if (schema_class->possibleInferiors == NULL) {
				schema_class->possibleInferiors = str_list_make(schema_class, NULL, NULL);
			}
			schema_class->possibleInferiors = str_list_add_const(schema_class->possibleInferiors,
								       c2->lDAPDisplayName);
		}
	}
	schema_class->possibleInferiors = str_list_unique(schema_class->possibleInferiors);
}

void schema_fill_constructed(struct dsdb_schema *schema) 
{
	struct dsdb_class *schema_class;

	schema_create_subclasses(schema);

	for (schema_class=schema->classes; schema_class; schema_class=schema_class->next) {
		schema_fill_possible_inferiors(schema, schema_class);
	}

	/* free up our internal cache elements */
	for (schema_class=schema->classes; schema_class; schema_class=schema_class->next) {
		talloc_free(schema_class->supclasses);
		talloc_free(schema_class->subclasses_direct);
		talloc_free(schema_class->subclasses);
		talloc_free(schema_class->posssuperiors);
		schema_class->supclasses = NULL;
		schema_class->subclasses_direct = NULL;
		schema_class->subclasses = NULL;
		schema_class->posssuperiors = NULL;
	}
}

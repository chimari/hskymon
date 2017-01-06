/* libVOTable - VOTABLE parser 
 Copyright (C) 2005  Malapert Jean-christophe - TERAPIX - IAP/CNRS

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 any later version.
     
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
     
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 Please use the following e_mail for questions, feedback and bug fixes <malapert@iap.fr>

 */

//-------------------------------------------------------------
//- Project: libVotable
//- Filename: votable.c
//-------------------------------------------------------------
//- Author: J-C MALAPERT
//- Creation date: 25/11/2004
//- Last modification date: 02/12/2004
//-------------------------------------------------------------
//- Comments:
//- 25/11/2004    Creation
//- 02/12/2004    - fix bug about memory free
//                - init list in Init_VO_Parser
//                - memory allocation for column in 
//                  Extract_VO_Header
//-------------------------------------------------------------

#ifndef _VOTABLE_H_
#define _VOTABLE_H_
#include <stdio.h>  /* using printf         */
#include <stdlib.h> /* using malloc and free */
#include <libxml/xmlmemory.h>
#include <libxml/xmlreader.h>
#include "config.h"

/* --------------------------------- Macros ---------------------------- */

#define DEBUG(x) printf(x)
/** \def RETURN_OK
 *  \brief One of Codes returned by functions
 *
 * is returned when the function successed
 */
#define RETURN_OK 0

/** \def RETURN_ERROR
 *  \brief One of Codes returned by functions
 *
 * is returned when the function failed
 */
#define RETURN_ERROR 1

/** \def EXIT_MEMORY
 *  \brief One of Codes returned by functions
 * 
 * is return when there is not enough memory
 */
#define EXIT_MEMORY 2

/** \def RETURN_READING
 *  \brief Can't open file
 *
 *  exit with EXIT_MEMORY value when xmlReaderForFile function failed
 */
#define EXIT_READING 3

/** \def QMALLOC
 *  \brief Macro for memory allocation
 *
 */
#ifdef USE_WIN32
#define QMALLOC(ptr, typ, nel) \
  {if (!(ptr = (typ *)malloc((size_t)(nel)*sizeof(typ))))      \
      exit(1);;}
#else
#define QMALLOC(ptr, typ, nel) \
  {if (!(ptr = (typ *)malloc((size_t)(nel)*sizeof(typ))))      \
                  error(EXIT_MEMORY, "Not enough memory for ", \
                        #ptr " (" #nel " elements) !");;}
#endif


/* ------------------------- Structure definition -----------------------*/

/** \struct _list_field
 * \brief Value and all attributes allowed for FIELD tag in VOTABLE
 * 
 * Linking list containing values and all attributes allowed for FIELD tag
 */
typedef struct _list_field list_field;

struct _list_field {
  xmlChar *ID;        /**< \brief  ID used : implied */
  xmlChar *unit;      /**< \brief  unit used : implied */
  xmlChar *datatype;  /**< \brief  datatype used  : implied
		       *\n
		       * among  : boolean, bit, unsignedByte, short ,int, long, char
		       * unicodeChar, float, double, floatComplex or doubleComplex 
		       */
  xmlChar *precision; /**< \brief  value precision used : implied */
  xmlChar *width;     /**< \brief  value width used : implied */
  xmlChar *ref;       /**< \brief  ref attribute used : implied */
  xmlChar *name;      /**< \brief  name attribute used : implied */
  xmlChar *ucd;       /**< \brief  unified content type attribute used : implied */
  xmlChar *arraysize; /**< \brief  arraysize used : implied */
  xmlChar *type;      /**< \brief  type used among : implied 
		       *\n
		       * among : hidden, no_query or trigger */
  int position;      /**< \brief position 
		      *\n
		      * position of the FIELD element in respect to others ones
		      */
  list_field *next; /**< \brief address of the next element of the list */
};


/** \struct _list_tabledata
 * \brief Value and all attributes allowed for TD tag in VOTABLE
 * 
 * Linking list containing all attributes allowed and values for TD tag
 */
typedef struct _list_tabledata list_tabledata;

struct _list_tabledata {
  xmlChar *value;     /**< \brief TD tag value used*/
  xmlChar *ref;       /**< \brief TABLEDATA attribute used */
  int colomn;         /**< \brief column to parse */
  list_tabledata *next; /**< \brief address of the next element of the list */
};


/** \struct _list_table
 * \brief Value and all attributes allowed for TABLE tag in VOTABLE
 * 
 * Linking list containing all attributes allowed and values for TABLE tag
 */
typedef struct _list_table list_table;

struct _list_table {
  xmlChar *ID;         /**< \brief ID attribute used : implied */
  xmlChar *name;       /**< \brief name attribute used : implied */
  xmlChar *ref;        /**< \brief ref attribute used : implied */
};


/** \struct _VOTable
 * \brief structure containing all linking list relative to VOTABLE
 * 
 */
typedef struct _VOTable VOTable;

struct _VOTable {
  list_table *table; /**< \brief linking list for TABLE tag */
  list_tabledata *tabledata; /**< \brief linking list for TABLEDATA tag */
  list_field *field; /**< \brief linking list for FIELD tag */
};

/* -------------------------- Protos --------------------------*/

/*
  PURPOSE : insert attributes of FIELD elements in a list
  INPUT   : Pointer on reader | pointer on header | FIELD positon in respect to the others ones
  OUTPUT  : list
  AUTHORS : J-C Malapert
  VERSION : 25/11/2004
*/
static list_field *insert_field(xmlTextReaderPtr reader, 
				  list_field *list, 
				  int position);


/*
  PURPOSE : insert values of TABLEDATA in a list for a particular FIELD
  INPUT   : Pointer on reader | pointer on tabledata FIELD positon in respect to the others ones
  OUTPUT  : list
  AUTHORS : J-C Malapert
  VERSION : 25/11/2004
*/
static list_tabledata *insert_tabledata(xmlTextReaderPtr reader, 
					list_tabledata *list, 
					int position);


/*
  PURPOSE : insert values of TABLE attributes in a structure
  INPUT   : Pointer on reader
  OUTPUT  : list
  AUTHORS : J-C Malapert
  VERSION : 25/11/2004
*/
static list_table *insert_table(xmlTextReaderPtr reader);



/** \fn  static void Free_VO_Fields(list_field *vlist_field,int **column)
 * \brief Free field linking list and column\n
 * PURPOSE : Free field linking list and column
 *
 * \param vlist_field Pointer on field linking list 
 * \param column Array representing the columns position to extract in TABLEDATA party
 */ 
static void Free_VO_Fields(list_field *vlist_field, 
			   int **column);



/** \fn static void Free_VO_Tabledata(list_tabledata *vlist_tabledata)
 * \brief Free tabledata linking list\n
 * PURPOSE : Free tabledata linking list
 *
 * \param vlist_tabledata Pointer on field linking list 
 */ 
static void Free_VO_Tabledata(list_tabledata *vlist_tabledata);



/** \fn  sattic void Free_VO_Table(list_table *vlist_table)
 * \brief Free table linking list\n
 * PURPOSE : Free table linking list
 *
 * \param vlist_table Pointer on field linking list 
 */ 
static void Free_VO_Table(list_table *vlist_table);



/** \defgroup Manage_memory */
/*@{*/
/** \fn xmlTextReaderPtr Init_VO_Parser(const char *filename,VOTable *votablePtr)
 * \brief Parser initialization\n
 * PURPOSE : Initializing of VOTable struct and reader
 *
 * \param filename VOTABLE filename
 * \param votablePtr Pointer on VOTable structure
 *
 * \exception EXIT_READING exit program
 * 
 * \return xmlTextReaderPtr Pointer on xmlTextReader
 */ 
xmlTextReaderPtr Init_VO_Parser(const char *filename,
				VOTable *votablePtr);



/** \fn   Free_VO_Parser(xmlTextReaderPtr reader,VOTable *votablePtr,int **column)
 * \brief VO_Parser memory free\n
 * PURPOSE : Free VOTable struct, column and reader
 *
 * \param reader Pointer on xmlTextReader
 * \param votablePtr Pointer on VOTable structure
 * \param column Array representing the columns position to extract embedded in TABLEDATA 
 *
 * \return RETURN_OK free successed
 */ 
int Free_VO_Parser(xmlTextReaderPtr reader,
		   VOTable *votablePtr,
		   int **column);
/*@}*/




/** \defgroup Extract_Elements */
/*@{*/
/** \fn void Extract_VO_Fields (xmlTextReaderPtr reader,VOTable *votablePtr,int *nbFields,int **columns)
 * \brief Procedure to extract FIELD attribute\n
 * PURPOSE : Free field linking list if needed, extracts FIELD attributes 
 * from VOTABLE file and stores them in VOTable structure
 *
 * \param reader Pointer on xmlTextReader\n
 * \param votablePtr Pointer on VOTable structure
 * \param nbFields FIELD tag occurence embedded in one TABLE element
 * \param columns Array representing the columns position to extract in TABLEDATA
 *
 * \return nbFields : Field tag occurence in one TABLE tag\n
 * columns : Array allocated to nbFields size and zero initialized 
 */ 
void Extract_VO_Fields ( xmlTextReaderPtr reader,
			 VOTable *votablePtr, 
			 int *nbFields, 
			 int **columns);



/** \fn void Extract_VO_TableData (xmlTextReaderPtr reader,VOTable *votablePtr,int nbcolumns,int *columns)
 * \brief Procedure to extract TD tag attributes and values\n
 * PURPOSE : Free tabledata linking list if needed, Extracts TD attributes and values 
 * from VOTABLE file and stores them in VOTable structure
 *
 * \param reader Pointer on xmlTextReader
 * \param votablePtr Pointer on VOTable structure
 * \param nbcolumns Field tag occurence embedded in one TABLE element
 * \param columns Array representing the columns position to extract in TABLEDATA tag
 * \return Number of records 
 */ 
int Extract_VO_TableData (xmlTextReaderPtr reader, 
			  VOTable *votablePtr,  
			  int nbcolumns, 
			  int *columns);



/** \fn void Extract_Att_VO_Table (xmlTextReaderPtr reader,VOTable *votablePtr)
 * \brief Procedure extracting TABLE tag attributes\n
 * PURPOSE : Free table linking list if needed, extracts TABLE attributes from VOTABLE file and stores them in VOTable structure
 *
 * \param reader Pointer on xmlTextReader
 * \param votablePtr Pointer on VOTable structure
 */ 
 
void Extract_Att_VO_Table(xmlTextReaderPtr reader, 
			  VOTable *votablePtr);
/*@}*/




/** \defgroup Move_to_next_element */
/*@{*/
/**\fn void Move_to_Next_VO_Fields (xmlTextReaderPtr reader)
 * \brief Try to move to the first FIELD tag contained in the next TABLE tag\n
 * PURPOSE : Try to move to the first FIELD tag embedded in the next TABLE tag
 *
 * \param reader Pointer on xmlTextReader
 *
 * \exception RETURN_ERROR can't move to the next TABLE tag\n
 * Move reader to the end of file
 * \return RETURN_OK Move reader to the first FIELD tag of the next TABLE tag
 */ 
int Move_to_Next_VO_Fields (xmlTextReaderPtr reader);



/** \fn  int Move_to_Next_VO_Table (xmlTextReaderPtr reader)
 * \brief Try to move to the next TABLE tag\n
 * PURPOSE : Try to move to the next TABLE tag
 *
 * \param reader Pointer on xmlTextReader
 *
 * \exception RETURN_ERROR can't move to the first FIELD embedded in the next TABLE element\n
 * Move reader to the end of file
 * \return RETURN_OK Move reader to the next TABLE tag 
 */ 
int Move_to_Next_VO_Table (xmlTextReaderPtr reader);
/*@}*/



#endif
/**\mainpage
 *
 * libVoTable is a library enable to parse VOTABLE files. This library is implemented in C and based on the xmlReader class. For this, it is very useful for large VOTABLE file warranting also an important economy of memory and faster.
 *
 *
 * \todo 
 * Parse all elements and attributes given by VOTABLE
 *\n
 * Check VOTABLE validity
 *\n
 * Implementing other parsing methods
 *
 * \note
 * libVOTable is a free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2 of the licence, or any later version
 *\n
 * This program is distributed, but WITHOUT ANY WARRANTY ; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public Licence for more details.
 *\n
 * Please use the following e_mail for questions, feedback and bug fixes <malapert@iap.fr>
 */

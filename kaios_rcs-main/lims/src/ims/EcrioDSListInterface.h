/******************************************************************************

Copyright (c) 2005-2020 Ecrio, Inc. All Rights Reserved.

Provided as supplementary materials for Licensed Software.

This file contains Confidential Information of Ecrio, Inc. and its suppliers.
Certain inventions disclosed in this file may be claimed within patents owned
or patent applications filed by Ecrio or third parties. No part of this
software may be reproduced or transmitted in any form or by any means or used
to make any derivative work (such as translation, transformation or
adaptation) without express prior written consent from Ecrio. You may not mark
or brand this file with any trade name, trademarks, service marks, or product
names other than the original brand (if any) provided by Ecrio. Any use of
Ecrio's or its suppliers work, confidential information, patented inventions,
or patent-pending inventions is subject to the terms and conditions of your
written license agreement with Ecrio. All other use and disclosure is strictly
prohibited.

Ecrio reserves the right to revise this software and to make changes in
content from time to time without obligation on the part of Ecrio to provide
notification of such revision or changes.

ECRIO MAKES NO REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF
ERRORS OR THAT THE SOFTWARE IS SUITABLE FOR YOUR USE. THE SOFTWARE IS PROVIDED
ON AN "AS IS" BASIS FOR USE AT YOUR OWN RISK. ECRIO MAKES NO WARRANTIES,
TERMS OR CONDITIONS, EXPRESS OR IMPLIED,EITHER IN FACT OR BY OPERATION OF LAW,
STATUTORY OR OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.

TO THE FULL EXTENT ALLOWED BY LAW, ECRIO ALSO EXCLUDES FOR ITSELF AND ITS
SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING
NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING OUT
OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF ECRIO HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES. THIS SOFTWARE MAY BE PROVIDED ON A DOWNLOAD SITE OR
ON COMPACT DISK AND THE OTHER SOFTWARE AND DOCUMENTATION ON THE DOWNLOAD SITE OR
COMPACT DISK ARE SUBJECT TO THE LICENSE AGREEMENT ACCOMPANYING THE COMPACT DISK.

******************************************************************************/

/***********************************************************************************
                            Header File
************************************************************************************/

#ifndef _ECRIO_DS_LIST_H
#define _ECRIO_DS_LIST_H

#include "EcrioPAL.h"
#include "EcrioDSCommon.h"

/***********************************************************************
                        Error Codes
************************************************************************/

/** @def
* Error Codes.
*/
#define ECRIO_DS_DATA_NOT_FOUND    (0x05)

/***********************************************************************
                        CallBack function declaration
************************************************************************/

/**
*Callback function for destroying data
*@Param IN		void* pData - Pointer to data to be destroyed
*@Param IN		void* pCallback - Pointer to callback data
*@Return		void
*/
typedef void (*EcrioDSDestroyCallback)
(
	void *pData,
	void *pCallback
);

/**
*Callback function for comparing data
*@Param IN		void* pData1 - Pointer to data to be compared
*@Param IN		void* pData2 - Pointer to data with which data is compared
*@Return		void* pCallback - pointer to callback data
*/
typedef s_int32 (*EcrioDSCompareCallback)
(
	void *pData1,
	void *pData2,
	void *pCallback
);

/***********************************************************************
                        Function Declaration
***********************************************************************/

/**
*API for creating/Initializing List
*@Param IN		compareCallback pCompare - Pointer to function, compare given data
*@Param IN		destroyCallback pDestroy - Pointer to function, frees data
*@Param IN/OUT	void*	list - List handle
*@Return		Error Code
*/
u_int32 EcrioDSListCreate
(
	LOGHANDLE logHandle,
	EcrioDSCompareCallback pCompareProc,
	EcrioDSDestroyCallback pDestroyProc,
	void *pCallbackData,
	void **pList
);

/**
*API for deleting all nodes of list and de-intialize the list structure
*@Param IN/OUT	void*	list - List handle
*@Return		Error Code
*/
u_int32 EcrioDSListDestroy
(
	void **pList
);

/**
*API for inserting new node in list having given data
*@Param IN		void*	Data - data to be inserted
*@Param IN/OUT	void*	list - List handle
*@Return		Error Code
*/
u_int32 EcrioDSListInsertData
(
	void *pListStruct,
	void *pData
);

/**
*API for deleting node of list having matching data
*@Param IN/OUT	void*	list - List handle
*@Param IN		void*	Data - To delete the node of matching data.
*@Return		Error Code
*/
u_int32 EcrioDSListRemoveData
(
	void *pList,
	void *pData
);

/**
*API for inserting new node in list at given index
*@Param IN/OUT	void*	list - List handle
*@Param IN		u_int32 index - Position at which new node is to be inserted.
*@Return		Error Code
*/
u_int32 EcrioDSListInsertDataAt
(
	void *pList,
	u_int32 index,
	void *pData
);

/**
*API for deleting node of list from given index
*@Param IN/OUT	void*	list - List handle
*@Param IN		u_int32 index - Position from which node is to be deleted.
*@Return		Error Code
*/
u_int32 EcrioDSListRemoveDataAt
(
	void *pList,
	u_int32 index
);

/**
*API for Getting data from given Index
*@Param IN		void*	list - List handle
*@Param IN		u_int32 index - Position from which data is to be accessed.
*@Param OUT		void*	pData - Accessed Data
*@Return		Error Code
*/
u_int32 EcrioDSListGetDataAt
(
	void *pList,
	u_int32 index,
	void **pData
);

/**
*API to get size of list
*@Param IN/OUT	void*	list - List handle
*@Param OUT		u_int32* size - size of the list
*@Return		Error Code
*/
u_int32 EcrioDSListGetSize
(
	void *pList,
	u_int32 *size
);

/**
*API to search data in the list
*@Param IN/OUT	void*	list - List handle
*@Param OUT		u_int32* size - size of the list
*@Return		Error Code
*/
u_int32 EcrioDSListFindData
(
	void *pListStruct,
	void *pData,
	void **ppData
);

u_int32 EcrioDSListGetAllNodes
(
	void *pListStruct,
	void **resultsList
);

u_int32 display
(
	void *pListStruct
);
#endif	// _ECRIO_DS_LIST_H

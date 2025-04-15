/******************************************************************************

Copyright (c) 2015-2020 Ecrio, Inc. All Rights Reserved.

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

#include "EcrioDSListInternal.h"

/**************************************************************************

  Function:		EcrioDSListCreate ()

  Purpose:		Create and initialze the data structure(linked list)

  Description:	This function can be called by upper layer module and Platform Manager.

  Input:		compareCallback pCompare - Pointer to function, compare given data
                destroyCallback pDestroy - Pointer to function, frees data
                void*	list - List handle

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListCreate
(
	LOGHANDLE logHandle,
	EcrioDSCompareCallback pCompareProc,
	EcrioDSDestroyCallback pDestroyProc,
	void *pCallbackData,
	void **ppListStruct
)
{
	EcrioDSListStruct *pTempListStruct = NULL;

	// quiet	DSLIBLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (*ppListStruct != NULL)
	{
		DSLIBLOGW(logHandle, KLogTypeGeneral, "%s:%u\tList already created", __FUNCTION__, __LINE__);
	}

	pal_MemoryAllocate(sizeof(EcrioDSListStruct), (void **)&pTempListStruct);
	if (pTempListStruct == NULL)
	{
		DSLIBLOGE(logHandle, KLogTypeGeneral, "%s:%u\tfailed to allocate memory for List structure",
			__FUNCTION__, __LINE__);

		return ECRIO_DS_INSUFFICIENT_MEMORY;
	}

	pTempListStruct->pCompareFunc = pCompareProc;
	pTempListStruct->pDestroyFunc = pDestroyProc;
	pTempListStruct->pHead = NULL;
	pTempListStruct->pTail = NULL;
	pTempListStruct->size = 0;
	pTempListStruct->pCallbackData = pCallbackData;
	pTempListStruct->pLogHandle = logHandle;
	*ppListStruct = pTempListStruct;						// Pointed to allocated memory

	// quiet	DSLIBLOGI(logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_DS_NO_ERROR;
}

/**************************************************************************

  Function:		EcrioDSListInsertData ()

  Purpose:		Insert data in list

  Description:	This function can be called by upper layer module and Platform Manager.

  Input:		void*	Data - data to be inserted
  IN/OUT		void*	pListStruct - List handle

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListInsertData
(
	void *pListStruct,
	void *pData
)
{
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode **pNodeList = NULL;
	ListNode *pTemp = NULL;
	LOGHANDLE pLogHandle = NULL;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;
	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pLogHandle = pTempListStruct->pLogHandle;

	// quiet	DSLIBLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(ListNode), (void **)&pTemp);
	if (pTemp == NULL)
	{
		DSLIBLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tfailed to allocate memory for new node in list",
			__FUNCTION__, __LINE__);

		return ECRIO_DS_INSUFFICIENT_MEMORY;
	}

	pTemp->pData = pData;
	pTemp->pNext = NULL;
	pNodeList = (ListNode **)&(pTempListStruct->pHead);

	if (*pNodeList == NULL)
	{
		pTempListStruct->pHead = pTemp;
	}
	else
	{
		(pTempListStruct->pTail)->pNext = pTemp;
	}

	pTempListStruct->pTail = pTemp;
	pTempListStruct->size++;

	// quiet	DSLIBLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_DS_NO_ERROR;
}

/**************************************************************************

  Function:		EcrioDSListInsertDataAt ()

  Purpose:		Insert data in list at given position

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	pListStruct - List handle

  Input			u_int32 index - Position at which new node is to be inserted.
                void*   pData - Data to be inserted at given position

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListInsertDataAt
(
	void *pListStruct,
	u_int32 index,
	void *pData
)
{
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode **pNodeList = NULL;
	ListNode *pTemp = NULL;
	u_int32 nCounter = 0;
	LOGHANDLE pLogHandle = NULL;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;
	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pLogHandle = pTempListStruct->pLogHandle;

	// quiet	DSLIBLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (index <= 0 || index - 1 > pTempListStruct->size)
	{
		DSLIBLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tfailed, insufficient data",
			__FUNCTION__, __LINE__);

		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pal_MemoryAllocate(sizeof(ListNode), (void **)&pTemp);
	if (pTemp == NULL)
	{
		DSLIBLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tfailed to allocate memory for List structure",
			__FUNCTION__, __LINE__);

		return ECRIO_DS_INSUFFICIENT_MEMORY;
	}

	pTemp->pData = pData;
	pTemp->pNext = NULL;
	pNodeList = (ListNode **)&(pTempListStruct->pHead);

	if (index == 1)
	{
		pTemp->pNext = pTempListStruct->pHead;
		pTempListStruct->pHead = pTemp;
	}
	else
	{
		while (nCounter < index - 2 && *pNodeList != NULL)
		{
			nCounter++;
			pNodeList = &(*pNodeList)->pNext;
		}

		if (pNodeList == NULL)
		{
			*pNodeList = pTemp;
			pTempListStruct->pTail = pTemp;
		}
		else
		{
			pTemp->pNext = (*pNodeList)->pNext;
			(*pNodeList)->pNext = pTemp;
		}
	}

	pTempListStruct->size++;

	// quiet	DSLIBLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_DS_NO_ERROR;
}

/**************************************************************************

  Function:		EcrioDSListGetDataAt ()

  Purpose:		Retrieve data from given position

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:  void*	list - List handle
  Input:		 u_int32 index - Position from which data is to be accessed.
                 void*   data - Retrieved data

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListGetDataAt
(
	void *pListStruct,
	u_int32 index,
	void **ppData
)
{
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode **pNodeList = NULL;
	u_int32 nCounter = 0;
	LOGHANDLE pLogHandle = NULL;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;

	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pLogHandle = pTempListStruct->pLogHandle;

	// quiet	DSLIBLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (index <= 0 || index > pTempListStruct->size)
	{
		DSLIBLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tfailed, index is out of Bounds",
			__FUNCTION__, __LINE__);

		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pNodeList = (ListNode **)&pTempListStruct->pHead;

	while (nCounter < index - 1 && *pNodeList != NULL)
	{
		nCounter++;
		pNodeList = &(*pNodeList)->pNext;
	}

	if (*pNodeList == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	*ppData = (*pNodeList)->pData;

	// quiet	DSLIBLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_DS_NO_ERROR;
}

/**************************************************************************

  Function:		EcrioDSListGetSize ()

  Purpose:		Get size of the list

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle
  Output:		u_int32* nsize - size of the list

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListGetSize
(
	void *pListStruct,
	u_int32 *size
)
{
	EcrioDSListStruct *pTempListStruct = NULL;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;

	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	*size = pTempListStruct->size;
	return ECRIO_DS_NO_ERROR;
}

/**************************************************************************

  Function:		EcrioDSListDestroy ()

  Purpose:		Empty the list structure

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListDestroy
(
	void **pListStruct
)
{
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode *pTempList = NULL;

	// LOGHANDLE pLogHandle = NULL;

	pTempListStruct = (EcrioDSListStruct *)(*pListStruct);

	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	// pLogHandle = pTempListStruct->pLogHandle;

	pTempList = pTempListStruct->pHead;

	while (pTempListStruct->size > 0 && pTempList != NULL)
	{
		EcrioDSListRemoveDataAt(*pListStruct, 1);	// Remove head of the list
		// Decrement the size
	}

	pTempListStruct->pCompareFunc = NULL;
	pTempListStruct->pDestroyFunc = NULL;
	pTempListStruct->pHead = NULL;
	pTempListStruct->pTail = NULL;
	pTempListStruct->size = 0;
	pTempListStruct->pCallbackData = NULL;
	pTempListStruct->pLogHandle = NULL;

	pal_MemoryFree((void **)&pTempListStruct);
	*pListStruct = NULL;
	return ECRIO_DS_NO_ERROR;
}

/**************************************************************************

  Function:		EcrioDSListRemoveData ()

  Purpose:		To remove the data from the list

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle
                void *pData - Pointer to data to be removed

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListRemoveData
(
	void *pListStruct,
	void *pData
)
{
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode *pTempList = NULL;
	ListNode *pPrevList = NULL;
	ListNode *pTempNode = NULL;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;

	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	if (pTempListStruct->pHead == NULL ||
		pTempListStruct->pCompareFunc == NULL)
	{
		DSLIBLOGE(pTempListStruct->pLogHandle, KLogTypeGeneral, "%s:%u\tinsuffiecient data in list Structure",
			__FUNCTION__, __LINE__);

		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pTempList = pTempListStruct->pHead;
	pPrevList = pTempListStruct->pHead;

	if (pTempListStruct->pCompareFunc(pTempList->pData, pData,
		pTempListStruct->pCallbackData) == 0)
	{
		pTempNode = pTempList;
		pTempListStruct->pHead = pTempList->pNext;
		// Sumit code changes
		pPrevList = pTempListStruct->pHead;
	}
	else
	{
		/* We have already covered the first Node in the above 'if'.
		 * Here, we can directly start from the second Node.
		 */
		pPrevList = pTempList;
		pTempList = pTempList->pNext;

		while (pTempList != NULL)
		{
			if (pTempListStruct->pCompareFunc(pTempList->pData, pData,
				pTempListStruct->pCallbackData) == 0)
			{
				pTempNode = pTempList;
				pPrevList->pNext = pTempList->pNext;
				break;
			}

			pPrevList = pTempList;
			pTempList = pTempList->pNext;
		}
	}

	if (pTempNode != NULL)
	{
		if (pTempListStruct->pDestroyFunc != NULL)
		{
			pTempListStruct->pDestroyFunc(pTempNode->pData, pTempListStruct->pCallbackData);
		}

		pTempNode->pData = NULL;// To Do
		pal_MemoryFree((void **)&pTempNode);
		if ((pPrevList == NULL) || (pPrevList->pNext == NULL))
		{
			pTempListStruct->pTail = pPrevList;
		}

		pTempListStruct->size--;
		return ECRIO_DS_NO_ERROR;
	}

	return ECRIO_DS_DATA_NOT_FOUND;
}

/**************************************************************************

  Function:		EcrioDSListRemoveDataAt ()

  Purpose:		To remove the data at the specified index from the list

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle
                u_int32 index - Index at which data is to be removed

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListRemoveDataAt
(
	void *pListStruct,
	u_int32 index
)
{
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode *pTempList = NULL;
	ListNode *pPrevList = NULL;
	ListNode *pTempNode = NULL;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;

	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	if (pTempListStruct->pHead == NULL ||
		pTempListStruct->pCompareFunc == NULL || pTempListStruct->pDestroyFunc == NULL ||
		index <= 0 || index > pTempListStruct->size)
	{
		DSLIBLOGE(pTempListStruct->pLogHandle, KLogTypeGeneral, "%s:%u\tinsufficient data",
			__FUNCTION__, __LINE__);

		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pTempList = pTempListStruct->pHead;
	pPrevList = pTempList;
	if (index == 1)
	{
		pTempNode = pTempList;
		pTempListStruct->pHead = pTempList->pNext;
		/* Update pPrevList here. It will be referenced below
		 * while updating the TAIL Node.
		 */
		pPrevList = pTempListStruct->pHead;
	}
	else
	{
		/* We have already covered the first Node in the above 'if'.
		 * Here, we can directly start from the second Node.
		 * So, we deduct 2 from the index before we begin.
		 */
		pPrevList = pTempList;
		pTempList = pTempList->pNext;
		index -= 2;

		while (index && pTempList)
		{
			pPrevList = pTempList;
			pTempList = pTempList->pNext;
			--index;
		}

		pTempNode = pTempList;
		pPrevList->pNext = pTempList->pNext;
	}

	/* Update the TAIL Node here.
	 * Case 1: pPrevList will be NULL only when there is a single Node
	 *         in the List and it is the one being Deleted, so we set
	 *         the TAIL Node = pPrevList = NULL
	 * Case 2: pPrevList->Next == NULL indicates pPrevList is the Last
	 *         Node, so set the TAIL Node to pPrevList
	 */
	if ((pPrevList == NULL) || (pPrevList->pNext == NULL))
	{
		pTempListStruct->pTail = pPrevList;
	}

	if (pTempListStruct->pDestroyFunc != NULL)
	{
		pTempListStruct->pDestroyFunc(pTempNode->pData, pTempListStruct->pCallbackData);
	}

	pal_MemoryFree((void **)&pTempNode);

	pTempListStruct->size--;
	return ECRIO_DS_NO_ERROR;
}

/**************************************************************************

  Function:		EcrioDSListPush ()

  Purpose:		Push element in stack

  Description:	This function can be called by upper layer module and Platform Manager.

  Input:		void*	Data - data to be inserted
  Input/Output:	void*	list - List handle

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListPush
(
	void *pData,
	void *pListStruct
)
{
	u_int32 error = ECRIO_DS_NO_ERROR;

	error = EcrioDSListInsertDataAt(pListStruct, 0, pData);		// insert at begining of the list for stack
	return error;
}

/**************************************************************************

  Function:		EcrioDSListPop ()

  Purpose:		Pop element from stack

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListPop
(
	void *pList
)
{
	u_int32 error = ECRIO_DS_NO_ERROR;

	error = EcrioDSListRemoveDataAt(pList, 0);			// Remove data from begining
	return error;
}

/**************************************************************************

  Function:		EcrioDSListGetDataFromStack ()

  Purpose:		Get data from stack (Topmost element)

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle

  Output:		void*	Data - Retrieved data

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListGetDataFromStack
(
	void *pListStruct,
	void *pData
)
{
	u_int32 error = ECRIO_DS_NO_ERROR;

	error = EcrioDSListGetDataAt(pListStruct, 0, pData);
	return error;
}

/**************************************************************************

  Function:		EcrioDSEnqueue ()

  Purpose:		Insert data into queue

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle
  Input:		void*	Data - data to be inserted

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSEnqueue
(
	void *pListStruct,
	void *pData
)
{
	u_int32 error = ECRIO_DS_NO_ERROR;
	EcrioDSListStruct *pTempListStruct = NULL;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;
	error = EcrioDSListInsertDataAt(pListStruct, pTempListStruct->size, pData);	// insert data at the end of the list for queue
	return error;
}

/**************************************************************************

  Function:		EcrioDSDequeue ()

  Purpose:		Delete data from queue

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle
  Input:		void*	Data - data to be inserted

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListDequeue
(
	void *pList
)
{
	u_int32 error = ECRIO_DS_NO_ERROR;

	error = EcrioDSListRemoveDataAt(pList, 0);			// Remove data from begining
	return error;
}

/**************************************************************************

  Function:		EcrioDSListGetDataFromQueue ()

  Purpose:		Get Data from queue (Front element)

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:	void*	list - List handle
  Output:		void*	Data - retrieved data

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListGetDataFromQueue
(
	void *pListStruct,
	void *pData
)
{
	u_int32 error = ECRIO_DS_NO_ERROR;

	error = EcrioDSListGetDataAt(pListStruct, 0, pData);	// Get front data of queue
	return error;
}

/**************************************************************************

  Function:		EcrioDSListFindData ()

  Purpose:		Find data in the list

  Description:	This function can be called by upper layer module and Platform Manager.

  Input/Output:  void*	list - List handle
  Input:		 void* pKey -
                 void**  ppData - Retrieved data

  Output:		None

  Returns:		error code

**************************************************************************/
u_int32 EcrioDSListFindData
(
	void *pListStruct,
	void *pData,
	void **ppData
)
{
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode **pNodeList = NULL;
	u_int32 nCounter = 0;
	u_int32 error = ECRIO_DS_NO_ERROR;
	BoolEnum foundFlag = Enum_FALSE;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;

	if (pTempListStruct == NULL)
	{
		error = ECRIO_DS_INSUFFICIENT_DATA;
		return error;
	}

	// quiet	DSLIBLOGI(pTempListStruct->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	// quiet	DSLIBLOGI(pTempListStruct->pLogHandle, KLogTypeFuncParams, "%s:%u\tpListStruct = %x, pData = %x, pData = %x",
	// quiet		__FUNCTION__, __LINE__, pListStruct, pData, ppData);

	if (pTempListStruct->size == 0)
	{
		// empty list
		error = ECRIO_DS_DATA_NOT_FOUND;
		goto Error_Level_01;
	}

	pNodeList = (ListNode **)&pTempListStruct->pHead;
	if (pNodeList == NULL || *pNodeList == NULL)
	{
		DSLIBLOGE(pTempListStruct->pLogHandle, KLogTypeGeneral, "%s:%u\tinsufficient data in List structure",
			__FUNCTION__, __LINE__);

		error = ECRIO_DS_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	while (nCounter < pTempListStruct->size && *pNodeList != NULL)
	{
		if (pTempListStruct->pCompareFunc((*pNodeList)->pData, pData,
			pTempListStruct->pCallbackData) == 0)
		{
			foundFlag = Enum_TRUE;
			break;
		}

		nCounter++;
		pNodeList = &(*pNodeList)->pNext;
	}

	if (foundFlag == Enum_FALSE)
	{
		error = ECRIO_DS_DATA_NOT_FOUND;
		goto Error_Level_01;
	}
	else
	{
		// data found
		*ppData = (*pNodeList)->pData;
	}

Error_Level_01:

	// quiet	DSLIBLOGI(pTempListStruct->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
	// quiet		__FUNCTION__, __LINE__, error);

	return error;
}

#if 0
u_int32 EcrioDSListGetAllNodes
(
	void *pListStruct,
	void **resultsList
)
{
	u_int32 error = ECRIO_DS_NO_ERROR;
	EcrioDSListStruct *pTempListStruct = NULL;
	ListNode *pTempList = NULL;
	u_int32 count = 0;

	pTempListStruct = (EcrioDSListStruct *)pListStruct;

	if (pTempListStruct == NULL)
	{
		return ECRIO_DS_INSUFFICIENT_DATA;
	}

	pTempList = pTempListStruct->pHead;

	while (pTempList != NULL)
	{
		pal_MemoryAllocate(sizeof(void **), &resultsList[count]);
		if (resultsList[count] == NULL)
		{
			DSLIBLOGE(pTempListStruct->pLogHandle, KLogTypeGeneral, "%s:%u\tfailed to allocate memory for List structure",
				__FUNCTION__, __LINE__);

			return ECRIO_DS_INSUFFICIENT_MEMORY;
		}

		*(resultsList + count) = pTempList->pData;
		pTempList = pTempList->pNext;
		count++;
	}

	return error;
}
#endif

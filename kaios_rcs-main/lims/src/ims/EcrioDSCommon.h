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

#ifndef _ECRIO_DATA_STRUCTURE_COMMON_INTERFACE_H_
#define  _ECRIO_DATA_STRUCTURE_COMMON_INTERFACE_H_

/** @def
* Error Codes.
*/

#define ECRIO_DS_NO_ERROR				(0x00)
#define ECRIO_DS_UNKNOWN_ERROR			(0x01)
#define ECRIO_DS_INVALID_HANDLE			(0x02)
#define ECRIO_DS_INSUFFICIENT_MEMORY	(0x03)
#define ECRIO_DS_INSUFFICIENT_DATA		(0x04)

#ifdef ENABLE_LOG
#define DSLIBLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentUndefined, (b), (c),##__VA_ARGS__);
#define DSLIBLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentUndefined, (b), (c),##__VA_ARGS__);
#define DSLIBLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentUndefined, (b), (c),##__VA_ARGS__);
#define DSLIBLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentUndefined, (b), (c),##__VA_ARGS__);
#define DSLIBLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentUndefined, (b), (c),##__VA_ARGS__);
#else
#define DSLIBLOGV(a, b, c, ...)		if (a != NULL) {;}
#define DSLIBLOGI(a, b, c, ...)		if (a != NULL) {;}
#define DSLIBLOGD(a, b, c, ...)		if (a != NULL) {;}
#define DSLIBLOGW(a, b, c, ...)		if (a != NULL) {;}
#define DSLIBLOGE(a, b, c, ...)		if (a != NULL) {;}
#endif

#endif	// _ECRIO_DATA_STRUCTURE_COMMON_INTERFACE_H_

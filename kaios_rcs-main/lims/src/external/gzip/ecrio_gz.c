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

#include "ecrio_gz.h"
#include "zutil.h"
//#include <io.h>

void* ecrio_set
(
	void* fd,
	char** ppbuf,
	int* pbuflen
)
{
	ecrio_fd *fd_temp = NULL;

	if(fd == NULL)
	{
		fd_temp = calloc(1,sizeof(ecrio_fd));
		if( ppbuf != NULL && *ppbuf != NULL && pbuflen != NULL && *pbuflen != 0 )
		{
			fd_temp->original_buf = calloc(*pbuflen,sizeof(char));
			zmemcpy( fd_temp->original_buf, *ppbuf , *pbuflen );
			fd_temp->buflen		  = *pbuflen;
			fd_temp->current_buf_len = 0;
			fd_temp->original_buf_len = fd_temp->buflen;
		}
		else
		{
			fd_temp->original_buf = calloc(ECRIO_MEM_CHUNK,sizeof(char));
			fd_temp->buflen		  = ECRIO_MEM_CHUNK;
			fd_temp->current_buf_len = 0;
			fd_temp->original_buf_len = 0;
		}
		fd_temp->pprefbuf = ppbuf;
		fd_temp->preflen = (unsigned int*)pbuflen;
		fd_temp->current_buf  = fd_temp->original_buf;		
	}
	else
        fd_temp = (ecrio_fd*)fd;

	return (void*)fd_temp;
}

long ecrio_write
(
	void* e_fd,
	char* buf,
	int buflen
)
{
	ecrio_fd *fd_temp = NULL;
//	int i = 0;

	if(!e_fd)
		return -1;

	fd_temp = (ecrio_fd*)(e_fd);

	if( buflen + fd_temp->current_buf_len > fd_temp->buflen )
	{
		char* pnewbuf = calloc( (buflen + fd_temp->current_buf_len), sizeof(char) );
		if( fd_temp->original_buf != NULL )
		{
			zmemcpy( pnewbuf , fd_temp->original_buf , fd_temp->original_buf_len );
			free( fd_temp->original_buf );
		}
		fd_temp->original_buf = pnewbuf;
		fd_temp->current_buf = fd_temp->original_buf + fd_temp->current_buf_len;
		fd_temp->buflen = buflen + fd_temp->current_buf_len;
	}
	zmemcpy(fd_temp->current_buf,buf,buflen);
	

    fd_temp->current_buf_len =  fd_temp->current_buf_len + buflen;
	fd_temp->current_buf =  fd_temp->current_buf + buflen;

	if( fd_temp->current_buf_len > fd_temp->original_buf_len )
		fd_temp->original_buf_len = fd_temp->current_buf_len ;
	return buflen;
}

long ecrio_read
(
	void* e_fd,
	char* buf,
	int buflen
)
{
	ecrio_fd *fd_temp = NULL;
//	int i = 0;

	if(!e_fd)
		return -1;

	fd_temp = (ecrio_fd*)(e_fd);

	if( buflen + fd_temp->current_buf_len > fd_temp->original_buf_len )
	{
		buflen = fd_temp->original_buf_len - fd_temp->current_buf_len;
	}
	zmemcpy(buf,fd_temp->current_buf,buflen);

	fd_temp->current_buf += buflen;
    fd_temp->current_buf_len += buflen;

	return buflen;
}

long ecrio_lseek
(
	void*  e_fd,
	long offset,
	int origin
)
{
	ecrio_fd *fd_temp = NULL;
//	char* t_buf = NULL;
//	int cnt = 0;

	if(!e_fd)
		return -1;
	fd_temp = (ecrio_fd*)e_fd;

	switch(origin)
	{
		case SEEK_SET:
		{
			fd_temp->current_buf = fd_temp->original_buf + offset;
			fd_temp->current_buf_len = offset;
		}
		break;
		case SEEK_CUR:
		{
			fd_temp->current_buf += offset;
			fd_temp->current_buf_len += offset;
		}
		break;
		case SEEK_END:
		{
			fd_temp->current_buf = fd_temp->original_buf + fd_temp->original_buf_len;
			fd_temp->current_buf_len = fd_temp->original_buf_len;
		}
	}
	return 0;
}


void ecrio_reset
(
	void*  e_fd
)
{
	ecrio_fd *fd_temp = NULL;

	fd_temp = (ecrio_fd*)e_fd;
	
	if(fd_temp)
	{
		fd_temp->buflen = 0;
		fd_temp->current_buf = NULL;
		if( fd_temp->original_buf != NULL )
		{
			if( fd_temp->pprefbuf != NULL )
			{
				if( *fd_temp->pprefbuf != NULL )
					free( *fd_temp->pprefbuf );
				*fd_temp->pprefbuf = calloc( fd_temp->original_buf_len, sizeof(char) );
				zmemcpy( *fd_temp->pprefbuf , fd_temp->original_buf, fd_temp->original_buf_len );
				*fd_temp->preflen = fd_temp->original_buf_len;
			}
			free(fd_temp->original_buf);
		}
		fd_temp->original_buf = NULL;
		fd_temp->start = NULL;
		free(fd_temp);
	}
}

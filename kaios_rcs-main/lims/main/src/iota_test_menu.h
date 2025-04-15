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

/**
 * @file iota_test_menu.h
 * @brief base menu class.
 */

#ifndef __IOTA_TEST_MENU_H__
#define __IOTA_TEST_MENU_H__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

/* The Menu class is the menuing base class. You can modify it. But usually just create
your own menus and sub menus. The biggest special ability is when exiting from any
sub menu, all parent levels are exited as well. */
class Menu
{
public:
static const int CONTINUE = 0;
static const int RETURN = 1;
static const int EXIT = -1;

Menu
	() {}
virtual ~Menu
	() {}

// Each menu needs to handle its own menu selections.
virtual int handler
(
	int value,
	string valString
) = 0;

// Default input handler. Can be replaced by each menu.
virtual int input
	()
{
	string myString;
	int value = 0;		// 0 is reserved to imply non-integer value.

	cout << endl << "Command: ";

	getline(cin, myString);

	stringstream(myString) >> value;

	return handler(value, myString);
}

// Default print handler to show the menu. Can be replaced if a menu has dynamic content.
virtual void print
	()
{
	cout << MenuText << endl;
}

int exec
	()
{
	// Show the current menu.
	print();

	int value;

	// Loop until exiting.
	while ((value = input()) == CONTINUE) {}

	if (value == EXIT)
	{
		// Run this menu's exit code.
		exit();
	}
	else if (value == RETURN)
	{
		// Reset the return value so we only go up one level.
		value = CONTINUE;
	}

	return value;
}

// Each menu *may* have its own exit/cleanup code.
virtual void exit
	() {}

private:

protected:
string MenuText;		// Each menu should set its own menu for printing.
};

#endif	// __IOTA_TEST_MENU_H__

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
 * @file iota_test_menu.cpp
 * @brief Entry point for the iota Main Test App.
 */

#include "EcrioPAL.h"
#include "lims.h"

#include "iota_test_menu.h"
#include "iota_test.h"
#include <fstream>
using namespace std;
// Define the global state here since it is initialized here.
iotaTestStateStruct iotaState;
#ifdef ENABLE_RCS
FileTransfer ft;

class RCSMenu : public Menu
{
	private: int iAuto;
	public: RCSMenu()
	{
		pal_MutexLock(iotaState.mutexHandle);
		destination.assign(iotaState.calleeNumber);
		message.assign(iotaState.message);
		//Msg.stBot.Json = NULL;
		this->iAuto = 0;
		pal_MutexUnlock(iotaState.mutexHandle);
	}

	public: RCSMenu(int iAuto)
	{
		pal_MutexLock(iotaState.mutexHandle);
		destination.assign(iotaState.calleeNumber);
		message.assign(iotaState.message);
		this->iAuto = iAuto;
		pal_MutexUnlock(iotaState.mutexHandle);
	}



	// Override the print function since we have a lot of dynamic content.
	void print()
	{
		if (!this->iAuto)
		{
			cout << "\nRCS Menu:\n";
			cout << "1. Set Destination MDN (" + destination + ")\n";
			cout << "----------------------\n";
			cout << "2. Send Pager Mode Message\n";
			cout << "3. Send Pager Mode IMDN Message\n";
			cout << "4. Send Text Message\n";
			cout << "5. Send session mode IMDN Message\n";
			cout << "6. Send IsComposing\n";
			cout << "7. Send File over HTTP\n";
			cout << "8. Send File over MSRP\n";
			cout << "9. Send RichCard\n";
			cout << "r. Return\n";
			cout << "\nPress (x) to exit. Press (m) to show this menu.";
		}
		else{
			cout << "\n\n\n";
			cout << "Group Chat Auto Test\n";
			cout << "----------------------\n";
			cout << "      MO Behaviour    \n";
			cout << "----------------------\n";
			cout << "31. Create and Delete Group Chat (max 100 participants)\n";
			cout << "32. Create and Delete Group Chat (max 100 groups)\n";
			cout << "33. Add participants in Group Chat (max 100 participants in Single REFER)\n";
			cout << "34. Remove participants from Group Chat (max 100 participants in Single REFER)\n";
			cout << "35. Create and Delete Group Chat with SUBSCRIBE (max 100 participants)\n";
			cout << "36. Add and Remove participants in Group Chat (max 100 participants)\n";
			cout << "37. Add participants in Group Chat (max 100 - REFER)\n";
			cout << "38. Remove participants from a Group Chat (max 100 - REFER)\n\n";
			cout << "----------------------\n";
			cout << "      MT Behaviour    \n";
			cout << "----------------------\n";
			cout << "41. Call Accept\n";
			cout << "42. Call Reject\n";
			cout << "43. Call Unavilable\n";
			cout << "44. Call Decline\n";
			cout << "45. Reset MT Behavior.\n\n";
			cout << "----------------------\n";
			cout << "r. Return\n";
			cout << "\nPress (x) to exit. Press (m) to show this menu.";
		}
	}

	int handler
	(
		int value,
		string valString
	)
	{
		string ans("initial string");
		string menu("initial string");

		switch (value)
		{
			case 1:
			{
				cout << "Enter the Destination MDN: ";
				getline(cin, destination);
				pal_MutexLock(iotaState.mutexHandle);
				strcpy(iotaState.calleeNumber, destination.c_str());
				pal_MutexUnlock(iotaState.mutexHandle);
			}
			break;
			case 2:
			{
				cout << "Enter the Message: ";
				getline(cin, message);
				pal_MutexLock(iotaState.mutexHandle);
				strcpy(iotaState.message, message.c_str());
				pal_MutexUnlock(iotaState.mutexHandle);
				iota_test_SendStandAloneMessage(iotaState.message, Enum_FALSE, Enum_FALSE);
			}
			break;
			case 3:
			{
				//cout << "Enter the Message: ";
				//getline(cin, message);
				pal_MutexLock(iotaState.mutexHandle);
				strcpy(iotaState.message, message.c_str());
				pal_MutexUnlock(iotaState.mutexHandle);
				iota_test_SendPagerIMDNMessage(iotaState.message);
			}
			break;
			case 4:
			{
				Msg = {};
				Msg.emsgType = iotaTestMessageTypeTEXT;
				cout << "Text message: \n";
				getline(cin, message);
				Msg.stData.Message.assign(message);
				Msg.stData.uMsgLen = Msg.stData.Message.length();
				cout << "Do you want to enter Bot Suggestion?(y/n)\n";
				getline(cin, ans);
				if (ans == "y")
				{
					cout << "Bot Suggestion reading from configration file... \n";
					pal_MutexLock(iotaState.mutexHandle);
					if (iota_test_config_getTestData(iotaState.configFile, &Msg) != 0)
					{
						iota_test_printf("Can't load %s for test data \n", iotaState.configFile);
						pal_MutexUnlock(iotaState.mutexHandle);
						return 1;
					}
					else
					{
						pal_MutexUnlock(iotaState.mutexHandle);
						/*Calculate the length */
						Msg.stBot.uJsonLen = Msg.stBot.Json.length();
						cout << "Is it a SCL response?(y/n) \n";
						getline(cin, ans);
						if (ans == "y")
						{
							Msg.stBot.bisSCLResponse = true;
						}
					}
				}
				else
				{
					cout << "Bot Suggestion is not sent! \n";
				}
				iota_test_SendMessage(Msg);
			}
			break;
			case 5:
			{
				Msg = {};
//				iota_test_SendIMDNMessage(Msg);
			}
			break;
			case 6:
			{
				Msg = {};
				iota_test_SendIsComposing(Msg);
			}
			break;
			case 7:
				{
					//iota_test_SendFileOverHTTP();
				}
			break;
			case 8:
				{
					//iota_test_SendFileOverMSRP();
				}
			break;
			case 9:
				{
					//For Richcard transfer
					Msg = {};
					Msg.emsgType = iotaTestMessageTypeRICHCARD;
					cout << "Rich Card data reading from configration file... \n";
					pal_MutexLock(iotaState.mutexHandle);
					if (iota_test_config_getTestData(iotaState.configFile, &Msg) != 0)
					{
						iota_test_printf("Can't load %s for test data \n", iotaState.configFile);
						pal_MutexUnlock(iotaState.mutexHandle);
						return 1;
					}
					else
					{
						pal_MutexUnlock(iotaState.mutexHandle);
						/*Calculate the length */
						Msg.stData.uMsgLen = Msg.stData.Message.length();
					}
					Msg.stBot.Json = "";//clearing out Json SCL data that was read while reading Richcard
					cout << "Do you want to enter Bot Suggestion?(y/n)\n";
					getline(cin, ans);
					if (ans == "y")
					{
						cout << "Bot Suggestion reading from configration file... \n";
						pal_MutexLock(iotaState.mutexHandle);
						if (iota_test_config_getTestData(iotaState.configFile, &Msg) != 0)
						{
							iota_test_printf("Can't load %s for test data \n", iotaState.configFile);
							pal_MutexUnlock(iotaState.mutexHandle);
							return 1;
						}
						else
						{
							pal_MutexUnlock(iotaState.mutexHandle);
							/*Calculate the length */
							Msg.stBot.uJsonLen = Msg.stBot.Json.length();
							cout << "Is it a SCL response?(y/n) \n";
							getline(cin, ans);
							if (ans == "y")
							{
								Msg.stBot.bisSCLResponse = true;
							}
						}
					}
					else
					{
						cout << "Bot Suggestion is not sent! \n";
					}
					//iota_test_SendRichCard(Msg);
				}
			break;

			default:
			{
				menu.assign(valString, 0, 1);
				if (menu == "u")
				{
					char userAgent[128];					
					cout << "Provide User-Agent value: ";
					getline(cin, inputstr);
					strcpy(userAgent, inputstr.c_str());
					iota_test_lims_SetUserAgent((u_char*)userAgent);
				}
				else if (menu == "x")
				{
					return EXIT;
				}
				else if (menu == "m")
				{
					print();
				}
				else if (menu == "r")
				{
					return RETURN;
				}
				else
				{
					cout << "Invalid Input!" << endl;
				}
			}
		}
		return 0;
	}
	private:
	string destination;
	string message;
	string inputstr;
	bool bfread_success;
	iotaMessageStruct Msg;
};
#endif //ENABLE_RCS

class MainMenu : public Menu
{
public:
MainMenu
	()
{
	MenuText = "\nMain Menu:\n";
	MenuText += "1. Init\n";
	MenuText += "2. Register\n";
	MenuText += "3. Close Connection\n";
	MenuText += "4. Deregister\n";
	MenuText += "5. Deinit\n";
#ifdef ENABLE_RCS
	MenuText += "--------------\n";
	MenuText += "8. RCS\n";
#endif // ENABLE_RCS
	MenuText += "--------------\n";
	MenuText += "9. QA\n";
	MenuText += "10. Auto Group Chat QA\n";
	MenuText += "--------------\n";
	MenuText += "11. Register with IPSec\n";
	MenuText += "\nPress (x) to exit. Press (m) to show this menu.";
}

int handler
(
	int value,
	string valString
)
{
	string menu("initial string");

	switch (value)
	{
		case 1:
		{
			if (iotaState.limsHandle != NULL)
			{
				iota_test_printf("Using existing Lims Handle!\n");
			}
			else
			{
#ifdef ENABLE_QCMAPI
				string iccid;
				cout << "Enter the iccid: ";
				getline(cin, iccid);

				if (iota_test_init(iccid) != LIMS_NO_ERROR)
#else
				if (iota_test_init() != LIMS_NO_ERROR)
#endif
				{
					iota_test_printf("Failed.\n");
				}
				else
				{
					iota_test_printf("Initialized.\n");
				}
			}
		}
		break;

		case 2:
		{
			if (iotaState.limsHandle != NULL)
			{
				if (iota_test_register() != LIMS_NO_ERROR)
				{
					iota_test_printf("Failed.\n");
				}
				else
				{
					iota_test_printf("Register request sent.\n");
				}
			}
			else
			{
				iota_test_printf("Not initialized.\n");
			}
		}
		break;

		case 4:
		{
			if (iotaState.limsHandle != NULL)
			{
				if (iota_test_deregister() != LIMS_NO_ERROR)
				{
					iota_test_printf("Failed.\n");
				}
				else
				{
					iota_test_printf("deregister request sent.\n");
				}
			}
			else
			{
				iota_test_printf("Not initialized.\n");
			}
		}
		break;

		case 5:
		{
			if (iotaState.limsHandle != NULL)
			{
				if (iota_test_deinit() != LIMS_NO_ERROR)
				{
					iota_test_printf("Failed.\n");
				}
				else
				{
					iota_test_printf("Deinitialized Lims.\n");
				}
			}
			else
			{
				iota_test_printf("Not initialized.\n");
			}
		}
		break;
		case 8:
		{
#ifdef ENABLE_RCS
			RCSMenu rcsMenu;
			return rcsMenu.exec();
#endif // ENABLE_RCS
		}

		case 11:
		{
			if (iotaState.limsHandle != NULL)
			{
				if (iota_test_register_IPsec() != LIMS_NO_ERROR)
				{
					iota_test_printf("Failed.\n");
				}
				else
				{
					iota_test_printf("Register request sent.\n");
				}
			}
			else
			{
				iota_test_printf("Not initialized.\n");
			}
		}
		break;

		default:
		{
			menu.assign(valString, 0, 1);
			if (menu == "x")
			{
				return EXIT;
			}
			else if (menu == "m")
			{
				print();
			}
			else
			{
				cout << "Invalid Input!" << endl;
			}
		}
	}

	return 0;
}

void exit
	()
{
	cout << endl;
	cout << "Main menu exit..." << endl;
}
};

#define Minor_NUMBER_TEST	0
/* Main entry point to the console test application. */
int main
(
	int argc,
	char *argv[]
)
{
	// Tailor the heading to your own taste.
	cout << "Starting the console..." << " version=" << ECRIO_IOTA_VERSION_STRING << "+" << Minor_NUMBER_TEST << endl;

	if (argc < 2)
	{
		cout << "No configuration file specified!" << endl;

		// @todo Consider having a default configuration (accessed from a default config function).
	}
	else
	{
		// Initialize global state.
		memset(&iotaState, 0, sizeof(iotaState));

		// Set the default local interface (mostly only for Linux).
		strcpy(iotaState.localInterface, "ens33");

		// Set the default audio device name (mostly only for Linux Alsa).
		strcpy(iotaState.audioCaptureDeviceName, "plughw:0,0");
		strcpy(iotaState.audioRenderDeviceName, "plughw:0,0");

		if (argc > 2)
		{
			// The second parameter will override the default local interface setting.
			strcpy(iotaState.localInterface, argv[2]);
		}

		//iotaState.callId = -1;

		iotaState.bRegistered = false;
		iotaState.hQueue = NULL;
		strcpy((char *)iotaState.seed, "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		strcpy((char *)iotaState.seedHex, "abcdef0123456789");

		// @note We assume that the first argument is the config .ini file.
		strcpy(iotaState.configFile, argv[1]);

		iota_test_Setup();

		// The MainMenu and the Menu base class should hold all the state information
		// needed by the application for ease of access.
		MainMenu mainMenu;

		// The event loop to handle menus.
		mainMenu.exec();

		iota_test_Teardown();
	}

	exit(0);
}

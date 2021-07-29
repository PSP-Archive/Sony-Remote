/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Sony Remote Control for PS2 and TV
 * Based on:
 *
 * main.c - Infrared Remote example
 *
 * 
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 * Copyright (c) 2005 Matthew H <matthewh@webone.com.au>
 * --------------------------------------------------
 * Included changes for much larger uses by melman101 : melman101@gmail.com
 * $$
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspsircs.h>
#include <stdlib.h>
#include <string.h>

/* Define the module info section */
PSP_MODULE_INFO("SIRCS", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

// sircs commands are easily found in lirc
// - interpret as described at 
//   http://sourceforge.net/mailarchive/message.php?msg_id=8833252

#define SIRCS_ADDR_DVD	0x1b5a
#define SIRCS_CMD_RESET	0x15
#define SIRCS_CMD_PLAY	0x32
#define SIRCS_CMD_PAUSE	0x39

void send_code(int type, int dev, int cmd) 
{
	struct sircs_data sd;
	int ret;
	int count = 06; // this seems like a good number

	sd.type = type;
	sd.cmd = cmd & 0x7f;
	sd.dev = dev & 0x1fff;

	ret = sceSircsSend(&sd, count); 
	if (ret < 0) 
	{
		printf ("sceSircsSend returned %d\n", ret);
	}
}

/* Exit callback */
int exit_callback(void)
{
	sceKernelExitGame();

	return 0;
}

/* Callback thread */
void CallbackThread(void *arg)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

int main(void)
{
	SceCtrlData pad;
	u32 buttonsold = 0;
	int initial_state = 0;
	int sirc_bits = 20; // # of bits in code, choose from 12, 15 or 20
	int sirc_bits2 = 12;
	int sent = 0;
	int double_button = 0;
	SetupCallbacks();
	pspDebugScreenInit();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	printf ("Remote Control for: ");
	pspDebugScreenSetTextColor(0xCD0000);
	printf ("PS2\n");
	pspDebugScreenSetTextColor(0xFFFFFF);	
        printf ("-------\n");
	printf ("By: melman101\n");
	printf ("Email: melman101@gmail.com\n");
	printf ("Send any suggestions or tips if you got!\n");
	printf ("-------\n");
	pspDebugScreenSetTextColor(0x3300ff);
	printf ("R Trigger & L Trigger Same Time switches remotes between PS2/Wega\n");
	pspDebugScreenSetTextColor(0xFFFFFF);
	printf ("-------\n");
	pspDebugScreenSetTextColor(0x3300ff);
	printf ("Much thanks and help from PSPSDK SIRCS example!\n");
	pspDebugScreenSetTextColor(0xFFFFFF);
      printf ("Square, X, Circle and Triangle all send same symbol\n");
	printf ("Triggers send L1 and R1\n");
      printf ("Up, Down, Left, Right on Directional Pad send same\n");
      printf ("Left Trigger + Start = on - Left Trigger + Select = off\n");
      pspDebugScreenSetXY(0, 2);
	do {
		sceCtrlReadBufferPositive(&pad, 1);
		if (initial_state == 1)
		{
			if (sent == 0)
			{
				if (pad.Lx > 170 && pad.Lx < 255)
				{
					printf("Menu right\n");
					send_code(sirc_bits2, 0x01, 0x33);
					sent = 1;
				} 
				if (pad.Ly > 170 && pad.Ly < 255)
	      		{
					printf("Menu down\n");
					send_code(sirc_bits2, 0x01, 0x75);
					sent = 1;
				}
				if (pad.Lx < 70 && pad.Lx > 0)
				{
					printf("Menu left\n");
					send_code(sirc_bits2, 0x01, 0x34);
					sent = 1;
				}	
				if (pad.Ly < 70 && pad.Ly > 0)
				{
					printf("Menu up\n");
					send_code(sirc_bits2, 0x01, 0x74);
					sent  = 1;
				}
			}
			if ((pad.Ly > 100 && pad.Ly < 150) && (pad.Lx > 100 && pad.Lx < 150))
			{
				sent = 0;
			}
		}
		if (initial_state == 2)
		{
			if (sent == 0)
			{
				if (pad.Lx > 170 && pad.Lx < 255)
				{
					printf("Scan Reverse\n");
					send_code(sirc_bits, SIRCS_ADDR_DVD, 0x34);
					sent = 1;
				} 
				if (pad.Lx < 70 && pad.Lx > 0)
				{
					printf("Scan Forward\n");
					send_code(sirc_bits, SIRCS_ADDR_DVD, 0x33);
					sent = 1;
				}	
			}
			if ((pad.Ly > 100 && pad.Ly < 150) && (pad.Lx > 100 && pad.Lx < 150))
			{
				sent = 0;
			}
		}		
		if (pad.Buttons != buttonsold) 
		{
			// PS2 Remote
			if (initial_state == 0) {
				if ((pad.Buttons & PSP_CTRL_SELECT) && (pad.Buttons & PSP_CTRL_LTRIGGER))
				{
					printf ("PS2 Power Off\n");
					send_code(sirc_bits, SIRCS_ADDR_DVD, 0x2F);
					double_button = 1;
				}
				if ((pad.Buttons & PSP_CTRL_START) && (pad.Buttons & PSP_CTRL_LTRIGGER))
				{
					printf ("PS2 Power On\n");
					send_code(sirc_bits, SIRCS_ADDR_DVD, 0x2E);
					double_button = 1;
				}
				if ((pad.Buttons & PSP_CTRL_LTRIGGER) && (pad.Buttons & PSP_CTRL_RTRIGGER))
				{
					initial_state = 1;
			            pspDebugScreenClear();
                              printf("Remote Control for: ");
                             	pspDebugScreenSetTextColor(0x3300ff);
                             	printf ("Sony WEGA\n");
                             	pspDebugScreenSetTextColor(0xFFFFFF);
					printf ("X = TV/Video                - Circle = Sleep Timer\n");
					printf ("Triangle = Pip on/off       - Square = PIP Swap\n");
					printf ("Left = Decrease Volume      - Right = Increase Volume\n");
					printf ("LTrigger + Triangle = Menu  - Analog Stick to scroll threw\n");
					printf ("Select = Enter on Menu      - Up/Down = Channel Up/Down\n");
					printf ("Start is Power\n");
					double_button = 1;
				}
				if ((pad.Buttons & PSP_CTRL_SELECT) && (double_button == 0))
				{
					printf ("Select Button\n");
					send_code(sirc_bits, SIRCS_ADDR_DVD, 0x50);
				}

				if ((pad.Buttons & PSP_CTRL_START) && (double_button == 0))
				{
					printf ("Start Button\n");
					send_code(sirc_bits, SIRCS_ADDR_DVD, 0x53);
				}
				if ((pad.Buttons & PSP_CTRL_SQUARE)  && (double_button == 0))
				{
					printf ("Sending Square\n");
					send_code(sirc_bits, SIRCS_ADDR_DVD, 0x5F);
				}
                        if ((pad.Buttons & PSP_CTRL_CIRCLE) && (double_button == 0))
				{
        				printf ("Sending Circle\n");
        				send_code(sirc_bits, SIRCS_ADDR_DVD, 0x5D);
				}
		        	if ((pad.Buttons & PSP_CTRL_TRIANGLE) && (double_button == 0))
				{
        				printf ("Sending Triangle\n");
        				send_code(sirc_bits, SIRCS_ADDR_DVD, 0x5C);
				} 
				if ((pad.Buttons & PSP_CTRL_CROSS) && (double_button == 0))
                        {
                             	printf ("Sending X\n");
                             	send_code(sirc_bits, SIRCS_ADDR_DVD, 0x5E);
                       	}
				if ((pad.Buttons & PSP_CTRL_UP) && (double_button == 0))
                        {
                           	printf ("Sending Up\n");
                             	send_code(sirc_bits, SIRCS_ADDR_DVD, 0x54);
                      	}	
	                	if ((pad.Buttons & PSP_CTRL_DOWN) && (double_button == 0))
                       	{
                             	printf ("Sending Down\n");
                             	send_code(sirc_bits, SIRCS_ADDR_DVD, 0x56);
                       	}	
                      	if ((pad.Buttons & PSP_CTRL_LEFT) && (double_button == 0))
                       	{
                             	printf ("Sending Left\n");
                             	send_code(sirc_bits, SIRCS_ADDR_DVD, 0x57);
                       	}
                       	if ((pad.Buttons & PSP_CTRL_RIGHT) && (double_button == 0))
                       	{
                             	printf ("Sending Right\n");
                             	send_code(sirc_bits, SIRCS_ADDR_DVD, 0x55);
                       	}	
                       	if ((pad.Buttons & PSP_CTRL_RTRIGGER) && (double_button == 0))
                       	{
					printf ("R1\n");
                             	send_code(sirc_bits, SIRCS_ADDR_DVD, 0x5B);

			      }
				if ((pad.Buttons & PSP_CTRL_LTRIGGER) && (double_button == 0))
                       	{
					printf ("L1\n");
                             	send_code(sirc_bits, SIRCS_ADDR_DVD, 0x5A);

			      }
			}
			// Sony WEGA Remote
			else if (initial_state == 1)
			{	
                                if ((pad.Buttons & PSP_CTRL_LTRIGGER) && (pad.Buttons & PSP_CTRL_TRIANGLE))
                                {
                                         printf ("Menu\n");
                                   	     send_code(sirc_bits2, 0x01, 0x60);
						     double_button = 1;
                                }

					  if ((pad.Buttons & PSP_CTRL_START) && (double_button == 0))
                                {
                                        printf ("Sending Power ON/OFF to TV!\n");
                                        send_code(sirc_bits2, 0x01, 0x15);
                                }
					  if ((pad.Buttons & PSP_CTRL_LEFT) && (double_button == 0))
                                {
                                        printf ("Decreasing TV Volume\n");
                                        send_code(sirc_bits2, 0x01, 0x13);
                                }
			              if ((pad.Buttons & PSP_CTRL_RIGHT) && (double_button == 0))
                                {
                                        printf ("Increasing TV Volume\n");
                                        send_code(sirc_bits2, 0x01, 0x12);
                                }
                                if ((pad.Buttons & PSP_CTRL_CROSS) && (double_button == 0))
                                {
                                        printf ("TV/VIDEO\n");
                                        send_code(sirc_bits2, 0x01, 0x25);
                                }
                                if ((pad.Buttons & PSP_CTRL_TRIANGLE) && (double_button == 0))
                                {
                                        printf ("PIP On\n");
                                        send_code(sirc_bits2, 0x01, 0x5b);
                                }
                                if ((pad.Buttons & PSP_CTRL_SQUARE) && (double_button == 0))
                                {
                                        printf ("PIP SWAP\n");
                                        send_code(sirc_bits2, 0x01, 0x5f);
                                }
                                if ((pad.Buttons & PSP_CTRL_CIRCLE) && (double_button == 0))
                                {
                                        printf ("Sleep Timer\n");
                                        send_code(sirc_bits2, 0x01, 0x36);
                                }

                                if ((pad.Buttons & PSP_CTRL_SELECT) && (double_button == 0))
                                {
                                         printf ("Enter\n");
                                   	     send_code(sirc_bits2, 0x01, 0x65);
                                }
					  if ((pad.Buttons & PSP_CTRL_UP)  && (double_button == 0))
					  {
						printf ("Channel up\n");
					      send_code(sirc_bits2, 0x01, 0x10);
				        }
					  if ((pad.Buttons & PSP_CTRL_DOWN)  && (double_button == 0))
                                {
                                    printf ("Channel down\n");
                                    send_code(sirc_bits2, 0x01, 0x11);
                                }
					  if ((pad.Buttons & PSP_CTRL_RTRIGGER) && (double_button == 0))
					  {
        					initial_state = initial_state + 1;
						pspDebugScreenClear();
						printf ("Remote Control for ");
        					pspDebugScreenSetTextColor(0x33DDAA);
        					printf ("PS2 DVD Player\n");
        					pspDebugScreenSetTextColor(0xFFFFFF);
				      	printf ("Triangle = On-Screen Controller - Circle = Play\n");
						printf ("X = Stop                        - Up/Down/Left/Right Control Display");
						printf ("Square = Pause                  - Start = Enter\n");
      					printf ("LTrigger = DVD Menu\n");
						printf ("Analog Stick Left = Scan Reverse\n");
						printf ("Analog Stick Left = Scan Forward\n");
					  }  
			}
			// Sony PS2 DVD Remote
			else if (initial_state == 2)
			{	

					  if ((pad.Buttons & PSP_CTRL_START) && (double_button == 0))
                                {
                                         printf ("Enter\n");
                                   	     send_code(sirc_bits, SIRCS_ADDR_DVD, 0x0B);
                                }
                                if ((pad.Buttons & PSP_CTRL_CROSS) && (double_button == 0))
                                {
                                        printf ("Stop\n");
                                        send_code(sirc_bits, SIRCS_ADDR_DVD, 0x38);
                                }
                                if ((pad.Buttons & PSP_CTRL_TRIANGLE) && (double_button == 0))
                                {
                                        printf ("On-Screen Controller\n");
                                        send_code(sirc_bits, SIRCS_ADDR_DVD, 0x50);
                                }
                                if ((pad.Buttons & PSP_CTRL_SQUARE) && (double_button == 0))
                                {
                                        printf ("Pause\n");
                                        send_code(sirc_bits, SIRCS_ADDR_DVD, 0x39);
                                }
                                if ((pad.Buttons & PSP_CTRL_CIRCLE) && (double_button == 0))
                                {
                                        printf ("Play\n");
                                        send_code(sirc_bits, SIRCS_ADDR_DVD, 0x32);
                                }
                                if ((pad.Buttons & PSP_CTRL_LTRIGGER) && (double_button == 0))
                                {
                                         printf ("DVD Menu\n");
                                   	     send_code(sirc_bits, SIRCS_ADDR_DVD, 0x1B);
                                }

					  if ((pad.Buttons & PSP_CTRL_UP)  && (double_button == 0))
					  {
						printf ("Up\n");
					      send_code(sirc_bits, SIRCS_ADDR_DVD, 0x79);
				        }
					  if ((pad.Buttons & PSP_CTRL_DOWN)  && (double_button == 0))
                                {
                                    printf ("Down\n");
                                    send_code(sirc_bits, SIRCS_ADDR_DVD, 0x7A);
                                }
					  if ((pad.Buttons & PSP_CTRL_LEFT) && (double_button == 0))
                                {
                                        printf ("Left\n");
                                        send_code(sirc_bits, SIRCS_ADDR_DVD, 0x7B);
                                }
			              if ((pad.Buttons & PSP_CTRL_RIGHT) && (double_button == 0))
                                {
                                        printf ("Right\n");
                                        send_code(sirc_bits, SIRCS_ADDR_DVD, 0x7C);
                                }
					  if ((pad.Buttons & PSP_CTRL_RTRIGGER) && (double_button == 0))
					  {
        					initial_state = 0;
						pspDebugScreenClear();
						printf ("Remote Control for ");
        					pspDebugScreenSetTextColor(0xCD0000);
        					printf ("PS2\n");
        					pspDebugScreenSetTextColor(0xFFFFFF);
				      	printf ("Square, X, Circle and Triangle all send same symbol\n");
      					printf ("Up, Down, Left, Right on Directional Pad send same\n");
      					printf ("Start is on, Select is off\n");
					  }  
			
			}
			double_button = 0;
			buttonsold = pad.Buttons;
		}

		sceDisplayWaitVblankStart(); 
	} while (1);

	return 0;
}

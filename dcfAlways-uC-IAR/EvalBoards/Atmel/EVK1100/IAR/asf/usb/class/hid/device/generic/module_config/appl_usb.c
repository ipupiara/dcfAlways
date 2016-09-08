#include "conf_usb.h"
#include "usb_protocol.h"
#include "udd.h"
#include "udc_desc.h"
#include "udi.h"
#include "udc.h"

#include <bsp.h>
#include <UartPrint.h>
#include <bsp.h>


uint8_t usbReadyToSend;



void initUSB()
{
    info_printf("starting USB ... \n");
    usbReadyToSend = 0;
    udc_start();
    
}

void attemptSendUSBString(char* st1)
{
    
    if ( usbReadyToSend) {
        // pn 27. jul 12: for a very first test we dont care about strlen(st1 > UDI_HID_REPORT_IN_SIZE
        // because then the 0-termination for the string will not be sent
        udi_hid_generic_send_report_in((uint8_t *)st1);
    }
}

void user_callback_vbus_action(unsigned char b_vbus_high)
{
//    info_printf("user_callback_vbus_action high=%i\n",b_vbus_high);
    if (b_vbus_high) {
        // Connect USB device
        udc_attach();
// ??        usbReadyToSend = 1;
    }else{
        // Disconnect USB device
        udc_detach();
        usbReadyToSend = 0;
    }
}

bool user_callback_UDI_HID_GENERIC_ENABLE_EXT() 
{
//    info_printf("user_callback_UDI_HID_GENERIC_ENABLE_EXT\n");
    usbReadyToSend = 1;
    return 1;

}



bool user_callback_UDI_HID_GENERIC_DISABLE_EXT()
{
//     info_printf("user_callback_UDI_HID_GENERIC_DISABLE_EXT \n");
     usbReadyToSend = 0;
     return 1;
}

void user_callback_UDI_HID_GENERIC_REPORT_IN_SENT()
{
//    info_printf("REPORT_IN_SENT\n");
    usbReadyToSend = 1;
}

void user_callback_UDI_HID_GENERIC_REPORT_OUT(uint8_t* ptr)
{
 //   info_printf("REPORT_OUT %s\n",ptr);
    
  	if ( ptr[0]=='1') {
		// A led must be on
		switch(ptr[1]) {
          	case '1':
			LED_On(LED0);
			break;
			case '2':
			LED_On(LED1);
			break;
			case '3':
			LED_On(LED2);
			break;
			case '4':
			LED_On(LED3);
			break;
		}
	} else {
		// A led must be off
		switch(ptr[1]) {
          	case '1':
			LED_Off(LED0);
			break;
			case '2':
			LED_Off(LED1);
			break;
			case '3':
			LED_Off(LED2);
			break;
			case '4':
			LED_Off(LED3);
			break;
		}
	}  
    

}

void user_callback_UDI_HID_GENERIC_SET_FEATURE(uint8_t*  ptr)
{
    info_printf("SET_FEATURE\n");

/*    
        code from original example 
    
    	if (*((uint32_t*)report)==0x55AA55AA) {
		// Disconnect USB Device
		udc_stop();
		ui_powerdown();
	}
*/    
    
  

}


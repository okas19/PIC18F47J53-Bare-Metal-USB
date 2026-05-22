#include <xc.h>
#include "usb_ds.h"
#include "config.h"
#include "Pin_config.h"
#include "Led.h"

volatile bool ep1_in_toggle = 0;
volatile bool ep1_out_toggle = 0;
volatile uint8_t ep0_in_toggle = 1;  // The first IN data packet is always DATA1
volatile uint8_t ep0_out_toggle = 1; // The first OUT data packet is always DATA1
volatile usb_state_t device_state = USB_STATE_IDLE;
volatile bool remote_wakeup_enabled = 0;

const HandlerFunc jump_table[2][2] = {
    { handle_endpoint_0_OUT, handle_endpoint_0_IN },
    { handle_endpoint_1_OUT, handle_endpoint_1_IN }
};

void config_endpoint_gate(uint8_t ep_num, uint8_t flags) {
    *uep_registers[ep_num] = flags;
}

void give_EP0_OUT_ownership_to_SIE(void) {
	BDT[0].Stat.CPU.UOWN = 1;      // Give ownership to SIE ; 
}

void give_EP0_IN_ownership_to_SIE(void) {
	BDT[1].Stat.CPU.UOWN = 1;      // Give ownership to SIE ; 
}

void give_EP1_OUT_ownership_to_SIE(void) {
	BDT[2].Stat.CPU.UOWN = 1;      // Give ownership to SIE ; 
}

void give_EP1_IN_ownership_to_SIE(void) {
	BDT[3].Stat.CPU.UOWN = 1;      // Give ownership to SIE ; 
}

void arm_endpoint_0_OUT(uint8_t count, uint16_t address) {
	BDT[0].Stat.Val = 0; // Take ownership
    BDT[0].CNT = (uint8_t)(count & 0xFF);
    BDT[0].ADRH = (uint8_t)(address >> 8);;
	BDT[0].ADRL = (uint8_t)(address & 0xFF);
    if (ep0_out_toggle == 1) {
        BDT[0].Stat.Val = 0xC8; // UOWN=1, DTS=1, DTSEN=1 (Wait for DATA1)
    } else {
        BDT[0].Stat.Val = 0x88; // UOWN=1, DTS=0, DTSEN=1 (Wait for DATA0)
    }
    ep0_out_toggle ^= 1; // Toggle the bit for the next packet
}

void arm_endpoint_0_IN(uint8_t count, uint16_t address) {
	BDT[1].Stat.Val = 0; // Take ownership
    BDT[1].CNT = (uint8_t)(count & 0xFF);
    BDT[1].ADRH = (uint8_t)(address >> 8);;
	BDT[1].ADRL = (uint8_t)(address & 0xFF);
    if (ep0_in_toggle == 1) {
        BDT[1].Stat.Val = 0xC8; // UOWN=1, DTS=1, DTSEN=1 (Send DATA1)
    } else {
        BDT[1].Stat.Val = 0x88; // UOWN=1, DTS=0, DTSEN=1 (Send DATA0)
    }
    
    ep0_in_toggle ^= 1; // Toggle the bit for the next packet 
}

void arm_endpoint_1_OUT(uint8_t count, uint16_t address) {
    BDT[2].Stat.Val = 0; // Take ownership
    BDT[2].CNT = (uint8_t)(count & 0xFF);
    BDT[2].ADRH = (uint8_t)(address >> 8);;
	BDT[2].ADRL = (uint8_t)(address & 0xFF);
    if(ep1_out_toggle) {
        BDT[2].Stat.Val = 0xC8; // UOWN=1, DTSEN=1, DTS=1 (DATA1) Give ownership to SIE
    } else {
        BDT[2].Stat.Val = 0x88; // UOWN=1, DTSEN=1, DTS=0 (DATA0) Give ownership to SIE
    }
	//BDT[2].Stat.CPU.UOWN = 1;      // Give ownership to SIE 
}

void arm_endpoint_1_IN(uint8_t count, uint16_t address) {
	BDT[3].Stat.Val = 0; // Take ownership
    BDT[3].CNT = (uint8_t)(count & 0xFF);
    BDT[3].ADRH = (uint8_t)(address >> 8);;
	BDT[3].ADRL = (uint8_t)(address & 0xFF);
    if(ep1_in_toggle) {
        BDT[3].Stat.Val = 0xC8; // UOWN=1, DTSEN=1, DTS=1 (DATA1) Give ownership to SIE
    } else {
        BDT[3].Stat.Val = 0x88; // UOWN=1, DTSEN=1, DTS=0 (DATA0) Give ownership to SIE
    }
	//BDT[3].Stat.CPU.UOWN = 1;      // Give ownership to SIE  
}

void handle_get_protocol(USB_Setup_Packet* ctrl_pkt) {
    device_state = USB_STATE_DATA_IN;
    Buffer_area_EP0_IN[0] = 0x01;                    // Report protocol (0x01), boot protocol (0x00)
    arm_endpoint_0_IN(1, (uint16_t)Buffer_area_EP0_IN);
    //give_EP0_IN_ownership_to_SIE();
    arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
    //give_EP0_OUT_ownership_to_SIE();
}

void handle_set_protocol(USB_Setup_Packet* ctrl_pkt) {
    device_state = USB_STATE_STATUS;
    // wValue low byte: 0x00 = boot, 0x01 = report. Can optionally be saved, for now we just acknowledge.
    arm_endpoint_0_IN(0, (uint16_t)Buffer_area_EP0_IN);
    //give_EP0_IN_ownership_to_SIE();
}

void handle_get_report(USB_Setup_Packet* ctrl_pkt) {
	uint8_t report_type = SETUP_PACKET.wValue >> 8;
	uint8_t report_byte = 0;
	device_state = USB_STATE_DATA_IN;
	switch (report_type)
		{
			case 0x01:  // INPUT REPORT
				if(Led_A0_LAT == 1) {report_byte = report_byte | 0x01;}
				if(Led_A1_LAT == 1) {report_byte = report_byte | 0x02;}
				USB_Data_Copy((const uint8_t*)&report_byte, (uint8_t*)Buffer_area_EP0_IN, 1);
				arm_endpoint_0_IN(1, (uint16_t)Buffer_area_EP0_IN);
				//give_EP0_IN_ownership_to_SIE();
				arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
				//give_EP0_OUT_ownership_to_SIE(); 
				break;
			case 0x02: // OUTPUT REPORT
				//
				break;
			case 0x03: // FEATURE REPORT
				//
				break;
			default:
			
				break;
		}		
}

void handle_set_idle(USB_Setup_Packet* ctrl_pkt) {
	device_state = USB_STATE_STATUS;
	arm_endpoint_0_IN(0, (uint16_t)Buffer_area_EP0_IN);
	//give_EP0_IN_ownership_to_SIE();
    arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
}

void handle_set_configuration(USB_Setup_Packet* ctrl_pkt) {
	device_state = USB_STATE_STATUS;
	if((SETUP_PACKET.wValue & 0xFF) == 0x01)
	{
		is_device_configured = 1;
	}
	arm_endpoint_0_IN(0, (uint16_t)Buffer_area_EP0_IN);
	//give_EP0_IN_ownership_to_SIE();
    arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
	arm_endpoint_1_OUT(64, (uint16_t)Buffer_area_EP1_OUT);
    // Reset toggle states (Initial packets will be DATA0)
    ep1_in_toggle = 0;
    ep1_out_toggle = 0;
	//give_EP1_OUT_ownership_to_SIE();
    // Place a valid report into the buffer first
    Buffer_area_EP1_IN[0] = (Led_A0_LAT ? 0x01 : 0x00) | (Led_A1_LAT ? 0x02 : 0x00);
    arm_endpoint_1_IN(1, (uint16_t)Buffer_area_EP1_IN);
}

void handle_clear_feature(USB_Setup_Packet* ctrl_pkt) {
device_state = USB_STATE_STATUS;
// Mask the last 5 bits of bmRequestType to determine the recipient
uint8_t recipient = SETUP_PACKET.bmRequestType & 0x1F;
if (recipient == 0x00) { 
    // RECIPIENT: DEVICE
    // If wValue == 1, this is a DEVICE_REMOTE_WAKEUP request
    if (SETUP_PACKET.wValue == 1) {
        remote_wakeup_enabled = 0; // PC revoked permission, stored in memory
    }
} 
else if (recipient == 0x02) { 
    // RECIPIENT: ENDPOINT 
    // If wValue == 0, this is an ENDPOINT_HALT (STALL) clear request
    if (SETUP_PACKET.wValue == 0) {
        uint8_t ep = SETUP_PACKET.wIndex & 0x0F;
        uint8_t dir = (SETUP_PACKET.wIndex >> 7) & 0x01; // 1 = IN, 0 = OUT

        if (ep == 0) {
            UEP0bits.EPSTALL0 = 0;
        } 
        else if (ep == 1) {
            UEP1bits.EPSTALL1 = 0;

            // USB STANDARD RULE: When an endpoint stall state is cleared, 
            // the data toggle (DTS) bit for that direction must be cleared to DATA0 (0)!
            if (dir == 1) {
                ep1_in_toggle = 0;  // Reset IN direction toggle state
            } else {
                ep1_out_toggle = 0; // Reset OUT direction toggle state
            }
        }
    }
}
// Prepare Endpoint 0 for the status stage
arm_endpoint_0_IN(0, (uint16_t)Buffer_area_EP0_IN);
arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT); 
}

void handle_set_feature(USB_Setup_Packet* ctrl_pkt) {
device_state = USB_STATE_STATUS;
// Mask the last 5 bits of bmRequestType to determine the recipient
uint8_t recipient = SETUP_PACKET.bmRequestType & 0x1F;

if (recipient == 0x00) { 
    // RECIPIENT: DEVICE
    // If wValue == 1, this is a DEVICE_REMOTE_WAKEUP enablement request
    if (SETUP_PACKET.wValue == 1) {
        remote_wakeup_enabled = 1; // PC granted us remote wake-up permission!
    }
} 
else if (recipient == 0x02) { 
    // RECIPIENT: ENDPOINT (If the Host intentionally wants to lock/STALL an endpoint)
    // If wValue == 0, this is an ENDPOINT_HALT (STALL) request
    if (SETUP_PACKET.wValue == 0) {
        uint8_t ep = SETUP_PACKET.wIndex & 0x0F;

        if (ep == 0) {
            UEP0bits.EPSTALL0 = 1;
        } 
        else if (ep == 1) {
            UEP1bits.EPSTALL1 = 1;
        }
    }
}
// Prepare Endpoint 0 to confirm the status stage (Z0LP)
arm_endpoint_0_IN(0, (uint16_t)Buffer_area_EP0_IN);
arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT); 
}


void handle_set_address(USB_Setup_Packet* ctrl_pkt) {
	device_state = USB_STATE_STATUS;
	usb_address = (uint8_t)(SETUP_PACKET.wValue & 0x00FF);
	is_set_address_status = 1;
	arm_endpoint_0_IN(0, (uint16_t)Buffer_area_EP0_IN);
	//give_EP0_IN_ownership_to_SIE();
    arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
}

void handle_get_descriptor(USB_Setup_Packet* ctrl_pkt) {
	device_state = USB_STATE_DATA_IN;
	uint8_t desc_type = SETUP_PACKET.wValue >> 8;      // High byte: descriptor type
    uint8_t desc_index = SETUP_PACKET.wValue & 0xFF;   // Low byte: index (required for string)
	uint8_t data_length_actual = 0;
    uint8_t data_length_to_be_sent = 0;
	uint8_t data_length_requested = (uint8_t)(SETUP_PACKET.wLength & 0x00FF);
	switch (desc_type)
		{
			case 0x01:  // DEVICE DESCRIPTOR
				// Data to be written in Buffer_area_EP0_IN
				data_length_actual = sizeof(device_dsc);
				data_length_to_be_sent = (data_length_requested > data_length_actual) ? data_length_actual : data_length_requested;
				USB_Data_Copy((const uint8_t*)device_dsc, (uint8_t*)Buffer_area_EP0_IN, data_length_to_be_sent);
				// End of explanation
				
				// Arm EP0_IN
				arm_endpoint_0_IN(data_length_to_be_sent, (uint16_t)Buffer_area_EP0_IN);
				// give_EP0_IN_ownership_to_SIE();
				// End of explanation
				
				// Arm EP0_OUT
				arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
				// give_EP0_OUT_ownership_to_SIE();
				// End of explanation
                break;
			case 0x02:	// CONFIGURATION DESCRIPTOR
				// Data to be written in Buffer_area_EP0_IN
				data_length_actual = sizeof(config_dsc);
				data_length_to_be_sent = (data_length_requested > data_length_actual) ? data_length_actual : data_length_requested;
				USB_Data_Copy((const uint8_t*)config_dsc, (uint8_t*)Buffer_area_EP0_IN, data_length_to_be_sent);
				// End of explanation
				
				// Arm EP0_IN
				arm_endpoint_0_IN(data_length_to_be_sent, (uint16_t)Buffer_area_EP0_IN);
				//give_EP0_IN_ownership_to_SIE();
				// End of explanation
				
				// Arm EP0_OUT
				arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
				//give_EP0_OUT_ownership_to_SIE();
				// End of explanation
				break;
			case 0x03:  // STRING DESCRIPTOR
				switch (desc_index)
				{
					case 0x00:  // STRING INDEX 0
						// Data to be written in Buffer_area_EP0_IN
						data_length_actual = sizeof(lang_id_dsc);
						data_length_to_be_sent = (data_length_requested > data_length_actual) ? data_length_actual : data_length_requested;
						USB_Data_Copy((const uint8_t*)lang_id_dsc, (uint8_t*)Buffer_area_EP0_IN, data_length_to_be_sent);
						// End of explanation
						
						// Arm EP0_IN
						arm_endpoint_0_IN(data_length_to_be_sent, (uint16_t)Buffer_area_EP0_IN);
						//give_EP0_IN_ownership_to_SIE();
						// End of explanation
						
						// Arm EP0_OUT
						arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
						//give_EP0_OUT_ownership_to_SIE();
						// End of explanation
						break;
					case 0x01:	// STRING INDEX 1
						// Data to be written in Buffer_area_EP0_IN
						data_length_actual = sizeof(manufacturer_string_dsc);
						data_length_to_be_sent = (data_length_requested > data_length_actual) ? data_length_actual : data_length_requested;
						USB_Data_Copy((const uint8_t*)manufacturer_string_dsc, (uint8_t*)Buffer_area_EP0_IN, data_length_to_be_sent);
						// End of explanation
						
						// Arm EP0_IN
						arm_endpoint_0_IN(data_length_to_be_sent, (uint16_t)Buffer_area_EP0_IN);
						//give_EP0_IN_ownership_to_SIE();
						// End of explanation
						
						// Arm EP0_OUT
						arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
						//give_EP0_OUT_ownership_to_SIE();
						// End of explanation
						break;
					case 0x02:  // STRING INDEX 2
						// Data to be written in Buffer_area_EP0_IN
						data_length_actual = sizeof(product_string_dsc);
						data_length_to_be_sent = (data_length_requested > data_length_actual) ? data_length_actual : data_length_requested;
						USB_Data_Copy((const uint8_t*)product_string_dsc, (uint8_t*)Buffer_area_EP0_IN, data_length_to_be_sent);
						// End of explanation
						
						// Arm EP0_IN
						arm_endpoint_0_IN(data_length_to_be_sent, (uint16_t)Buffer_area_EP0_IN);
						//give_EP0_IN_ownership_to_SIE();
						// End of explanation
						
						// Arm EP0_OUT
						arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
						//give_EP0_OUT_ownership_to_SIE();
						// End of explanation
						break;
					default:
					
						break;
				}
				break;
			case 0x22:  // HID REPORT
				// Data to be written in Buffer_area_EP0_IN
				data_length_actual = sizeof(report_dsc);
				data_length_to_be_sent = (data_length_requested > data_length_actual) ? data_length_actual : data_length_requested;
				USB_Data_Copy((const uint8_t*)report_dsc, (uint8_t*)Buffer_area_EP0_IN, data_length_to_be_sent);
				// End of explanation
				
				// Arm EP0_IN
				arm_endpoint_0_IN(data_length_to_be_sent, (uint16_t)Buffer_area_EP0_IN);
				//give_EP0_IN_ownership_to_SIE();
				// End of explanation
				
				// Arm EP0_OUT
				arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
				//give_EP0_OUT_ownership_to_SIE();
				// End of explanation
			
				break;	
			default:
			
				break;
		}
}

void endpoint_0_OUT_set_up_handler(USB_Setup_Packet* ctrl_pkt) {
	ep0_in_toggle = 1;
    ep0_out_toggle = 1;
    uint8_t req_type = SETUP_PACKET.bmRequestType & 0x60;
	if (req_type == 0x00) {
		// Standard requests: bRequest 0x05, 0x06, 0x09...
		switch (SETUP_PACKET.bRequest)
		{
			case 0x01:  // CLEAR FEATURE
                handle_clear_feature(ctrl_pkt);
                break;
            case 0x03:  // SET FEATURE
                handle_set_feature(ctrl_pkt);
                break;
            case 0x05:  // SET_ADDRESS
				handle_set_address(ctrl_pkt);  
				break;
			case 0x06:	// GET_DESCRIPTOR
				handle_get_descriptor(ctrl_pkt);
				break;
			case 0x09:  // SET_CONFIGURATION
				handle_set_configuration(ctrl_pkt);
				break;
			default:
			
				break;
		}
		
	} else if (req_type == 0x20) {
		// HID class requests: bRequest 0x03, 0x0A...
		switch (SETUP_PACKET.bRequest)
		{
			case 0x01:  // GET_REPORT
				handle_get_report(ctrl_pkt);
				break;
			case 0x03:	// GET_PROTOCOL
				handle_get_protocol(ctrl_pkt);
				break;
			case 0x0A:  // SET_IDLE
				handle_set_idle(ctrl_pkt);
                break;
			case 0x0B:  // SET_PROTOCOL
				handle_set_protocol(ctrl_pkt);
				break;	
			default:
			
				break;
		}
	} else {
		// Unsupported -> STALL
	};
}

void handle_endpoint_0_OUT(uint8_t pid, USB_Setup_Packet* ctrl_pkt) {
    switch (pid) 
    {
        case PID_SET_UP:
            device_state = USB_STATE_SET_UP; //state_setup(); 
            endpoint_0_OUT_set_up_handler(ctrl_pkt);
			UCONbits.PKTDIS = 0;
            break;
        case PID_IN:
            if(device_state == USB_STATE_SET_UP || device_state == USB_STATE_DATA_IN)
            {
                device_state = USB_STATE_DATA_IN; //state_datain();
            }
            else if(device_state == USB_STATE_DATA_OUT)
            {
                device_state = USB_STATE_STATUS; //state_status();
            }
            break;
        case PID_OUT:
            if(device_state == USB_STATE_SET_UP || device_state == USB_STATE_DATA_OUT)
            {
                device_state = USB_STATE_DATA_OUT; //state_dataout();
            }
            else if(device_state == USB_STATE_DATA_IN)   
            {
                device_state = USB_STATE_STATUS; //state_status(); 
            }
			else if(device_state == USB_STATE_STATUS)   
			{
				device_state = USB_STATE_IDLE; //state_idle(); // ZLP received, status completed
                arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
			}
            else if (device_state == USB_STATE_IDLE)    // Unexpected OUT token, safely ignore
            {
                // Do nothing
            }
            break;
        case PID_ACK:
            if(device_state == USB_STATE_STATUS)
            {
                device_state = USB_STATE_IDLE;//state_idle();
            }
            break;
        default:
            break;  
    }
}

void handle_endpoint_0_IN(uint8_t pid, USB_Setup_Packet* ctrl_pkt) {
    switch (pid)
    {
        case PID_IN:
            if (device_state == USB_STATE_DATA_IN) {
                // Data sent, proceed to status stage
                device_state = USB_STATE_STATUS;
            } else if (device_state == USB_STATE_STATUS) {
                // Status stage IN token (send empty packet)
                if (is_set_address_status) {
                    UADDR = usb_address;                // Update address immediately
                    BDT[0].Stat.CPU.DTS = 0;           // Reset Data Toggle for the new address
                    is_set_address_status = 0;
                }
                device_state = USB_STATE_IDLE;
            }
            break;
        case PID_ACK:
            if (device_state == USB_STATE_STATUS) {
                device_state = USB_STATE_IDLE;
            }
            break;
        default:
            break;
    }
} 

void handle_endpoint_1_OUT(uint8_t pid, USB_Setup_Packet* ctrl_pkt) {
    test_A2_LAT =! test_A2_LAT;
    uint8_t output_report = 0;
    output_report = Buffer_area_EP1_OUT[0];
    // LED 0 Control (is the 1st bit set?)
    if ((output_report & 0x01) != 0) {
        Led_0_On();
    } else {
        Led_0_Off();
    }
    
    // LED 1 Control (is the 2nd bit set?)
    if ((output_report & 0x02) != 0) { // Correct masking method
        Led_1_On();
    } else {
        Led_1_Off();
    }

    uint8_t report_byte = (Led_A0_LAT ? 0x01 : 0x00) | (Led_A1_LAT ? 0x02 : 0x00);
    Buffer_area_EP1_IN[0] = report_byte;
    // Reception successful, toggle OUT toggle for the next packet and arm
    ep1_out_toggle = !ep1_out_toggle;
    arm_endpoint_1_IN(1, (uint16_t)Buffer_area_EP1_IN);
    arm_endpoint_1_OUT(64, (uint16_t)Buffer_area_EP1_OUT);
	
}

void handle_endpoint_1_IN(uint8_t pid, USB_Setup_Packet* ctrl_pkt) {
    test_B2_LAT =! test_B2_LAT;
    //Led_A0_LAT =! Led_A0_LAT;
    //Led_A1_LAT =! Led_A1_LAT;
    uint8_t report_byte = 0;
    if(Led_A0_LAT == 1) {report_byte = report_byte | 0x01;}
	if(Led_A1_LAT == 1) {report_byte = report_byte | 0x02;}
	USB_Data_Copy((const uint8_t*)&report_byte, (uint8_t*)Buffer_area_EP1_IN, 1);
    // Transmission successful, toggle IN toggle for the next packet and arm
    ep1_in_toggle = !ep1_in_toggle;
	arm_endpoint_1_IN(1, (uint16_t)Buffer_area_EP1_IN);
} 

void execute_handler(uint8_t x, uint8_t y, uint8_t pid) {
    if (x < 2 && y < 2) {
        HandlerFunc function_to_be_called = jump_table[x][y]; 
		if (function_to_be_called != NULL)
		{
            // We pass the ADDRESS of our defined SETUP_PACKET
            function_to_be_called(pid, &SETUP_PACKET);
		}
    }
}

void check_led_status(void) {
	if(Led_A0_LAT == 1)
	{
		current_Led_A0_state = 1;
	}
	else
	{	
		current_Led_A0_state = 0;
	}
	if(Led_A1_LAT == 1)
	{
		current_Led_A1_state = 1;
	}
	else 
	{
		current_Led_A1_state = 0;
	}
} 

void Prepare_BDT(void)
{
    arm_endpoint_0_OUT(64, (uint16_t)Buffer_area_EP0_OUT);
    //give_EP0_OUT_ownership_to_SIE();
    
    arm_endpoint_0_IN(64, (uint16_t)Buffer_area_EP0_IN);
    //give_EP0_IN_ownership_to_SIE();
    
    arm_endpoint_1_OUT(64, (uint16_t)Buffer_area_EP1_OUT);
    //give_EP1_OUT_ownership_to_SIE();
    
    arm_endpoint_1_IN(64, (uint16_t)Buffer_area_EP1_IN); //Should not be armed; otherwise, random data is sent if host executes ep1 PID IN after enumeration.
    //give_EP1_IN_ownership_to_SIE();
    
	config_endpoint_gate(0, USB_EPINEN_MASK | USB_EPOUTEN_MASK | USB_EPHSHK_MASK);
    config_endpoint_gate(1, USB_EPINEN_MASK | USB_EPOUTEN_MASK | USB_EPHSHK_MASK); 
}

void USB_Initialize(void)
{
    // PLL settings are done via PLLDIV<2:0>, FOSC2, PLLEN, CFGPLLEN in config.h Last setting via FSEN by register UCFG 
    Prepare_BDT(); 
    if (UCONbits.USBEN == 0)
    {
        test_B3_LAT =! test_B3_LAT;
        UCFGbits.FSEN = 1;
        UCFGbits.PPB0 = 0;
        UCFGbits.PPB1 = 0;  // Ping pong disabled  
        UCFGbits.UTRDIS = 0;
        UCFGbits.UPUEN = 1; // Pull ups enabled
              
        // Start of Top level usb interrupts for error detection
        UIRbits.SOFIF = 0;
        UIEbits.SOFIE = 0;
        
        UIRbits.TRNIF = 0; 
        UIEbits.TRNIE = 1;
        
        UIRbits.IDLEIF = 0;
        UIEbits.IDLEIE = 1;
               
        UIRbits.UERRIF = 0; // Read only!
        UIEbits.UERRIE = 1;
        
        UIRbits.STALLIF = 0;
        UIEbits.STALLIE = 1;
        
        UIRbits.ACTVIF = 0;
        UIEbits.ACTVIE = 0; // Inside ISR 
        
        UIRbits.URSTIF = 0;
        UIEbits.URSTIE = 1;
        // End of Top level usb interrupts for error detection
        
        // Start of Second level usb error conditions
        UEIR = 0;
        UEIE = 0x9F;
        // End of Second level usb error conditions
        
        PIR2bits.USBIF = 0;
        PIE2bits.USBIE = 1;
    }
    Enable_USB();
}

void Enable_USB(void)
{
    if (UCONbits.SUSPND == 1)
    {
        UCONbits.SUSPND = 0;
        __delay_ms(2);
        UCONbits.USBEN = 1;
        
    }
    else
    {
        __delay_ms(2); // wait for PLL lock 
        UCONbits.USBEN = 1;
    }
}

void USB_Data_Copy(const uint8_t *source, uint8_t *destination, uint8_t length)
{
    // Using 8-bit loop index and length to minimize CPU overhead
    for(uint8_t i = 0; i < length; i++)
    {
        // Read value from source, write to destination, and increment both addresses
        *destination++ = *source++;
    }
}

void USB_Remote_Wakeup_PC(void)
{
    // Should only execute if the computer suspended us (SUSPND=1) and the PC granted us remote wake-up permission
    if(UCONbits.SUSPND == 1 && remote_wakeup_enabled == 1) 
    {
        // 1. To prevent our own signal from causing an untimely jump into the ISR
        // We temporarily disable the Activity interrupt permission.
        UIEbits.ACTVIE = 0;
        UCONbits.RESUME = 1;   // Line is pulled to 'K' state (we tell the PC to "Wake up!")
        __delay_ms(10);        // Wait exactly 10 ms for USB 2.0 Spec Table compliance
        UCONbits.RESUME = 0;   // Terminate signal, leave control to Host
        UCONbits.SUSPND = 0;   // We are also waking up from sleep mode
        // 4. Clear the "false" activity flag raised by the hardware during signaling
        UIRbits.ACTVIF = 0;    
        // 5. Re-enable the interrupt gate for future real Suspend/Resume events
        UIEbits.ACTVIE = 1; 
    }
}
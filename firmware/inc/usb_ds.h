#include <xc.h> // include processor files - each processor file is guarded. 
#include <stdbool.h>

// Device Descriptor (18 Byte)
const uint8_t device_dsc[] = {
    0x12,                   // bLength
    0x01,                   // bDescriptorType (Device)
    0x01, 0x01,             // bcdUSB (v1.10)
    0x00,                   // bDeviceClass
    0x00,                   // bDeviceSubClass
    0x00,                   // bDeviceProtocol
    0x40,                   // bMaxPacketSize0 (64 byte)
    0x34, 0x12,             // idVendor (0x1234)
    0x78, 0x56,             // idProduct (0x5678)
    0x00, 0x01,             // bcdDevice (v1.00)
    0x01,                   // iManufacturer (Index 1)
    0x02,                   // iProduct (Index 2)
    0x00,                   // iSerialNumber
    0x01                    // bNumConfigurations
}; 

// Manufacturer String Descriptor (Index 1)
const uint8_t manufacturer_string_dsc[] = {
    20,                     // bLength (8 characters * 2 + 2 = 18)
    0x03,                   // bDescriptorType (String)
    'M', 0x00, 'y', 0x00, 'C', 0x00, 'o', 0x00, 'm', 0x00, 'p', 0x00, 'a', 0x00, 'n', 0x00, 'y', 0x00
};

// String Descriptor 2 (Product Name)
// "PIC18F47J53 Led Controller"
const uint8_t product_string_dsc[] = {
    54,                     // bLength
    0x03,                   // bDescriptorType (String)
    'P', 0x00, 'I', 0x00, 'C', 0x00, '1', 0x00, '8', 0x00, 'F', 0x00, 
    '4', 0x00, '7', 0x00, 'J', 0x00, '5', 0x00, '3', 0x00, ' ', 0x00, 
    'L', 0x00, 'e', 0x00, 'd', 0x00, ' ', 0x00, 'C', 0x00, 'o', 0x00, 
    'n', 0x00, 't', 0x00, 'r', 0x00, 'o', 0x00, 'l', 0x00, 'l', 0x00, 
    'e', 0x00, 'r', 0x00
};

const uint8_t lang_id_dsc[] = {
    0x04,                   // bLength
    0x03,                   // bDescriptorType
    0x09, 0x04              // Language ID (English US: 0x0409)
}; 

// --- CORRECTED Configuration Descriptor Set (no embedded Report descriptor) ---
const uint8_t config_dsc[] = {
    // 1. Configuration Descriptor (9 byte)
    0x09,           // bLength
    0x02,           // bDescriptorType: CONFIGURATION (0x02)
    0x29, 0x00,     // wTotalLength: 9(Config)+9(Interface)+9(HID)+7(EP1 IN)+7(EP1 OUT) = 41 bytes (0x29)
    0x01,           // bNumInterfaces
    0x01,           // bConfigurationValue
    0x00,           // iConfiguration
    0xA0,           // bmAttributes (Bus-powered, Remote Wakeup)
    0x19,           // bMaxPower (50 mA)

    // 2. Interface Descriptor (9 byte)
    0x09,           // bLength
    0x04,           // bDescriptorType: INTERFACE
    0x00,           // bInterfaceNumber
    0x00,           // bAlternateSetting
    0x02,           // bNumEndpoints (excluding EP0)
    0x03,           // bInterfaceClass: HID
    0x00,           // bInterfaceSubClass
    0x00,           // bInterfaceProtocol
    0x00,           // iInterface

    // 3. HID Descriptor (9 byte)
    0x09,           // bLength
    0x21,           // bDescriptorType: HID
    0x11, 0x01,     // bcdHID (1.11)
    0x00,           // bCountryCode
    0x01,           // bNumDescriptors
    0x22,           // bDescriptorType[0]: Report (0x22)
    0x22, 0x00,     // wDescriptorLength: Report descriptor length (35 bytes, see report_dsc[]) 

    // 4. Endpoint 1 IN Descriptor (7 byte)
    0x07,           // bLength
    0x05,           // bDescriptorType: ENDPOINT
    0x81,           // bEndpointAddress: EP1 IN
    0x03,           // bmAttributes: Interrupt
    0x40, 0x00,     // wMaxPacketSize: 64 bytes
    0x64,           // bInterval: 100 ms 

    // 5. Endpoint 1 OUT Descriptor (7 byte)
    0x07,           // bLength
    0x05,           // bDescriptorType: ENDPOINT
    0x01,           // bEndpointAddress: EP1 OUT
    0x03,           // bmAttributes: Interrupt
    0x40, 0x00,     // wMaxPacketSize: 64 bytes
    0x01            // bInterval: 1 ms
};

// --- HID Report Descriptor (separate, as required by HID spec) ---
const uint8_t report_dsc[] = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined) 0xFF00
    0x09, 0x01,        // Usage (Vendor 1)
    0xA1, 0x01,        // Collection (Application)

    // OUTPUT: 1 byte (LED Control)
    0x09, 0x01,        //   Usage (Vendor 1)    - LED data
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)   (0x26: 2-byte data)
    0x75, 0x08,        //   Report Size (8 bit)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x02,        //   Output (Data, Var, Abs)

    // INPUT: 1 byte (LED status readback)
    0x09, 0x02,        //   Usage (Vendor 2)    - LED status
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bit)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data, Var, Abs)

    0xC0               // End Collection
};

// *** NEW USB_Setup_Packet definition (simple struct, not union) ***
typedef struct __attribute__((packed)) {
    uint8_t   bmRequestType;
    uint8_t   bRequest;
    uint16_t  wValue;
    uint16_t  wIndex;
    uint16_t  wLength;
} USB_Setup_Packet;

// *** Macro name changed: SETUP_PACKET ***
#define SETUP_PACKET (*((USB_Setup_Packet*)0x500))

typedef union _BD_STAT
{
    uint8_t Val;
    struct {
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned BSTALL:1;
        unsigned DTSEN:1;
        unsigned res4:1;
        unsigned res5:1;
        unsigned DTS:1;
        unsigned UOWN:1;
    } CPU;
    struct {
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned PID0:1;
        unsigned PID1:1;
        unsigned PID2:1;
        unsigned PID3:1;
        unsigned res6:1;
        unsigned UOWN:1;
    } SIE;
} BD_STAT; 

typedef struct _BDT_ENTRY
{
    BD_STAT Stat;
    uint8_t CNT;
    uint8_t ADRL;
    uint8_t ADRH;
} BDT_ENTRY;

// Bank 15 of PIC18F47J53
volatile BDT_ENTRY BDT[64] __at(0xD00);
volatile unsigned char Buffer_area_EP0_OUT[64]  __at(0x500);
volatile unsigned char Buffer_area_EP0_IN[64]   __at(0x540);
volatile unsigned char Buffer_area_EP1_OUT[64]  __at(0x580);
volatile unsigned char Buffer_area_EP1_IN[64]   __at(0x5C0);

#define USB_EPSTALL_MASK 	0x01
#define USB_EPINEN_MASK  	0x02
#define USB_EPOUTEN_MASK 	0x04
#define USB_EPCONDIS_MASK 	0x08
#define USB_EPHSHK_MASK 	0x10

volatile uint8_t* const uep_registers[] = {
        &UEP0, &UEP1, &UEP2, &UEP3, &UEP4, &UEP5, &UEP6, &UEP7,
        &UEP8, &UEP9, &UEP10, &UEP11, &UEP12, &UEP13, &UEP14, &UEP15
    };
	
typedef enum {
    USB_STATE_IDLE,
    USB_STATE_SET_UP,
    USB_STATE_DATA_IN,
    USB_STATE_DATA_OUT,
    USB_STATE_STATUS
} usb_state_t;

typedef enum {
    PID_SET_UP = 0X0D,
    PID_IN     = 0X09,
    PID_OUT    = 0X01,
    PID_ACK    = 0X02,
    PID_NAK    = 0X0A
} usb_pid_t;

typedef void (*HandlerFunc)(uint8_t pid, USB_Setup_Packet*);
extern volatile usb_state_t device_state;
extern const HandlerFunc jump_table[2][2];
extern volatile uint8_t usb_address;
extern volatile bool set_new_address_flag;
extern volatile bool is_device_configured;
extern volatile bool current_Led_A0_state;
extern volatile bool current_Led_A1_state;
extern volatile bool previous_Led_A0_state;
extern volatile bool previous_Led_A1_state;
extern volatile bool Leds_state_changed;
extern volatile bool is_set_address_status;	

void config_endpoint_gate(uint8_t ep_num, uint8_t flags);
void give_EP0_OUT_ownership_to_SIE(void);
void give_EP0_IN_ownership_to_SIE(void);
void give_EP1_OUT_ownership_to_SIE(void);
void give_EP1_IN_ownership_to_SIE(void);
void arm_endpoint_0_OUT(uint8_t count, uint16_t address);
void arm_endpoint_0_IN(uint8_t count, uint16_t address);
void arm_endpoint_1_OUT(uint8_t count, uint16_t address);
void arm_endpoint_1_IN(uint8_t count, uint16_t address);
void handle_get_protocol(USB_Setup_Packet* ctrl_pkt);
void handle_set_protocol(USB_Setup_Packet* ctrl_pkt);
void handle_get_report(USB_Setup_Packet* ctrl_pkt);
void handle_clear_feature(USB_Setup_Packet* ctrl_pkt);
void handle_set_feature(USB_Setup_Packet* ctrl_pkt);

void handle_set_idle(USB_Setup_Packet* ctrl_pkt);
void handle_set_configuration(USB_Setup_Packet* ctrl_pkt);
void handle_set_address(USB_Setup_Packet* ctrl_pkt);
void handle_get_descriptor(USB_Setup_Packet* ctrl_pkt);
void endpoint_0_OUT_set_up_handler(USB_Setup_Packet* ctrl_pkt);
void handle_endpoint_0_OUT(uint8_t pid, USB_Setup_Packet* ctrl_pkt);
void handle_endpoint_0_IN(uint8_t pid, USB_Setup_Packet* ctrl_pkt);
void handle_endpoint_1_OUT(uint8_t pid, USB_Setup_Packet* ctrl_pkt);
void handle_endpoint_1_IN(uint8_t pid, USB_Setup_Packet= ctrl_pkt);
void execute_handler(uint8_t x, uint8_t y, uint8_t pid);
void check_led_status(void);
void Prepare_BDT(void);
void USB_Initialize(void);
void Enable_USB(void);
void USB_Data_Copy(const uint8_t *source, uint8_t *destination, uint8_t length);
void USB_Remote_Wakeup_PC(void);
#ifndef VTPBUDDY_FIELDVALUES_H
#define VTPBUDDY_FIELDVALUES_H

// DSAP value used for VTP purposes (802.2 unnumbered PDU)
#define SNAP_DSAP_IDENTIFIER 0xAA
// SSAP value used for VTP purposes (802.2 unnumbered PDU)
#define SNAP_SSAP_IDENTIFIER 0xAA
// control field value we use (802.2 unnumbered PDU)
#define SNAP_CONTROL_VALUE 0x03

// put DSAP and SSAP those values together for easier comparation
#define SNAP_DSAP_SSAP_IDENTIFIER (SNAP_DSAP_IDENTIFIER << 8 | SNAP_SSAP_IDENTIFIER)

// VTP protocol identifier (SNAP field)
#define VTP_IDENTIFIER 0x2003

// OUI for Cisco used in SNAP extension field
const uint8_t OUI_Cisco[3] = {0x00, 0x00, 0x0C};

// TPID value used in tagged 802.1Q traffic
#define DOT1Q_VTP_TPID 0x8100
// PCP (Priority Code Point) used in tagged 802.1Q traffic
#define DOT1Q_VTP_PCP 0
// DEI (Drop Eligible Indicator) used in tagged 802.1Q traffic
#define DOT1Q_VTP_DEI 0

// Address used by ISL encapsulation
const uint8_t ISL_Dest_MAC[6] = {0x01, 0x00, 0x0c, 0x00, 0x00, 0x00};

// Address used in all encapsulations as destination (ISL uses this in encapsulated ethernet frame also)
const uint8_t VTP_Dest_MAC[6] = {0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc};

#endif

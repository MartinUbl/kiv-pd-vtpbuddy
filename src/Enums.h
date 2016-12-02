#ifndef VTPBUDDY_ENUMS_H
#define VTPBUDDY_ENUMS_H

// VTPBuddy operation mode, config option
enum OperationMode
{
    OM_SERVER       = 0,
    OM_CLIENT       = 1,
    OM_TRANSPARENT  = 2,
    MAX_OM
};

// VTP encapsulation option, config option
enum VTPEncapsulation
{
    ENCAPS_ISL      = 0,        // Inter-Switch Link protocol
    ENCAPS_DOT1Q    = 1,        // 802.1Q protocol
};

#endif

#ifndef VTPBUDDY_ENUMS_H
#define VTPBUDDY_ENUMS_H

enum OperationMode
{
    OM_SERVER       = 0,
    OM_CLIENT       = 1,
    OM_TRANSPARENT  = 2,
    MAX_OM
};

enum VTPEncapsulation
{
    ENCAPS_ISL      = 0,        // Inter-Switch Link protocol
    ENCAPS_DOT1Q    = 1,        // 802.1Q protocol
};

#endif

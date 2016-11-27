/*
 * PacketReceiving.hpp
 *
 *  Created on: Dec 6, 2014
 *      Author: tomas
 */

#ifndef PACKETRECEIVING_HPP_
#define PACKETRECEIVING_HPP_

#include "SummaryAdvertPacket.h"
#include "SubsetAdvertPacket.h"
#include "AdvertRequestPacket.h"

namespace VTP3
{
    struct summary_advert_mask
    {
        uint8_t version;
        uint8_t code;
        uint8_t followers;
        uint8_t domain_len;
        char    domain_name[32];
        uint8_t revision_nr[4];
        uint8_t updater_id[4];
        uint8_t update_timestamp[12];
        uint8_t md5_digest[16];
    };

    struct subset_advert_mask
    {
        uint8_t version;
        uint8_t code;
        uint8_t sequence_number;
        uint8_t domain_length;
        char    domain_name[32];
        uint8_t revision_nr[4];
    };

    struct vlan_field_mask
    {
        uint8_t len;
        uint8_t status;
        uint8_t type;
        uint8_t name_len;
        uint8_t vlan_id[2];
        uint8_t mtu_size[2];
        uint8_t index80211[4];
    };

    struct advert_request_mask
    {
        uint8_t version;
        uint8_t code;
        uint8_t reserved;
        uint8_t domain_len;
        char    domain_name[32];
        uint8_t start_value[4];
    };

    struct sniffing_data
    {
        char *dev_name;
        void (*summary_advert_recv)(SummaryAdvertPacket *);
        void (*subset_advert_recv)(SubsetAdvertPacket *, std::vector<std::shared_ptr<VlanInfo>>);
        void (*advert_request_recv)(AdvertRequestPacket *);
        int sockfd;
    };

    void process_advert_request_pkt(int, const u_char *);
    void process_subset_advert_pkt(uint16_t, int, const u_char *);
    void process_summary_advert_pkt(int, u_char *);
    void *pkt_receiving(struct sniffing_data *);

    void got_packet(const u_char *packet);
    void set_sock_timeout(int sockfd);
    void stop_pkt_receiving();
}

#endif /* PACKETRECEIVING_HPP_ */

//szymon_pielat 308859
#include <iostream>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include "utility.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <bits/stdint-uintn.h>
#include <string.h>

#define TIME_WAIT 1000

using namespace std;


struct PACKETS
{
    int amount;
    int order;
    bool addr_unique;
    long long time;
    string sender_addr[3];
};

icmp create_header(u_int16_t seq, u_int16_t pid)
{
    struct icmp header;
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_hun.ih_idseq.icd_id = pid;
    header.icmp_hun.ih_idseq.icd_seq = seq;
    header.icmp_cksum = 0;
    header.icmp_cksum = compute_icmp_checksum((u_int16_t *)&header, sizeof(header));

    return header;
}

void send_packet(string ip_addr, int sockfd, int ttl, uint16_t pid, uint16_t seq)
{

    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr.c_str(), &recipient.sin_addr);

    struct icmp header = create_header(seq, pid);

    int succ = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

    if (succ != 0)
    {
        throw runtime_error("setsockopt");
    }

    ssize_t bytes_sent = sendto(
        sockfd,
        &header,
        sizeof(header),
        0,
        (struct sockaddr *)&recipient,
        sizeof(recipient));

    if (bytes_sent <= 0)
    {
        throw runtime_error("sendto");
    }
}

PACKETS ping(uint16_t ttl, int sockfd, uint16_t pid, string ip_addr)
{

    PACKETS recived_packets;
    recived_packets.amount = 0;
    recived_packets.order = ttl;
    recived_packets.addr_unique = true;
    recived_packets.time = 0;
    recived_packets.sender_addr[0] = "";
    recived_packets.sender_addr[1] = "";
    recived_packets.sender_addr[2] = "";

    for (int i = 0; i < 3; i++)
    {
        uint16_t seq = (ttl << 2) + i;
        send_packet(ip_addr, sockfd, ttl, pid, seq);
    }
    long long time_out = current_timestamp();

    while (recived_packets.amount < 3 && recived_packets.time < TIME_WAIT)
    {
        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(sockfd, &descriptors);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ready = select(sockfd + 1, &descriptors, NULL, NULL, &tv);

        if (ready < 0)
        {
            throw runtime_error("select");
        }
        else if (ready == 0)
        {
            return recived_packets;
        }

        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);
        u_int8_t buffer[IP_MAXPACKET];
        ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&sender, &sender_len);

        if (packet_len < 0)
        {
            throw runtime_error("recvfrom");
        }
        long long time_came = current_timestamp();
        char sender_ip_str[20];
        inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));

        struct ip *ip_header = (struct ip *)buffer;
        ssize_t ip_header_len = 4 * ip_header->ip_hl;
        u_int8_t *icmp_packet = buffer + ip_header_len;
        struct icmp *icmp_header = (struct icmp *)icmp_packet;
        ssize_t icmp_header_len = 8;

        if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED)
        {
            ssize_t inside_packet_offset = ip_header_len + icmp_header_len;
            ip_header = (struct ip *)(buffer + inside_packet_offset);
            icmp_packet = buffer + inside_packet_offset + ip_header_len;
            icmp_header = (struct icmp *)icmp_packet;
        }

        uint16_t p_id = icmp_header->icmp_hun.ih_idseq.icd_id;
        uint16_t p_seq = icmp_header->icmp_hun.ih_idseq.icd_seq;

        uint16_t order = p_seq >> 2;

        if (p_id == pid && order == ttl)
        {
            if (recived_packets.amount >= 1 && recived_packets.sender_addr[0].compare(sender_ip_str) == 0)
                recived_packets.addr_unique = false;
            else
                recived_packets.sender_addr[recived_packets.amount] = sender_ip_str;
            recived_packets.time += time_came - time_out;
            recived_packets.amount++;
        }
    }
    return recived_packets;
}

int main(int argc, char **argv)
{
    try
    {
        if (argc < 2)
        {
            throw invalid_argument("Brak adresu IP");
        }
        if (!validate_ip_address(argv[1]))
        {
            throw invalid_argument("Niepoprawny addres IP");
        }
        const uint16_t pid = getpid();
        int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

        if (sockfd < 0)
        {
            throw runtime_error("socket");
        }
        string ip_addr = argv[1];

        for (int ttl = 1; ttl <= 30; ttl++)
        {
            cout << ttl << ". ";
            PACKETS response = ping(ttl, sockfd, pid, ip_addr);
            if (response.amount == 0)
            {
                cout << "*\n";
            }
            else
            {
                if (response.addr_unique == false)
                    cout << response.sender_addr[0];
                else
                    cout << response.sender_addr[0] << " " << response.sender_addr[1] << " " << response.sender_addr[2];

                if (response.amount < 3)
                {
                    cout << " ???\n";
                }
                else
                {
                    cout << " " << response.time / 3 << "ms\n";
                }
            }
            if (response.sender_addr[0] == ip_addr)
            {
                return 0;
            }
        }
    }
    catch (const exception &e)
    {
        cerr << "error: " << e.what() << "\n";
    }
    return 0;
}
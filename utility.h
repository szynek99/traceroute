#include <iostream>


#ifndef UTILITY_H
#define UTILITY_H



u_int16_t compute_icmp_checksum (const void *buff, int length);

bool validate_ip_address(const std::string &ipAddress);
void print_as_bytes (unsigned char* buff, ssize_t length);
long long current_timestamp();
#endif
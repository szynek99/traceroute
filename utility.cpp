#include <iostream>
#include <cassert>
#include <sys/time.h>
#include <arpa/inet.h>
#include <vector>
#include <string>

u_int16_t compute_icmp_checksum(const void *buff, int length)
{
	u_int32_t sum;
	const u_int16_t *ptr = (const u_int16_t *)buff;
	assert(length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}

long long current_timestamp()
{
	struct timeval te;
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
	return milliseconds;
}
bool validate_ip_address(const std::string &ipAddress)
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
	return result != 0;
}
void print_as_bytes(unsigned char *buff, ssize_t length)
{
	for (ssize_t i = 0; i < length; i++, buff++)
		printf("%.2x ", *buff);
}
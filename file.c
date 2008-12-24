/*
 * file.c
 *
 * Version:	0.2.6,	08/12/2008 [dd/mm/yyyy]
 * (C) 2007,2008, BlackLight <blacklight86@gmail.com>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		3 of the License, or (at your option) any later version.
 */

#include "tcpsmash.h"

void file_dump (char* file)  {
	struct ethhdr *eth = (struct ethhdr*) malloc(sizeof(struct ethhdr));
	struct iphdr  *ip  = (struct iphdr*)  malloc(sizeof(struct iphdr));
	struct arphdr_t *arp = (struct arphdr_t*) malloc(sizeof(struct arphdr_t));
	struct pcap_pkthdr pcap;
	struct timeval tv;

	FILE *fp;
	int len;
	char *buff;

	if (!(fp=fopen(file,"rb")))  {
		fprintf (stderr,"%s*** Error: Unable to read from %s: %s ***%s\n",
				RED, file, strerror(errno), NORMAL);
		exit(1);
	}

	while (!feof(fp))  {
		buff = NULL;
		len=0;

		fread (&tv, sizeof(tv), 1, fp);
		fread (eth, sizeof(struct ethhdr), 1, fp);
		len += sizeof(struct ethhdr);
		buff = (char*) realloc(buff,len);
		memcpy (buff, eth, sizeof(struct ethhdr));
		
		if (eth->h_proto == ntohs(ETH_P_ARP))  {
			fread (arp, sizeof(struct arphdr_t), 1, fp);
			len += sizeof(struct arphdr_t);
			buff = (char*) realloc(buff,len);
			memcpy (buff+sizeof(struct ethhdr), arp, sizeof(struct arphdr_t));
		} else if (eth->h_proto == ntohs(ETH_P_IP))  {
			fread (ip, sizeof(struct iphdr), 1, fp);
			len += sizeof(struct iphdr);
			buff = (char*) realloc(buff,len);
			memcpy (buff+sizeof(struct ethhdr), ip, sizeof(struct iphdr));

			len += ( ntohs(ip->tot_len) - sizeof(struct iphdr) );
			buff = (char*) realloc(buff,len);

			fread (
				buff + sizeof(struct ethhdr) + sizeof(struct iphdr),
				(htons(ip->tot_len) - sizeof(struct iphdr)),
				1,
				fp
			);
		}

		memset (&pcap, 0x0, sizeof(pcap));
		pcap.ts = tv;
		pcap.caplen = len;
		pcap.len = len;
		pack_handle(NULL,&pcap,(u8*) buff);
		free(buff);
	}

	return;
}


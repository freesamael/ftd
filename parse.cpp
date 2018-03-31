#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <pcap/pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

using namespace std;

struct FTD
{
  uint8_t type;
  uint8_t exthdr_len;
  uint16_t data_len;
};

#define FTD_TYPE_NONE       (0x0)
#define FTD_TYPE_FTDC       (0x1)
#define FTD_TYPE_COMPRESSED (0x2)

struct FTDCBase
{
  uint8_t ver;
  uint8_t type;
  uint8_t unknown;
  char chain;
};

#define FTDC_TYPE_REQ (0x0)
#define FTDC_TYPE_RES (0x3)

struct FTDCDetail
{
  uint16_t seq_series;
  // Split 32bits tx_id to 2 16bits data to prevent alignment issue
  uint16_t tx_id_l;
  uint16_t tx_id_r;
  // Split 32bits seq_no to 2 16bits data to prevent alignment issue
  uint16_t seq_no_l;
  uint16_t seq_no_r;
  uint16_t field_count;
  uint16_t content_len;
  // Split 32bits seq_no to 2 16bits data to prevent alignment issue
  uint16_t req_id_l;
  uint16_t req_id_r;
};

struct Field
{
  uint16_t type;
  uint16_t len;
};

#define FIELD_TYPE_ORDER_FIELD (0x2508)

void pcapCallback(uint8_t *user, const struct pcap_pkthdr *h,
  const uint8_t *bytes);
void handleFTD(const FTD* ftd, const uint8_t* ftd_content_bytes);
void handleField(const Field* field, const uint8_t* bytes);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s input.pcap\n", argv[0]);
  }

  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* handle = pcap_open_offline(argv[1], errbuf);
  if (!handle) {
    fprintf(stderr, "pcap_open_offline: %s\n", errbuf);
    return EXIT_FAILURE;
  }
  if (!pcap_loop(handle, -1, &pcapCallback, nullptr)) {
    pcap_perror(handle, "pcap_loop: ");
  }
  pcap_close(handle);
  return 0;
}

/**
 * Handle a pcap packet.
 */
void pcapCallback(uint8_t *user, const struct pcap_pkthdr *h,
  const uint8_t *bytes) {

  // Extract Ethernet and IP headers.
  const ether_header* ethhdr = (const ether_header*) bytes;
  const ip* iphdr = (const ip*)(bytes + ETHER_HDR_LEN);

  // We don't care about non-TCP and loopback packets.
  if (iphdr->ip_p != IPPROTO_TCP ||
      ntohl(iphdr->ip_src.s_addr) == INADDR_LOOPBACK ||
      ntohl(iphdr->ip_dst.s_addr) == INADDR_LOOPBACK) {
    return;
  }

  size_t payload_offset = ETHER_HDR_LEN + sizeof(ip) + sizeof(tcphdr);
  size_t payload_size = ntohs(iphdr->ip_len) - (sizeof(ip) + sizeof(tcphdr));

  // Extract FTD packets from the TCP payload.
  const uint8_t* payload = bytes + payload_offset;
  const uint8_t* current = payload;
  while (current < (payload + payload_size)) {
#ifdef DEBUG
    // Sanity check.
    assert(current + sizeof(FTD) <= payload + payload_size);
#endif
    const FTD* ftdhdr = (const FTD*)current;
    handleFTD(ftdhdr, current + sizeof(FTD) + ftdhdr->exthdr_len);
    current += sizeof(FTD) + ftdhdr->exthdr_len + ntohs(ftdhdr->data_len);
  }

#ifdef DEBUG
  // Sanity check.
  assert(current == payload + payload_size);
#endif
}

/**
 * Process a FTD packet.
 */
void handleFTD(const FTD* ftd, const uint8_t* ftd_content_bytes) {
  size_t ftd_content_bytes_len = ntohs(ftd->data_len);
  if (!ftd_content_bytes_len) { /* No data to process. */ return; }

  const FTDCBase* ftdcb = (const FTDCBase*) ftd_content_bytes;
  // We're only interested in responses, and we don't support chains.
  if (ftdcb->type != FTDC_TYPE_RES) {
    return;
  }

  // Bytes after FTDCBase can be compressed, so we may need to decompress it.
  const uint8_t* ftdc_base_data = ftd_content_bytes + sizeof(FTDCBase);
  size_t ftdc_base_data_len = ftd_content_bytes_len - sizeof(FTDCBase);
  vector<uint8_t> decompress;
  if (ftd->type == FTD_TYPE_COMPRESSED) {
    for (size_t i = sizeof(FTDCBase); i < ftd_content_bytes_len; i++) {
      if (ftd_content_bytes[i] == 0xe0) {
        decompress.push_back(ftd_content_bytes[++i]);
      } else if (ftd_content_bytes[i] > 0xe0 && ftd_content_bytes[i] <= 0xef) {
        for (int z = 0; z < (ftd_content_bytes[i] - 0xe0); z++) {
          decompress.push_back(0);
        }
      } else {
        decompress.push_back(ftd_content_bytes[i]);
      }
    }

    ftdc_base_data_len = decompress.size();
    ftdc_base_data = decompress.data();
  }

  const FTDCDetail* ftdcd = (const FTDCDetail*)ftdc_base_data;
#ifdef DEBUG
  printf("Raw bytes: ");
  for (size_t i = sizeof(FTDCBase); i < ftd_content_bytes_len; i++) {
    printf("%02x", ftd_content_bytes[i]);
  }
  printf("\n");
  printf("Dec bytes: ");
  for (size_t i = 0; i < ftdc_base_data_len; i++) {
    printf("%02x", ftdc_base_data[i]);
  }
  printf("\n");
  printf("FTD:\n"
         "  type=0x%02x\n"
         "  exthdr_len=%u\n"
         "  data_len=%u\n"
         "  FTDC:\n"
         "    ver=%u\n"
         "    type=0x%02x\n"
         "    chain=%c\n"
         "    seq_series=0x%04x\n"
         "    tx_id=0x%08x\n"
         "    seq_no=0x%08x\n"
         "    field_count=%u\n"
         "    content_len=%u\n"
         "    req_id=0x%08x\n",
         ftd->type, ftd->exthdr_len, ntohs(ftd->data_len),
         ftdcb->ver, ftdcb->type, ftdcb->chain,
         ntohs(ftdcd->seq_series),
         ntohl(((uint32_t)ftdcd->tx_id_r << 16) + ftdcd->tx_id_l),
         ntohl(((uint32_t)ftdcd->seq_no_r << 16) + ftdcd->seq_no_l),
         ntohs(ftdcd->field_count), ntohs(ftdcd->content_len),
         ntohl(((uint32_t)ftdcd->req_id_r << 16) + ftdcd->req_id_l));
#endif
  if (!ftdcd->content_len) { /* No data to process. */ return; }

  const uint8_t* msg = ftdc_base_data + sizeof(FTDCDetail);
  const uint8_t* unhandled_msg = msg;
  uint16_t field_count = 0;
  while (unhandled_msg < msg + ntohs(ftdcd->content_len)) {
    field_count++;
    const Field* field = (const Field*) unhandled_msg;
    const uint8_t* field_bytes = unhandled_msg + sizeof(Field);
    handleField(field, field_bytes);
    unhandled_msg = field_bytes + ntohs(field->len);
  }

#ifdef DEBUG
  // Sanity check.
  assert(unhandled_msg == msg + ntohs(ftdcd->content_len));
  assert(field_count == ntohs(ftdcd->field_count));
#endif
}

/**
 * Handle a FTDC field.
 */
void handleField(const Field* field, const uint8_t* bytes) {
#ifdef DEBUG
  printf("    Field:\n"
         "      type=%04x\n"
         "      len=%u\n",
         ntohs(field->type), ntohs(field->len));
#endif

  // We only care about FIELD_TYPE_ORDER_FIELD
  if (ntohs(field->type) != FIELD_TYPE_ORDER_FIELD) { return; }
  const char* tradingDay = (const char*)bytes;
  const char* settlementGroupID = (const char*)(bytes + 9);
  const char* participantID = (const char*)(bytes + 35);
  const char* clientID = (const char*)(bytes + 46);
  const char* userID = (const char*)(bytes + 57);
  const char* instrumentID = (const char*)(bytes + 73);
  const char direction = *(const char*)(bytes + 105);
  uint64_t limitPriceStorage = ntohll(*(uint64_t*)(bytes + 116));
  double limitPrice = *((double*)&limitPriceStorage);
  uint32_t volume = ntohl(*(uint32_t*)(bytes + 124));
  const char* orderLocalID = (const char*)(bytes + 153);
  const char orderStatus = *(const char*)(bytes + 171);
#ifdef DEBUG
  printf("      tradingDay=%s\n"
         "      settlementGroupID=%s\n"
         "      participantID=%s\n"
         "      clientID=%s\n"
         "      userID=%s\n"
         "      instrumentID=%s\n"
         "      direction=%c\n"
         "      limitPrice=%lf\n"
         "      volume=%08x\n"
         "      orderLocalID=%s\n"
         "      orderStatus=%c\n"
         ,
         tradingDay, settlementGroupID, participantID, clientID, userID,
         instrumentID, direction, limitPrice, volume, orderLocalID, orderStatus
         );
#endif
}

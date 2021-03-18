//SPI MASTER (ARDUINO)

/* testing checklist:
    - signal voltages don't exceed 3.3V
    - wires are connected to correct ports
    - wires do not come into contact (short)
*/

/* issues:
 * - CRC is producing arbitrary changing values on payload array
 * - need to split SPI mechanism from payload processing
 */

#include <SPI.h>

// assigns GPIO pins to be SS pins
#define SS1 3
#define SS2 4
#define SS3 5
#define SS4 6

// assigns GPIO pins to be EN pins
#define EN1 A1
#define EN2 A2
#define EN3 A3
#define EN4 A4

uint8_t req_array_A[16] = {0}; // need to allocate max possible size of uint8_t
uint8_t req_array_B[16] = {0};
uint8_t req_array_C[16] = {0};
uint8_t req_array_D[16] = {0};
uint8_t req_array_E[16] = {0};
uint8_t req_len_A;
uint8_t req_len_B;
uint8_t req_len_C;
uint8_t req_len_D;
uint8_t req_len_E;

uint8_t rpl_array_Z[16] = {0}; // need to allocate max possible size of uint8_t
uint8_t rpl_array_Y[16] = {0};
uint8_t rpl_array_X[16] = {0};
uint8_t rpl_array_W[16] = {0};
uint8_t rpl_len_Z;
uint8_t rpl_len_Y;
uint8_t rpl_len_X;
uint8_t rpl_len_W;

uint8_t req_payload_rw1[7] = {0};
uint8_t req_payload_len_rw1 = 0;
uint8_t req_packet_rw1[14] = {0};
uint8_t req_packet_len_rw1 = 0;

uint8_t rpl_payload_rw1[7] = {0};
uint8_t rpl_payload_len_rw1 = 0;
uint8_t rpl_packet_rw1[14] = {0};
uint8_t rpl_packet_len_rw1 = 0;

uint8_t result;


unsigned int crc_value = 0xFFFF;
unsigned int crc_table[] = {0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
                            0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
                            0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
                            0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
                            0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
                            0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
                            0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
                            0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
                            0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
                            0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
                            0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
                            0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
                            0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
                            0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
                            0x7E97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
                            0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
                            0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
                            0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
                            0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
                            0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
                            0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
                            0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
                            0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
                            0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
                            0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
                            0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
                            0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
                            0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
                            0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
                            0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
                            0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
                            0x6e17, 0x7E36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
                           };

SPISettings spiSet(200000, MSBFIRST, SPI_MODE0);

#include "rwa_command_functions.h"
#include "rwa_process_functions.h"
  
void setup (void) {
  Serial.begin(115200);

  // pulls SS1 HIGH (disengaged)
  SPI.begin();
  pinMode(SS1, OUTPUT);
  digitalWrite(SS1, HIGH);

  // powers on RW0-1
  pinMode(EN1, OUTPUT);
  digitalWrite(EN1, HIGH);

  Serial.println("master setup complete");
}


void loop(void) {
  Serial.println(" ");
  Serial.println("loop start");

  // example process
  // RW0-1, sending cmd10 ping
  
    // generates payload array
  reqPayloadWrite_cmd10(&req_payload_rw1[0], &req_payload_len_rw1);
    // process payload into packet
  reqPacketProcess(&req_payload_rw1[0], &req_packet_rw1[0], &req_payload_len_rw1, &req_packet_len_rw1);
    // sends Request to RW0-1, receives empty flag bytes back
  reqSpiTransfer(&req_packet_rw1[0], &req_packet_len_rw1, SS1);

    // SPI wait timeout (ms)
  delay(100);

    // queries Reply from RWO-1 with empty flag bytes, receives packet back
  rplSpiTransfer(&rpl_packet_rw1[0], &rpl_packet_len_rw1, SS1);
    // process packet into payload
  rplPacketProcess(&rpl_payload_rw1[0], &rpl_packet_rw1[0], &rpl_payload_len_rw1, &rpl_packet_len_rw1);
    // convert payload to data
  rplPayloadRead_cmd10(&rpl_payload_rw1[0], &rpl_payload_len_rw1, &result);

          Serial.print("req_packet_rw1:\t");
          for (uint8_t yy = 0; yy < req_packet_len_rw1; yy++) {
            Serial.print(req_packet_rw1[yy], HEX);
            Serial.print("\t");
          }
          Serial.println(" ");

          Serial.print("rpl_packet_rw1:\t");
          for (uint8_t yy = 0; yy < rpl_packet_len_rw1; yy++) {
            Serial.print(rpl_packet_rw1[yy], HEX);
            Serial.print("\t");
          }
          Serial.println(" ");

  delay(10000);
}

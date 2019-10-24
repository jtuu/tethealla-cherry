#ifndef TETHEALLA_PSO_CRYPT_H
#define TETHEALLA_PSO_CRYPT_H

#include <stdint.h>

#define TABLE_SIZE (1024 + 18)

#define PSO_CRYPT_TYPE_DC 0
#define PSO_CRYPT_TYPE_GC 1
#define PSO_CRYPT_TYPE_BB 2

typedef struct pso_crypt {
  uint32_t tbl[TABLE_SIZE];
  int32_t type;
  int32_t cur;
  int32_t size;
  void (*mangle)(struct pso_crypt *);
} PSO_CRYPT;

/* for DC */
void pso_crypt_table_init_dc(PSO_CRYPT *pcry, const uint8_t *salt);

/* for GC */
void pso_crypt_table_init_gc(PSO_CRYPT *pcry, const uint8_t *salt);

/* for BB */
void pso_crypt_table_init_bb(PSO_CRYPT *pcry, const uint8_t *salt);
void pso_crypt_decrypt_bb(PSO_CRYPT *pcry, uint8_t *data, unsigned
  length);
void pso_crypt_encrypt_bb(PSO_CRYPT *pcry, uint8_t *data, unsigned
  length);
void pso_crypt_init_key_bb(uint8_t *data);
void RleDecode(uint8_t *src, uint8_t *dest, uint32_t src_size);
uint32_t RleEncode(uint8_t *src, uint8_t *dest, uint32_t src_size);

/* common */

/* a RC4 expanded key session */
#define RC4publicKey (uint8_t []) { \
  103, 196, 247, 176, 71, 167, 89, 233, 200, 100, 044, 209, 190, 231, 83, 42, \
  6, 95, 151, 28, 140, 243, 130, 61, 107, 234, 243, 172, 77, 24, 229, 156 \
}

struct rc4_key {
    uint8_t state[256];
    uint32_t x, y;
};

void pso_crypt_init(PSO_CRYPT *pcry, const uint8_t *salt, int32_t type);
void pso_crypt(PSO_CRYPT *pcry, uint8_t *data, int32_t len, int32_t enc);
uint32_t pso_crypt_get_num(PSO_CRYPT *pcry);
void rc4(uint8_t *buffer, uint32_t len, struct rc4_key *key);
void prepare_key(uint8_t *keydata, uint32_t len, struct rc4_key *key);

#endif

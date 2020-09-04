/*
 * Copyright (c) 2019 Siddharth Chandrasekaran <siddharth@embedjournal.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OSDP_COMMON_H_
#define _OSDP_COMMON_H_

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#include <utils/utils.h>
#include <utils/queue.h>
#include <utils/slab.h>

#include <osdp.h>
#include "osdp_config.h"  /* generated */
#include "osdp_export.h"  /* generated */

#ifndef NULL
#define NULL                           ((void *)0)
#endif

#define ARG_UNUSED(x)                  (void)(x)

#define ISSET_FLAG(p, f)               (((p)->flags & (f)) == (f))
#define SET_FLAG(p, f)                 ((p)->flags |= (f))
#define CLEAR_FLAG(p, f)               ((p)->flags &= ~(f))

#define BYTE_0(x)                      (uint8_t)(((x) >>  0) & 0xFF)
#define BYTE_1(x)                      (uint8_t)(((x) >>  8) & 0xFF)
#define BYTE_2(x)                      (uint8_t)(((x) >> 16) & 0xFF)
#define BYTE_3(x)                      (uint8_t)(((x) >> 24) & 0xFF)

/* casting helpers */
#define TO_OSDP(p)                     ((struct osdp *)p)
#define TO_CP(p)                       (((struct osdp *)(p))->cp)
#define TO_PD(p, i)                    (((struct osdp *)(p))->pd + i)
#define TO_CTX(p)                      ((struct osdp *)p->__parent)

#define GET_CURRENT_PD(p)              (TO_CP(p)->current_pd)
#define SET_CURRENT_PD(p, i)                                    \
	do {                                                    \
		TO_CP(p)->current_pd = TO_PD(p, i);             \
		TO_CP(p)->pd_offset = i;                        \
	} while (0)
#define PD_MASK(ctx) \
	(uint32_t)((1 << (TO_CP(ctx)->num_pd)) - 1)
#define AES_PAD_LEN(x)                 ((x + 16 - 1) & (~(16 - 1)))
#define NUM_PD(ctx)                    (TO_CP(ctx)->num_pd)
#define OSDP_COMMAND_DATA_MAX_LEN      sizeof(struct osdp_cmd)

/**
 * @brief OSDP reserved commands
 */
#define CMD_POLL                0x60
#define CMD_ID                  0x61
#define CMD_CAP                 0x62
#define CMD_DIAG                0x63
#define CMD_LSTAT               0x64
#define CMD_ISTAT               0x65
#define CMD_OSTAT               0x66
#define CMD_RSTAT               0x67
#define CMD_OUT                 0x68
#define CMD_LED                 0x69
#define CMD_BUZ                 0x6A
#define CMD_TEXT                0x6B
#define CMD_RMODE               0x6C
#define CMD_TDSET               0x6D
#define CMD_COMSET              0x6E
#define CMD_DATA                0x6F
#define CMD_XMIT                0x70
#define CMD_PROMPT              0x71
#define CMD_SPE                 0x72
#define CMD_BIOREAD             0x73
#define CMD_BIOMATCH            0x74
#define CMD_KEYSET              0x75
#define CMD_CHLNG               0x76
#define CMD_SCRYPT              0x77
#define CMD_CONT                0x79
#define CMD_ABORT               0x7A
#define CMD_MAXREPLY            0x7B
#define CMD_MFG                 0x80
#define CMD_SCDONE              0xA0
#define CMD_XWR                 0xA1

/**
 * @brief OSDP reserved responses
 */
#define REPLY_ACK               0x40
#define REPLY_NAK               0x41
#define REPLY_PDID              0x45
#define REPLY_PDCAP             0x46
#define REPLY_LSTATR            0x48
#define REPLY_ISTATR            0x49
#define REPLY_OSTATR            0x4A
#define REPLY_RSTATR            0x4B
#define REPLY_RAW               0x50
#define REPLY_FMT               0x51
#define REPLY_PRES              0x52
#define REPLY_KEYPPAD           0x53
#define REPLY_COM               0x54
#define REPLY_SCREP             0x55
#define REPLY_SPER              0x56
#define REPLY_BIOREADR          0x57
#define REPLY_BIOMATCHR         0x58
#define REPLY_CCRYPT            0x76
#define REPLY_RMAC_I            0x78
#define REPLY_MFGREP            0x90
#define REPLY_BUSY              0x79
#define REPLY_XRD               0xB1

/**
 * @brief secure block types
 */
#define SCS_11                  0x11    /* CP -> PD -- CMD_CHLNG */
#define SCS_12                  0x12    /* PD -> CP -- REPLY_CCRYPT */
#define SCS_13                  0x13    /* CP -> PD -- CMD_SCRYPT */
#define SCS_14                  0x14    /* PD -> CP -- REPLY_RMAC_I */

#define SCS_15                  0x15    /* CP -> PD -- packets w MAC w/o ENC */
#define SCS_16                  0x16    /* PD -> CP -- packets w MAC w/o ENC */
#define SCS_17                  0x17    /* CP -> PD -- packets w MAC w ENC*/
#define SCS_18                  0x18    /* PD -> CP -- packets w MAC w ENC*/

/* Global flags */
#define FLAG_CP_MODE		0x00000001 /* Set when initialized as CP */

/* PD Flags */
#define PD_FLAG_SC_CAPABLE	0x00000001 /* PD secure channel capable */
#define PD_FLAG_TAMPER		0x00000002 /* local tamper status */
#define PD_FLAG_POWER		0x00000004 /* local power status */
#define PD_FLAG_R_TAMPER	0x00000008 /* remote tamper status */
#define PD_FLAG_AWAIT_RESP	0x00000020 /* set after command is sent */
#define PD_FLAG_SKIP_SEQ_CHECK	0x00000040 /* disable seq checks (debug) */
#define PD_FLAG_SC_USE_SCBKD	0x00000080 /* in this SC attempt, use SCBKD */
#define PD_FLAG_SC_ACTIVE	0x00000100 /* secure channel is active */
#define PD_FLAG_SC_SCBKD_DONE	0x00000200 /* indicated that SCBKD check is done */
#define PD_FLAG_INSTALL_MODE	0x40000000 /* PD is in install mode */
#define PD_FLAG_PD_MODE		0x80000000 /* device is setup as PD */

/* logging short hands */
#define LOG_EM(...)	(osdp_log(LOG_EMERG, __VA_ARGS__))
#define LOG_ALERT(...)	(osdp_log(LOG_ALERT, __VA_ARGS__))
#define LOG_CRIT(...)	(osdp_log(LOG_CRIT, __VA_ARGS__))
#define LOG_ERR(...)	(osdp_log(LOG_ERR, __VA_ARGS__))
#define LOG_INF(...)	(osdp_log(LOG_INFO, __VA_ARGS__))
#define LOG_WRN(...)	(osdp_log(LOG_WARNING, __VA_ARGS__))
#define LOG_NOT(...)	(osdp_log(LOG_NOTICE, __VA_ARGS__))
#define LOG_DBG(...)	(osdp_log(LOG_DEBUG, __VA_ARGS__))

enum osdp_pd_nak_code_e {
	/**
	 * @brief Dummy
	 */
	OSDP_PD_NAK_NONE,
	/**
	 * @brief Message check character(s) error (bad cksum/crc)
	 */
	OSDP_PD_NAK_MSG_CHK,
	/**
	 * @brief Command length error
	 */
	OSDP_PD_NAK_CMD_LEN,
	/**
	 * @brief Unknown Command Code – Command not implemented by PD
	 */
	OSDP_PD_NAK_CMD_UNKNOWN,
	/**
	 * @brief Unexpected sequence number detected in the header
	 */
	OSDP_PD_NAK_SEQ_NUM,
	/**
	 * @brief Unexpected sequence number detected in the header
	 */
	OSDP_PD_NAK_SC_UNSUP,
	/**
	 * @brief unsupported security block or security conditions not met
	 */
	OSDP_PD_NAK_SC_COND,
	/**
	 * @brief BIO_TYPE not supported
	 */
	OSDP_PD_NAK_BIO_TYPE,
	/**
	 * @brief BIO_FORMAT not supported
	 */
	OSDP_PD_NAK_BIO_FMT,
	/**
	 * @brief Unable to process command record
	 */
	OSDP_PD_NAK_RECORD,
	/**
	 * @brief Dummy
	 */
	OSDP_PD_NAK_SENTINEL
};

enum osdp_pd_state_e {
	OSDP_PD_STATE_IDLE,
	OSDP_PD_STATE_SEND_REPLY,
	OSDP_PD_STATE_ERR,
};

enum osdp_cp_phy_state_e {
	OSDP_CP_PHY_STATE_IDLE,
	OSDP_CP_PHY_STATE_SEND_CMD,
	OSDP_CP_PHY_STATE_REPLY_WAIT,
	OSDP_CP_PHY_STATE_WAIT,
	OSDP_CP_PHY_STATE_ERR,
	OSDP_CP_PHY_STATE_ERR_WAIT,
	OSDP_CP_PHY_STATE_CLEANUP,
};

enum osdp_state_e {
	OSDP_CP_STATE_INIT,
	OSDP_CP_STATE_IDREQ,
	OSDP_CP_STATE_CAPDET,
	OSDP_CP_STATE_SC_INIT,
	OSDP_CP_STATE_SC_CHLNG,
	OSDP_CP_STATE_SC_SCRYPT,
	OSDP_CP_STATE_SET_SCBK,
	OSDP_CP_STATE_ONLINE,
	OSDP_CP_STATE_OFFLINE
};

enum osdp_pkt_errors_e {
	OSDP_ERR_PKT_FMT   = -1,
	OSDP_ERR_PKT_WAIT  = -2,
	OSDP_ERR_PKT_SKIP  = -3
};

struct osdp_slab {
	int block_size;
	int num_blocks;
	int free_blocks;
	uint8_t *blob;
};

struct osdp_notifiers {
	void *data;
	keypress_callback_t keypress;
	cardread_callback_t cardread;
	pd_command_callback_t  command_handler;
};

#ifdef CONFIG_OSDP_SC_ENABLED
struct osdp_secure_channel {
	uint8_t scbk[16];
	uint8_t s_enc[16];
	uint8_t s_mac1[16];
	uint8_t s_mac2[16];
	uint8_t r_mac[16];
	uint8_t c_mac[16];
	uint8_t cp_random[8];
	uint8_t pd_random[8];
	uint8_t pd_client_uid[8];
	uint8_t cp_cryptogram[16];
	uint8_t pd_cryptogram[16];
};
#endif

struct osdp_cmd_queue {
	queue_t queue;
	slab_t slab;
};

struct osdp_pd {
	void *__parent;
	int offset;
	uint32_t flags;

	/* OSDP specified data */
	int baud_rate;
	int address;
	int seq_number;
	struct osdp_pd_cap cap[OSDP_PD_CAP_SENTINEL];
	struct osdp_pd_id id;

	/* PD state management */
	int state;
	int phy_state;

	int64_t tstamp;
	int64_t sc_tstamp;
	uint8_t rx_buf[OSDP_PACKET_BUF_SIZE];
	int rx_buf_len;
	int64_t phy_tstamp;

	int cmd_id;
	int reply_id;
	uint8_t cmd_data[OSDP_COMMAND_DATA_MAX_LEN];

	struct osdp_cmd_queue cmd;
	struct osdp_channel channel;
#ifdef CONFIG_OSDP_SC_ENABLED
	struct osdp_secure_channel sc;
#endif
	void *command_callback_arg;
	pd_command_callback_t command_callback;
};

struct osdp_cp {
	void *__parent;
	uint32_t flags;
	int num_pd;
	struct osdp_pd *current_pd;	/* current operational pd's pointer */
	int pd_offset;			/* current pd's offset into ctx->pd */
	void *event_callback_arg;
	cp_event_callback_t event_callback;
};

struct osdp {
	int magic;
	uint32_t flags;
	struct osdp_cp *cp;
	struct osdp_pd *pd;
#ifdef CONFIG_OSDP_SC_ENABLED
	uint8_t sc_master_key[16];
#endif
};

enum log_levels_e {
	LOG_EMERG,
	LOG_ALERT,
	LOG_CRIT,
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG,
	LOG_MAX_LEVEL
};

/* from osdp_phy.c */
int osdp_phy_packet_init(struct osdp_pd *p, uint8_t *buf, int max_len);
int osdp_phy_packet_finalize(struct osdp_pd *p, uint8_t *buf,
			       int len, int max_len);
int osdp_phy_decode_packet(struct osdp_pd *p, uint8_t *buf, int len);
void osdp_phy_state_reset(struct osdp_pd *pd);
int osdp_phy_packet_get_data_offset(struct osdp_pd *p, const uint8_t *buf);
uint8_t *osdp_phy_packet_get_smb(struct osdp_pd *p, const uint8_t *buf);

/* from osdp_sc.c */
void osdp_compute_scbk(struct osdp_pd *p, uint8_t *scbk);
void osdp_compute_session_keys(struct osdp *ctx);
void osdp_compute_cp_cryptogram(struct osdp_pd *p);
int osdp_verify_cp_cryptogram(struct osdp_pd *p);
void osdp_compute_pd_cryptogram(struct osdp_pd *p);
int osdp_verify_pd_cryptogram(struct osdp_pd *p);
void osdp_compute_rmac_i(struct osdp_pd *p);
int osdp_decrypt_data(struct osdp_pd *p, int is_cmd, uint8_t *data, int length);
int osdp_encrypt_data(struct osdp_pd *p, int is_cmd, uint8_t *data, int length);
int osdp_compute_mac(struct osdp_pd *p, int is_cmd, const uint8_t *data, int len);
void osdp_sc_init(struct osdp_pd *p);

/* from osdp_common.c */
int64_t osdp_millis_now(void);
int64_t osdp_millis_since(int64_t last);
void osdp_dump(const char *head, uint8_t *buf, int len);
uint16_t osdp_compute_crc16(const uint8_t *buf, size_t len);
void osdp_log(int log_level, const char *fmt, ...);
void osdp_log_ctx_set(int log_ctx);
void osdp_log_ctx_reset();
void osdp_log_ctx_restore();
void osdp_encrypt(uint8_t *key, uint8_t *iv, uint8_t *data, int len);
void osdp_decrypt(uint8_t *key, uint8_t *iv, uint8_t *data, int len);
void osdp_fill_random(uint8_t *buf, int len);
void safe_free(void *p);

int osdp_cmd_queue_init(struct osdp_pd *pd);
void osdp_cmd_queue_del(struct osdp_pd *pd);

struct osdp_cmd *osdp_cmd_alloc(struct osdp_pd *pd);
void osdp_cmd_free(struct osdp_pd *pd, struct osdp_cmd *cmd);
void osdp_cmd_enqueue(struct osdp_pd *pd, struct osdp_cmd *cmd);
int osdp_cmd_dequeue(struct osdp_pd *pd, struct osdp_cmd **cmd);
struct osdp_cmd *osdp_cmd_get_last(struct osdp_pd *pd);

#endif	/* _OSDP_COMMON_H_ */

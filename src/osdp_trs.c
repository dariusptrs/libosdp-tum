/*
 * Copyright (c) 2022 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file OSDP Transparent Reader Support
 *
 * TODO: Add notes about TRS mode and the overall command/reply exchange
 * between the CP and PD.
 */

#include "osdp_trs.h"

LOGGER_DECLARE(osdp, "TRS");

#define TO_TRS(pd) (pd)->trs

enum osdp_trs_state_e {
	OSDP_TRS_STATE_INIT,
};

struct osdp_trs {
	enum osdp_trs_state_e state;
	uint8_t mode;
};

#define MODE_CODE(mode, pcmnd) (uint16_t)(((mode) & 0xff) << 8u | ((pcmnd) & 0xff))

#define CMD_MODE_GET       MODE_CODE(0, 1)
#define CMD_MODE_SET       MODE_CODE(0, 2)
#define CMD_SEND_APDU      MODE_CODE(1, 1)
#define CMD_TERMINATE      MODE_CODE(1, 2)
#define CMD_ENTER_PIN      MODE_CODE(1, 3)
#define CMD_CARD_SCAN      MODE_CODE(1, 4)

/* if REPLY code is 0, it indicates and error */
#define REPLY_CURRENT_MODE        MODE_CODE(0, 1)
#define REPLY_CARD_INFO_REPORT    MODE_CODE(0, 2)
#define REPLY_CARD_PRSENT	      MODE_CODE(1, 1)
#define REPLY_CARD_DATA			  MODE_CODE(1, 2)
#define REPLY_PIN_ENTRY_COMPLETE  MODE_CODE(1, 3)

struct osdp_trs_cmd {
	uint16_t mode_code;
	union {
		struct cmd_mode_set {
			uint8_t mode;
			uint8_t config;
		} mode_set;
		struct send_apdu {
			int apdu_length;
			uint8_t apdu[64];
		} send_apdu;
		struct pin_entry {
			uint8_t timeout;
			uint8_t timeout2;
			uint8_t format_string;
			uint8_t pin_block_string;
			uint8_t ping_length_format;
			uint8_t pin_max_extra_digit_msb;
			uint8_t pin_max_extra_digit_lsb;
			uint8_t pin_entry_valid_condition;
			uint8_t pin_num_messages;
			uint8_t language_id_msb;
			uint8_t language_id_lsb;
			uint8_t msg_index;
			uint8_t teo_prologue[3];
			uint8_t apdu_length_msb;
			uint8_t apdu_length_lsb;
			uint8_t apdu[64];
		} pin_entry;
	};
};

struct osdp_trs_reply {
	// uint8_t mode;
	// uint8_t preply;
	uint16_t mode_code;
	union {
		struct cmd_reply_NAK {
			uint8_t err_code;
		} reply_nak;
		struct mode_setting_report {
			uint8_t mode;
			uint8_t mode_config;
		} mode_report;
		struct card_info_report {
			uint8_t reader;
			uint8_t protocol;
			uint8_t csn_len;
			uint8_t protocol_data_len;
			uint8_t csn[0];
			uint8_t protocol_data[0];
		} card_info_report;
		struct card_present_status {
			uint8_t reader;
			uint8_t status;
		} card_status;
		struct card_data {
			uint8_t reader;
			uint8_t status;
			uint8_t apdu[64];
		} card_data;
		struct pin_entry_complete {
			uint8_t reader;
			uint8_t status;
			uint8_t tries;
		} pin_entry_complete;
	};
};

/* --- Sender CMD/RESP Handers --- */

int osdp_trs_cmd_build(struct osdp_pd *pd, uint8_t *buf, int max_len)
{
	int len = 0, apdu_length, needed_space;
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_cmd *cmd = (struct osdp_trs_cmd *)pd->ephemeral_data;

	uint8_t mode = BYTE_1(cmd->mode_code);
	uint8_t code = BYTE_0(cmd->mode_code);

	/* mode <=> code validation */
	if (code == 0 || (mode != 0 && mode != 1) ||
	    (mode == 0 && code > 2) || (mode == 1 && code > 4)) {
		return -1;
	}

	buf[len++] = mode;
	buf[len++] = code;

	if (cmd->mode_code == CMD_MODE_GET) {
		goto out;
	}

	if (cmd->mode_code == CMD_MODE_SET) {
		buf[len++] = cmd->mode_set.mode;
		buf[len++] = cmd->mode_set.config;
		goto out;
	}

	buf[len++] = 0;  /* reader -- always 0 */

	switch(cmd->mode_code) {
	case CMD_SEND_APDU:
		buf[len++] = cmd->send_apdu.apdu_length;
		apdu_length = cmd->send_apdu.apdu_length;
		if (apdu_length > sizeof(cmd->send_apdu.apdu) ||
		    apdu_length > (max_len - len)) {
			LOG_ERR("APDU length 2BIG or Invalid! need/have: %d/%d",
				(max_len - len), apdu_length);
			return -1;
		}
		memcpy(buf, cmd->send_apdu.apdu, apdu_length);
		len += apdu_length;
		break;
	case CMD_ENTER_PIN:
		buf[len++] = cmd->pin_entry.timeout;
		buf[len++] = cmd->pin_entry.timeout2;
		buf[len++] = cmd->pin_entry.format_string;
		buf[len++] = cmd->pin_entry.pin_block_string;
		buf[len++] = cmd->pin_entry.ping_length_format;
		buf[len++] = cmd->pin_entry.pin_max_extra_digit_msb;
		buf[len++] = cmd->pin_entry.pin_max_extra_digit_lsb;
		buf[len++] = cmd->pin_entry.pin_entry_valid_condition;
		buf[len++] = cmd->pin_entry.pin_num_messages;
		buf[len++] = cmd->pin_entry.language_id_msb;
		buf[len++] = cmd->pin_entry.language_id_lsb;
		buf[len++] = cmd->pin_entry.msg_index;
		buf[len++] = cmd->pin_entry.teo_prologue[0];
		buf[len++] = cmd->pin_entry.teo_prologue[1];
		buf[len++] = cmd->pin_entry.teo_prologue[2];
		buf[len++] = cmd->pin_entry.apdu_length_msb;
		buf[len++] = cmd->pin_entry.apdu_length_lsb;

		apdu_length = cmd->pin_entry.apdu_length_msb << 8;
		apdu_length |= cmd->pin_entry.apdu_length_lsb;
		needed_space = max_len - len - sizeof(cmd->pin_entry.apdu);
		if (apdu_length > sizeof(cmd->pin_entry.apdu) ||
		    apdu_length > needed_space) {
			LOG_ERR("APDU length 2BIG or Invalid! need/have: %d/%d",
				needed_space, apdu_length);
			return -1;
		}
		memcpy(buf, cmd->pin_entry.apdu, apdu_length);
		len += apdu_length;
		break;
	}
out:
	return len;
}

int osdp_trs_reply_decode(struct osdp_pd *pd, uint8_t *buf, int len)
{
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_reply *reply;
	int csn_len, prot_data_len, pos = 0;

	reply = (struct osdp_trs_reply *)pd->ephemeral_data;

	switch(reply->mode_code) {
		case REPLY_CURRENT_MODE:
			reply->mode_report.mode = buf[pos++];
			reply->mode_report.mode_config = buf[pos++];
			break;
		case REPLY_CARD_INFO_REPORT:
			reply->card_info_report.reader = buf[pos++];
			reply->card_info_report.protocol = buf[pos++];

			csn_len = buf[pos++];
			prot_data_len = buf[pos++];
			reply->card_info_report.csn_len = csn_len;
			reply->card_info_report.protocol_data_len = prot_data_len;
			memcpy(reply->card_info_report.csn, buf+pos, csn_len);
			pos+=csn_len;
			memcpy(reply->card_info_report.protocol_data, buf+pos, prot_data_len);
			pos+=prot_data_len;
			break;
		case REPLY_CARD_PRSENT:
			reply->card_status.reader = buf[pos++];
			reply->card_status.status = buf[pos++];
			break;
		case REPLY_CARD_DATA:
			reply->card_data.reader = buf[pos++];
			reply->card_data.status = buf[pos++];
			memcpy(reply->card_data.apdu, buf+pos, len-2);
			break;
		case REPLY_PIN_ENTRY_COMPLETE:
			reply->pin_entry_complete.reader = buf[pos++];
			reply->pin_entry_complete.status = buf[pos++];
			reply->pin_entry_complete.tries = buf[pos++];
			break;
		default:
			break;

	}

	return 0;
}

/* --- Receiver CMD/RESP Handler --- */

int osdp_trs_reply_build(struct osdp_pd *pd, uint8_t *buf, int max_len)
{
	int len = 0, csn_len, prot_data_len;
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_reply *reply;

	reply = (struct osdp_trs_reply *)pd->ephemeral_data;

	buf[len++] = reply->mode;
	buf[len++] = reply->preply;

	switch (reply->mode_code)
	{
		case REPLY_CURRENT_MODE:
			buf[len++] = reply->mode_report.mode;
			buf[len++] = reply->mode_report.mode_config;
			break;
		case REPLY_CARD_INFO_REPORT:
			buf[len++] = reply->card_info_report.reader;
			buf[len++] = reply->card_info_report.protocol;

			csn_len = reply->card_info_report.csn_len;
			prot_data_len = reply->card_info_report.protocol_data_len;
			buf[len++] = reply->card_info_report.csn_len;
			buf[len++] = reply->card_info_report.protocol_data_len;
			memcpy(buf+len, reply->card_info_report.csn, csn_len);
			len+=csn_len;
			memcpy(buf+len, reply->card_info_report.protocol_data, prot_data_len);
			len+=prot_data_len;
			break;
		case REPLY_CARD_PRSENT:
			buf[len++] = reply->card_status.reader;
			buf[len++] = reply->card_status.status;
			break;
		case REPLY_CARD_DATA:
			buf[len++] = reply->card_data.reader;
			buf[len++] = reply->card_data.status;
			memcpy(buf+len, reply->card_data.apdu, max_len-2);
			break;
		case REPLY_PIN_ENTRY_COMPLETE:
			buf[len++] = reply->pin_entry_complete.reader;
			buf[len++] = reply->pin_entry_complete.status;
			buf[len++] = reply->pin_entry_complete.tries;
			break;
		default:
			break;
	}
	return len;
}

int osdp_trs_cmd_decode(struct osdp_pd *pd, uint8_t *buf, int len)
{
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_cmd *cmd = (struct osdp_trs_cmd *)pd->ephemeral_data;
	int pos = 0, remaining_space;
	int mode, next_mode, code, next_config, mode_code, apdu_length;

	memset(pd->ephemeral_data, 0, sizeof(pd->ephemeral_data));
	mode = buf[pos++];
	code = buf[pos++];
	mode_code = MODE_CODE(mode, code);

	/* mode <=> code validation */
	if (code == 0 || (mode != 0 && mode != 1) ||
	    (mode == 0 && code > 2) || (mode == 1 && code > 4)) {
		return -1;
	}

	/* only mode 0 commands are allowed in all modes */
	if (!mode && mode != trs->mode) {
		return -1;
	}

	if (code == CMD_MODE_GET) {
		return 0;
	}

	if (code == CMD_MODE_SET) {
		cmd->mode_set.mode = buf[pos++];
		cmd->mode_set.config = buf[pos++];
		return 0;
	}

	pos++;  /* reader -- always 0 */

	switch(mode_code) {
	case CMD_SEND_APDU:
		apdu_length = buf[pos++];
		remaining_space = len - pos;
		if (apdu_length > sizeof(cmd->send_apdu.apdu) ||
		    apdu_length > remaining_space) {
			LOG_ERR("APDU length 2BIG or Invalid! need/have: %d/%d",
				remaining_space, apdu_length);
			return -1;
		}
		cmd->send_apdu.apdu_length = apdu_length;
		memcpy(cmd->send_apdu.apdu, buf, apdu_length);
		pos += apdu_length;
		break;
	case CMD_ENTER_PIN:
		cmd->pin_entry.timeout = buf[pos++];
		cmd->pin_entry.timeout2 = buf[pos++];
		cmd->pin_entry.format_string = buf[pos++];
		cmd->pin_entry.pin_block_string = buf[pos++];
		cmd->pin_entry.ping_length_format = buf[pos++];
		cmd->pin_entry.pin_max_extra_digit_msb = buf[pos++];
		cmd->pin_entry.pin_max_extra_digit_lsb = buf[pos++];
		cmd->pin_entry.pin_entry_valid_condition = buf[pos++];
		cmd->pin_entry.pin_num_messages = buf[pos++];
		cmd->pin_entry.language_id_msb = buf[pos++];
		cmd->pin_entry.language_id_lsb = buf[pos++];
		cmd->pin_entry.msg_index = buf[pos++];
		cmd->pin_entry.teo_prologue[0] = buf[pos++];
		cmd->pin_entry.teo_prologue[1] = buf[pos++];
		cmd->pin_entry.teo_prologue[2] = buf[pos++];
		cmd->pin_entry.apdu_length_msb = buf[pos++];
		cmd->pin_entry.apdu_length_lsb = buf[pos++];

		remaining_space = (len - pos - sizeof(cmd->pin_entry.apdu));
		apdu_length = cmd->pin_entry.apdu_length_msb << 8;
		apdu_length |= cmd->pin_entry.apdu_length_lsb;
		if (apdu_length > sizeof(cmd->pin_entry.apdu) ||
		    apdu_length > remaining_space) {
			LOG_ERR("APDU length 2BIG or Invalid! need/have: %d/%d",
				remaining_space, apdu_length);
			return -1;
		}
		memcpy(cmd->pin_entry.apdu, buf, apdu_length);
		pos += apdu_length;
		break;
	}

	return pos;
}

/* --- State Management --- */

static int trs_cmd_set_mode(struct osdp_pd *pd, int to_mode, int to_config)
{
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_cmd *cmd;

	cmd = cp_cmd_alloc(pd);
	if (cmd == NULL) {
		return -1;
	}
	cmd->id = CMD_XWR;

	cp_cmd_enqueue(pd, cmd);
	return 0;
}

static int trs_state_update(struct osdp_pd *pd)
{
	struct osdp_trs_cmd *cmd = (struct osdp_trs_cmd *)pd->ephemeral_data;

	switch(pd->state) {

	};
}

/* --- Exported Methods --- */

OSDP_EXPORT
void osdp_register_challenge()
{
}
/*
 * Copyright (c) 2021 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OSDP_TRS_H_
#define _OSDP_TRS_H_

#include "osdp_common.h"

/**
 * @file OSDP Transparent Reader Support
 *
 * TODO: Add notes about TRS mode and the overall command/reply exchange
 * between the CP and PD.
 */

/* TRS Flags */
#define OSDP_TRS_MODE00			0x00 /* Default – no specific Behavior Mode is in effect */
#define OSDP_TRS_MODE01			0x01 /* Transparent smart card interface support */

/* Disables/Enables the Mode 0 Extended Read Card Info Report Response */
#define OSDP_TRS_CIR_DISABLED		0x00
#define OSDP_TRS_CIR_ENABLED		0x01

/* Mode specific reply codes for XRW_MODE = 1 */
#define OSDP_TRS_XRD_REPLY_ERROR			0x00 /* General error indication: PD was unable to process the command */
#define OSDP_TRS_XRD_REPLY_CONNECTION_STATUS		0x01 /* Notifies the CP of the current Smart Card Connection status. */
#define OSDP_TRS_XRD_REPLY_APDU_RESPONSE		0x02 /* Returning an APDU embedded in this from the specified reader */
#define OSDP_TRS_XRD_REPLY_PIN_SEQUENCE_COMPLETED	0x03 /* Reports that the reader has completed a “Secure PIN Entry” sequence */

/* Smart Card Present Stats */
#define OSDP_TRS_SCPS_NOT_PRESENT			0x00 /* Card Not Present */
#define OSDP_TRS_SCPS_PRESENT_INTERFACE_NOT_SPECIFIED	0x01 /* Card Present – Interface not specified */
#define OSDP_TRS_SCPS_PRESENT_CONTACTLESS_INTERFACE	0x02 /* Card Present on Contactless Interface */
#define OSDP_TRS_SCPS_PRESENT_CONTACT_INTERFACE		0x03 /* Card Present on Contact Interface */

enum osdp_trs_cmd_e {
	OSDP_TRS_CMD_M0_REQ,
	OSDP_TRS_CMD_M0_SET,
	OSDP_TRS_CMD_M1_XMIT,
	OSDP_TRS_CMD_M1_SCDONE,
	OSDP_TRS_CMD_M1_SPE,
	OSDP_TRS_CMD_M1_SCSCAN
};

enum osdp_trs_xrd_reply_e {
	OSDP_TRS_REPLY_M0_ERROR,
	OSDP_TRS_REPLY_M0_REQR,
	OSDP_TRS_REPLY_M0_CIRR,
	OSDP_TRS_REPLY_M1_ERROR,
	OSDP_TRS_REPLY_M1_MES,
	OSDP_TRS_REPLY_M1_SCREP,
	OSDP_TRS_REPLY_M1_SPER
};

struct osdp_trs_cmd {
	uint8_t mode;
	uint8_t pcmnd;
};

struct osdp_trs_reply {
	uint8_t mode;
	uint8_t preply;
};

/* --- Sender CMD/RESP Handers --- */

int osdp_trs_cmd_build(struct osdp_pd *pd, uint8_t *buf, int max_len)
{
	int len = 0;
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_cmd *cmd = (struct osdp_trs_cmd *)pd->ephemeral_data;

	buf[len++] = cmd->mode;
	buf[len++] = cmd->pcmnd;

	switch(trs->cmd){
	case OSDP_TRS_CMD_M0_REQ:
		break;
	case OSDP_TRS_CMD_M0_SET:
		buf[len++] = 0; /* code */
		buf[len++] = 0; /* config */
		break;
	case OSDP_TRS_CMD_M1_SCSCAN:
	case OSDP_TRS_CMD_M1_SCDONE:
		buf[len++] = 0; /* id */
		break;
	case OSDP_TRS_CMD_M1_XMIT:
		buf[len++] = 0; /* id */
		// TODO: memcpy APDU
		break;
	case OSDP_TRS_CMD_M1_SPE:
		buf[len++] = 0; /* id */
		buf[len++] = 0; /* timeout */
		buf[len++] = 0; /* timeout2 */
		buf[len++] = 0; /* format_string */
		buf[len++] = 0; /* pin_block_string */
		buf[len++] = 0; /* ping_length_format */
		buf[len++] = 0; /* pin_max_extra_digit_msb */
		buf[len++] = 0; /* pin_max_extra_digit_lsb */
		buf[len++] = 0; /* pin_entry_valid_condition */
		buf[len++] = 0; /* pin_num_messages */
		buf[len++] = 0; /* language_id_msb */
		buf[len++] = 0; /* language_id_lsb */
		buf[len++] = 0; /* msg_index */
		buf[len++] = 0; /* teo_prologue[0] */
		buf[len++] = 0; /* teo_prologue[1] */
		buf[len++] = 0; /* teo_prologue[2] */
		buf[len++] = 0; /* data_length_msb */
		buf[len++] = 0; /* data_length_lsb */
		buf[len++] = 0; /* ab_data[0] */
		// TODO: memcpy ab_data
		break;
	default:
		log_error("Unknown sub command");
	}

	return len;
}

int osdp_trs_reply_decode(struct osdp_pd *pd, uint8_t *buf, int len)
{
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_reply *reply = (struct osdp_trs_reply *)pd->ephemeral_data;

	return 0;
}

/* --- Receiver CMD/RESP Handler --- */

int osdp_trs_reply_build(struct osdp_pd *pd, uint8_t *buf, int max_len)
{
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_reply *reply = (struct osdp_trs_reply *)pd->ephemeral_data;

	buf[len++] = cmd->mode;
	buf[len++] = cmd->preply;

	switch (trs->reply) {
	case OSDP_TRS_REPLY_M1_MES:
		break;
	case OSDP_TRS_REPLY_M0_ERROR:
	case OSDP_TRS_REPLY_M1_ERROR:
		buf[len++] = 0; /* error */
		break;
	case OSDP_TRS_REPLY_M0_REQR:
		buf[len++] = 0; /* code */
		buf[len++] = 0; /* config */
		break;
	case OSDP_TRS_REPLY_M0_CIRR:
		buf[len++] = 0; /* id */
		buf[len++] = 0; /* protocol */
		buf[len++] = 0; /* csn_len */
		buf[len++] = 0; /* protocol_len */
		buf[len++] = 0; /* csn_data */
		// TODO: memcpy data
		break;
	case OSDP_TRS_REPLY_M1_SCREP:
		buf[len++] = 0; /* id */
		buf[len++] = 0; /* status */
		// TODO: memcpy APDU
		break;
	case OSDP_TRS_REPLY_M1_SPER:
		buf[len++] = 0; /* id */
		buf[len++] = 0; /* status */
		buf[len++] = 0; /* tries */
		break;
	default:
		log_error("Unknown sub command");
	}
}

int osdp_trs_cmd_decode(struct osdp_pd *pd, uint8_t *buf, int len)
{
	struct osdp_trs *trs = TO_TRS(pd);
	struct osdp_trs_cmd *cmd = (struct osdp_trs_cmd *)pd->ephemeral_data;

	return 0;
}

/* --- State Management --- */

static int trs_state_update(struct osdp_pd *pd)
{
	struct osdp_trs_cmd *cmd = (struct osdp_trs_cmd *)pd->ephemeral_data;

	switch(pd->state) {
	case OSDP_CP_STATE_TRS_M0_SET:
		osdp_trs_m00_set(pd);
		cmd->mode = 1;
		cp_cmd_enqueue(pd, CMD_XWR);
		break;
	case OSDP_CP_STATE_TRS_M0_REQ:
		/**
		 * Need to expose as API ?
		 */
		osdp_trs_m00_req(pd);
		break;
	case OSDP_CP_STATE_TRS_M1_XMIT:
		osdp_trs_m01_xmit(pd);
		cp_cmd_enqueue(pd, CMD_XWR);
		break;
	case OSDP_CP_STATE_TRS_M1_SCDONE:
		osdp_trs_m01_scdone(pd);
		break;
	case OSDP_CP_STATE_TRS_M1_SPE:
		osdp_trs_m01_spe(pd);
		break;
	case OSDP_CP_STATE_TRS_M1_SCSCAN:
		/**
		 * Need to expose as API ?
		 */
		osdp_trs_m01_scscan(pd);
		break;
	default:
		break;
	}
}

/* --- Exported Methods --- */

OSDP_EXPORT
void osdp_register_challenge()
{
}
/*
 * Copyright (c) 2021 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
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

/* Mode specific command codes for XRW_MODE = 0 */
enum osdp_trs_xwr_m00_pcmnd_e {
	OSDP_CP_TRS_PCMND_PR00REQ = 1, /* Request the PD to return the current XRW_MODE in effect */
	OSDP_CP_TRS_PCMND_PROOSET      /* Set the PD to the specified XRW_MOD */
};

/* Mode specific command codes for XRW_MODE = 1 */
enum osdp_trs_xwr_m01_pcmnd_e {
	OSDP_CP_TRS_PCMND_PR01XMIT = 1,	/* Pass the APDU embedded in this command to the specified reader */
	OSDP_CP_TRS_PCMND_PRO1SCDONE,	/* Notifies the designated reader to terminate its connection to the Smart Card */
	OSDP_CP_TRS_PCMND_PR01SPE,	/* Instructs the designated reader to perform “Secure PIN Entry” */
	OSDP_CP_TRS_PCMND_PR01SCSCAN,	/* Instructs the designated reader to perform a Smart Card Scan */
};

/* Mode specific reply codes for XRW_MODE = 0 */
enum osdp_trs_xrd_m00_preply_e {
	OSDP_CP_TRS_PREPLY_PR00ERROR,	/* General error indication: PD was unable to process the command */
	OSDP_CP_TRS_PREPLY_PR00REQR,	/* Returns the current XRW_MODE in effect */
	OSDP_CP_TRS_PREPLY_PR00CIRR	/* Returns a Card Information Report when a Smart Card is detected */
};

/* Mode specific reply codes for XRW_MODE = 1 */
enum osdp_trs_xrd_m01_preply_e {
	OSDP_CP_TRS_PREPLY_PR01ERROR,	/* General error indication: PD was unable to process the command */
	OSDP_CP_TRS_PREPLY_PR01PRES,	/* Notifies the CP of the current Smart Card Connection status. */
	OSDP_CP_TRS_PREPLY_PR01SCREP,	/* Returning an APDU embedded in this from the specified reader */
	OSDP_CP_TRS_PREPLY_PR01SPER	/* Reports that the reader has completed a “Secure PIN Entry” sequence */
};

/**
 * Tranparent mode commands
 */
struct osdp_cmd_xwr_pr00req {
	uint8_t pcmnd;
};

struct osdp_cmd_xwr_pr00set {
	uint8_t pcmnd;
	uint8_t code;
	uint8_t config;
};

struct osdp_cmd_xwr_pr01xmit {
	uint8_t pcmnd;
	uint8_t id;
	uint8_t apdu[255-3];
};

struct osdp_cmd_xwr_pr01scdone {
	uint8_t pcmnd;
	uint8_t id;
};

struct osdp_cmd_xwr_pr01spe {
	uint8_t pcmnd;
	uint8_t id;
	uint8_t btimeOut;
	uint8_t btimeOut2;
	uint8_t bmFormatString;
	uint8_t bmPINBlockString;
	uint8_t bmPINLengthFormat;
	uint8_t wPINMaxExtraDigit[2];
	uint8_t bEntryValidationCondition;
	uint8_t bNumerMessage;
	uint8_t wLangId[2];
	uint8_t bMsgIndex;
	uint8_t bTeoPrologue[3];
	uint32_t uIDataLength;
	uint8_t abData[255-21];
};

struct osdp_cmd_xwr_pr01scscan {
	uint8_t pcmnd;
	uint8_t id;
};

struct osdp_cmd_xwr {
	uint8_t length;
	uint8_t mode;
	uint8_t command;
	union {
		struct osdp_cmd_xwr_pr00req pr00req;
		struct osdp_cmd_xwr_pr00set pr00set;
		struct osdp_cmd_xwr_pr01xmit pr01xmit;
		struct osdp_cmd_xwr_pr01scdone pr01scdone;
		struct osdp_cmd_xwr_pr01spe pr01spe;
		struct osdp_cmd_xwr_pr01scscan pr01scscan;
	};
};

/**
 * Tranparent mode response
 */
struct osdp_reply_xrd_pr001error {
	uint8_t preply;
	uint8_t error;
};

struct osdp_reply_xrd_pr00reqr {
	uint8_t preply;
	uint8_t code;
	uint8_t config;
};

struct osdp_reply_xrd_pr00cirr {
	uint8_t preply;
	uint8_t id;
	uint8_t cprotocol;
	uint8_t csnlen;
	uint8_t protocollen;
	uint8_t csndata[0];
	uint8_t protocoldata[0];
};

struct osdp_reply_xrd_pr01pres {
	uint8_t preply;
	uint8_t id;
	uint8_t status;
};

struct osdp_reply_xrd_pr01screp {
	uint8_t preply;
	uint8_t id;
	uint8_t status;
	uint8_t apdu[0];
};

struct osdp_reply_xrd_pr01sper {
	uint8_t preply;
	uint8_t id;
	uint8_t status;
	uint8_t tries;
};

struct osdp_reply_xrd {
	uint8_t length;
	uint8_t mode;
	union {
		struct osdp_reply_xrd_pr001error pr001error;
		struct osdp_reply_xrd_pr00reqr pr00reqr;
		struct osdp_reply_xrd_pr00cirr pr00cirr;
		struct osdp_reply_xrd_pr01pres pr01pres;
		struct osdp_reply_xrd_pr01screp pr01screp;
		struct osdp_reply_xrd_pr01sper pr01sper;
	};
};


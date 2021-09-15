/*
 * Copyright (c) 2021 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OSDP_TRS_H_
#define _OSDP_TRS_H_

#include "osdp_common.h"

#ifdef CONFIG_OSDP_TRS

static int trs_estate_update(struct osdp_pd *pd)
{
	case OSDP_CP_STATE_TRS_M00_SET:
		osdp_trs_m00_set(pd);
		cp_cmd_dispatcher(pd, CMD_XWR);
		cp_set_state(pd, OSDP_CP_STATE_TRS_M01_XMIT);
		break;
	case OSDP_CP_STATE_TRS_M00_REQ:
		/**
		 * Need to expose as API ?
		 */
		osdp_trs_m00_req(pd);
		cp_cmd_dispatcher(pd, CMD_XWR);
		cp_set_state(pd, OSDP_CP_STATE_ONLINE);
		break;
	case OSDP_CP_STATE_TRS_M01_XMIT:
		osdp_trs_m01_xmit(pd);
		cp_cmd_dispatcher(pd, CMD_XWR);
		cp_set_state(pd, OSDP_CP_STATE_ONLINE);
		break;
	case OSDP_CP_STATE_TRS_M01_SCDONE:
		osdp_trs_m01_scdone(pd);
		cp_cmd_dispatcher(pd, CMD_XWR);
		cp_set_state(pd, OSDP_CP_STATE_ONLINE);
		break;
	case OSDP_CP_STATE_TRS_M01_SPE:
		osdp_trs_m01_spe(pd);
		cp_cmd_dispatcher(pd, CMD_XWR);
		cp_set_state(pd, OSDP_CP_STATE_ONLINE);
		break;
	case OSDP_CP_STATE_TRS_M01_SCSCAN:
		/**
		 * Need to expose as API ?
		 */
		osdp_trs_m01_scscan(pd);
		cp_cmd_dispatcher(pd, CMD_XWR);
		cp_set_state(pd, OSDP_CP_STATE_ONLINE);
		break;
}

#else /* CONFIG_OSDP_TRS */

#endif /* CONFIG_OSDP_TRS */

#endif /* _OSDP_TRS_H_ */
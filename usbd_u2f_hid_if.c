/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @author         : MCD Application Team
  * @version        : V2.2.0
  * @date           : 13-June-2014
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_u2f_hid_if.h"
#include "u2f_hid.h"
#include "u2f.h"
#include "u2f_messages.h"
#include "list.h"
#include "endian.h"
#include "uart_printf.h"

#define d_printf uart_printf


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

__ALIGN_BEGIN static uint8_t U2F_HID_ReportDesc_FS[USBD_U2F_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
  /* USER CODE BEGIN 0 */ 
	0x06,
	0xD0,
	0xF1,
	0x09,
	0x01,
	0xA1,
	0x01,
	0x09,
	0x20,
	0x15,
	0x00,
	0x26,
	0xFF,
	0x00,
	0x75,
	0x08,
	0x95,
	0x40,
	0x81,
	0x02,
	0x09,
	0x21,
	0x15,
	0x00,
	0x26,
	0xFF,
	0x00,
	0x75,
	0x08,
	0x95,
	0x40,
	0x91,
	0x02,
  /* USER CODE END 0 */ 
  0xC0    /*     END_COLLECTION	             */
   
}; 
/* USB handler declaration */
/* Handle for USB Full Speed IP */
USBD_HandleTypeDef  *hUsbDevice_0;

extern USBD_HandleTypeDef hUsbDeviceFS;

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int8_t U2F_HID_Init_FS     (void);
static int8_t U2F_HID_DeInit_FS   (void);
static int8_t U2F_HID_OutEvent_FS (uint8_t *data, uint16_t len);
/* USER CODE BEGIN 2 */ 
/* USER CODE END 2 */ 

USBD_U2F_HID_ItfTypeDef USBD_U2FHID_fops_FS =
{
  U2F_HID_ReportDesc_FS,
  U2F_HID_Init_FS,
  U2F_HID_DeInit_FS,
  U2F_HID_OutEvent_FS,
};

/* Private functions ---------------------------------------------------------*/
uint8_t USBD_U2F_HID_SendFrame(USBD_HandleTypeDef  *pdev,
                                 U2FHID_FRAME *frame)
{
    USBD_U2F_HID_HandleTypeDef *hhid = (USBD_U2F_HID_HandleTypeDef *)pdev->pClassData;
    volatile int *state = &hhid->state;

    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        while (*state != U2F_HID_IDLE) {
            HAL_Delay(1);
        }
        hhid->state = U2F_HID_BUSY;
        USBD_LL_Transmit(pdev,
                        U2F_HID_EPIN_ADDR,
                        (uint8_t *)frame,
                        HID_RPT_SIZE);
    }
    return USBD_OK;
}

#define ISIZE sizeof(frame.init.data)
#define CSIZE sizeof(frame.cont.data)

uint8_t USBD_U2F_HID_SendResponse(USBD_HandleTypeDef  *pdev,
                                 uint32_t cid,
                                 uint8_t cmd,
                                 uint8_t *data,
                                 uint16_t len)
{
    static U2FHID_FRAME frame;
    uint16_t off = 0;
    size_t copy;
    int seq = 0;

    frame.cid = cid;
    frame.type = TYPE_INIT;
    frame.init.cmd |= cmd;
    frame.init.bcnth = (len >> 8) & 0xff;
    frame.init.bcntl = len & 0xff;

    copy = MIN(len, ISIZE);
    USBD_memcpy(frame.init.data, data, copy);
    if (copy < ISIZE) {
        USBD_memset(&frame.init.data[copy], 0, ISIZE - copy);
    }
    USBD_U2F_HID_SendFrame(pdev, &frame);
    off += copy;

    while (len > off) {
        frame.cont.seq = seq++;
        copy = MIN(len - off, CSIZE);
        USBD_memcpy(frame.cont.data, data + off, copy);
        if (copy < CSIZE) {
            USBD_memset(&frame.cont.data[copy], 0, CSIZE - copy);
        }
        USBD_U2F_HID_SendFrame(pdev, &frame);
        off += copy;
    }

    return USBD_OK;
}

/**
  * @brief  U2F_HID_DeInit_FS
  *         DeInitializes the U2F HID media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t U2F_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */ 
  return (0);
  /* USER CODE END 5 */ 
}

#define CID_STATE_IDLE 1
#define CID_STATE_RECV 2
#define CID_STATE_READY 3

struct u2f_channel {
    struct list_head list;
    struct list_head ready;
    uint32_t cid;                        // Channel identifier
    uint32_t expire;
    uint16_t buff_size;
    uint16_t bcnt_total;
    uint16_t bcnt_current;
    uint8_t state;
    uint8_t cmd;
    uint8_t seq;
    uint8_t data[0];
};

#define MAX_CHANNELS 5
#define MAX_U2F_BUFSIZE 7609
uint8_t g_cnt_cid = 0;
struct u2f_channel *u2f_channel_alloc(uint16_t buff_size)
{
    struct u2f_channel *ret;
    size_t size = sizeof(struct u2f_channel) + buff_size;

    if (g_cnt_cid >= MAX_CHANNELS) {
        return NULL;
    }
    ret = USBD_malloc(size);
    if (ret) {
        g_cnt_cid++;
    }

    return ret;
}

void u2f_channel_free(struct u2f_channel *c) {
    USBD_free(c);
    g_cnt_cid--;
}

uint32_t g_cid = 1;
struct list_head channel_list_head = LIST_HEAD_INIT(channel_list_head);
struct list_head ready_list_head = LIST_HEAD_INIT(ready_list_head);

void u2f_channel_init(struct u2f_channel *c, uint16_t buff_size, uint32_t cid)
{
    size_t size = sizeof(struct u2f_channel) + buff_size;

    USBD_memset(c, 0, size);
    c->cid = cid;
    c->buff_size = buff_size;
    c->state = CID_STATE_IDLE;
    INIT_LIST_HEAD(&c->list);
    INIT_LIST_HEAD(&c->ready);
    list_add_tail(&c->list, &channel_list_head);
}

void u2f_channel_deinit(struct u2f_channel *c)
{
    list_del(&c->list);
}

struct u2f_channel* u2f_channel_find(uint32_t cid)
{
    struct list_head *p;
    list_for_each(p, &channel_list_head) {
        struct u2f_channel *c = list_entry(p, struct u2f_channel, list);
        if (c->cid == cid) {
            return c;
        }
    }

    return NULL;
}

static void u2f_response_error(uint8_t code)
{

}

static void u2f_execute_ping(struct u2f_channel *c)
{
    USBD_U2F_HID_SendResponse(&hUsbDeviceFS, c->cid, c->cmd, c->data, c->bcnt_total);
}

struct u2f_request_message_apdu
{
	uint8_t cla;
	uint8_t ins;
	uint8_t p1;
	uint8_t p2;
	uint8_t lc1;
	uint8_t lc2;
	uint8_t lc3;
} __attribute__((packed));


static void u2f_msg_zero_len_response(struct u2f_channel *c, uint16_t status)
{
    uint16_t be_status = cpu_to_be16(status);
    USBD_U2F_HID_SendResponse(&hUsbDeviceFS, c->cid, c->cmd, (uint8_t *)&be_status, sizeof(be_status));
}

static void u2f_execute_msg(struct u2f_channel *c)
{
    uint32_t req_size;
    struct u2f_request_message_apdu *req_hdr =
        (struct u2f_request_message_apdu *)c->data;
    uint8_t *slack = c->data + c->bcnt_total;
    uint16_t slack_size = c->buff_size - c->bcnt_total;

    d_printf("%s {\n", __func__);
    if (req_hdr->cla != 0) {
        /* BUGBUG: ?? */
    }

    req_size = ((uint32_t)req_hdr->lc1) << 16 |  ((uint32_t)req_hdr->lc2) << 8 |  req_hdr->lc1;

    switch (req_hdr->ins)
    {
        case U2F_REGISTER:
        {
            U2F_REGISTER_REQ *preq = (U2F_REGISTER_REQ *)(req_hdr + 1);
            U2F_REGISTER_RESP *presp = (U2F_REGISTER_RESP *)slack;
            d_printf("REGISTER\n");
            if (slack_size < sizeof(U2F_REGISTER_RESP) + 2)    {
                u2f_msg_zero_len_response(c, VENDOR_U2F_NOMEM);
            } else {
                uint16_t status, len;
                status = u2f_register(preq, presp, req_hdr->p1, &len);
                status = cpu_to_be16(status);
                memcpy(slack + len, &status, sizeof(status));
                USBD_U2F_HID_SendResponse(&hUsbDeviceFS, c->cid, c->cmd, slack, len + sizeof(status));
            }
        }
        break;
        case U2F_AUTHENTICATE:
        {
            U2F_AUTHENTICATE_REQ *preq = (U2F_AUTHENTICATE_REQ *)(req_hdr + 1);
            U2F_AUTHENTICATE_RESP *presp = (U2F_AUTHENTICATE_RESP *)slack;
            d_printf("AUTHENTICATE\n");
            if (slack_size < sizeof(U2F_AUTHENTICATE_RESP) + 2) {
                u2f_msg_zero_len_response(c, VENDOR_U2F_NOMEM);
            }
            else {
                uint16_t status, len = sizeof(U2F_AUTHENTICATE_RESP);
                status = u2f_authenticate(preq, presp, req_hdr->p1);
                if (status == U2F_SW_NO_ERROR) {
                    status = cpu_to_be16(status);
                    memcpy(slack + len, &status, sizeof(status));
                    USBD_U2F_HID_SendResponse(&hUsbDeviceFS, c->cid, c->cmd, slack, len + sizeof(status));
                } else {
                    u2f_msg_zero_len_response(c, status);
                }
            }
        }
        break;
        case U2F_VERSION:
        {
            const char *version_str = VENDOR_U2F_VERSION;
            d_printf("VERSION\n");
            if (slack_size < sizeof(VENDOR_U2F_VERSION) + 2) {
                u2f_msg_zero_len_response(c, VENDOR_U2F_NOMEM);
            }
            else {
                uint16_t status = cpu_to_be16(U2F_SW_NO_ERROR), len = sizeof(VENDOR_U2F_VERSION)-1;
                memcpy(slack, version_str, len);
                memcpy(slack + len, &status, sizeof(status));
                USBD_U2F_HID_SendResponse(&hUsbDeviceFS, c->cid, c->cmd, slack, len + sizeof(status));
            }
        }
        break;
        case U2F_AUTHENTICATE_BATCH:
        case U2F_CHECK_REGISTER:
        default:
            d_printf("unknown\n");
            u2f_msg_zero_len_response(c, U2F_SW_INS_NOT_SUPPORTED);
            break;
    }
}

static void u2f_execute_lock(struct u2f_channel *c)
{

}

static void u2f_execute_init(struct u2f_channel *c)
{
    U2FHID_INIT_REQ *req_init = (U2FHID_INIT_REQ *)c->data;
    U2FHID_INIT_RESP *resp_init = (U2FHID_INIT_RESP *)c->data;
    struct u2f_channel *newc;

    if (c->cid != 0xffffffff) {
         u2f_response_error(ERR_INVALID_PAR);
         //BUGBUG reset
         return;
    }

    newc = u2f_channel_alloc(MAX_U2F_BUFSIZE);
    if (!newc) {
         u2f_response_error(ERR_OTHER);
         //BUGBUG reset
         return;
    }
    u2f_channel_init(newc, MAX_U2F_BUFSIZE, g_cid++);

    /* Nonce is already at same location, no need to copy */
    resp_init->cid = newc->cid;                     // Channel identifier 
    resp_init->versionInterface = U2FHID_IF_VERSION;// Interface version
    resp_init->versionMajor = 1;                    // Major version number
    resp_init->versionMinor = 0;                    // Minor version number
    resp_init->versionBuild = 0;                    // Build version number
    resp_init->capFlags = CAPFLAG_WINK;             // Capabilities flags

    USBD_U2F_HID_SendResponse(&hUsbDeviceFS, c->cid, c->cmd, (uint8_t *)resp_init, sizeof(*resp_init));
}

static void u2f_execute_wink(struct u2f_channel *c)
{

}

static void u2f_execute_sync(struct u2f_channel *c)
{

}

static void u2f_execute_command(struct u2f_channel *c)
{
    switch(c->cmd)
    {
    case U2FHID_PING: // Echo data through local processor only
        d_printf("U2FHID_PING\n");
        return u2f_execute_ping(c);
    case U2FHID_MSG:  // Send U2F message frame
        d_printf("U2FHID_MSG\n");
        return u2f_execute_msg(c);
    case U2FHID_LOCK: // Send lock channel command
        d_printf("U2FHID_LOCK\n");
        return u2f_execute_lock(c);
    case U2FHID_INIT: // Channel initialization
        d_printf("U2FHID_INIT\n");
        return u2f_execute_init(c);
    case U2FHID_WINK: // Send device identification wink
        d_printf("U2FHID_WINK\n");
        return u2f_execute_wink(c);
    case U2FHID_SYNC: // Protocol resync command
        d_printf("U2FHID_SYNC\n");
        return u2f_execute_sync(c);
    case U2FHID_VENDOR_FIRST: // First vendor defined command
        d_printf("U2FHID_VENDOR_FIRST\n");
        break;
    default:
        d_printf("unk command!\n");
        break;
    }
}

void u2f_channel_process_ready()
{
    struct list_head *p, *n;
    list_for_each_safe(p, n, &ready_list_head)    {
        struct u2f_channel *c = list_entry(p, struct u2f_channel, ready);
        u2f_execute_command(c);
        list_del(&c->ready);
        c->state = CID_STATE_IDLE;
    }
}

static int8_t u2f_channel_recv_frame(struct u2f_channel *c, U2FHID_FRAME *frame)
{
    uint16_t copy;

    if (FRAME_TYPE(*frame) == TYPE_INIT) {
        if (c->state != CID_STATE_IDLE) {
            u2f_response_error(ERR_INVALID_PAR);
            //BUGBUG reset
            return (0);
        }

        c->bcnt_total = MSG_LEN(*frame); //BUGBUG check smaller then max buff size
        c->cmd = frame->init.cmd;
        c->seq = 0;
        c->expire = HAL_GetTick() + U2FHID_TRANS_TIMEOUT;
        c->state = CID_STATE_RECV;
        c->bcnt_current = copy = MIN(c->bcnt_total, sizeof(frame->init.data));
        USBD_memcpy(c->data, frame->init.data, copy);
    } else if (FRAME_TYPE(*frame) == TYPE_CONT) {
        if (c->state != CID_STATE_RECV) {
            u2f_response_error(ERR_INVALID_PAR);
            return (0);
        }
        if (FRAME_SEQ(*frame) != c->seq++) {
            u2f_response_error(ERR_INVALID_SEQ);
            return (0);
        }
        copy = MIN(c->bcnt_total - c->bcnt_current, sizeof(frame->cont.data));
        USBD_memcpy(c->data + c->bcnt_current, frame->cont.data, copy);
        c->bcnt_current += copy;
    } else {
        u2f_response_error(ERR_INVALID_PAR);
        return (0);
    }

    if (c->state == CID_STATE_RECV &&
        c->bcnt_current == c->bcnt_total) {
        c->state = CID_STATE_READY;
        d_printf("channel %d ready\n", c->cid);
        list_add_tail(&c->ready, &ready_list_head);
    }

    return (0);
}

/**
  * @brief  U2F_HID_OutEvent_FS
  *         Manage the U2F HID class events       
  * @param  event_idx: event index
  * @param  state: event state
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t U2F_HID_OutEvent_FS  (uint8_t *data, uint16_t len)
{
    U2FHID_FRAME *frame = (U2FHID_FRAME *)data;
    struct u2f_channel *c;

    if(len != HID_RPT_SIZE) {
        u2f_response_error(ERR_INVALID_LEN);
        return (0);
    }

    c = u2f_channel_find(frame->cid);
    if (!c) {
        u2f_response_error(ERR_INVALID_PAR);
        return (0);
    }

    return u2f_channel_recv_frame(c, frame);

    return (0);
}

/**
  * @brief  U2F_HID_Init_FS
  *         Initializes the U2F HID media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t U2F_HID_Init_FS(void)
{
    /* USER CODE BEGIN 4 */
    struct u2f_channel *newc;
    hUsbDevice_0 = &hUsbDeviceFS;

    newc = u2f_channel_alloc(MAX_U2F_BUFSIZE);
    if (!newc) {
         return -1;
    }
    u2f_channel_init(newc, MAX_U2F_BUFSIZE, 0xffffffff);

    return (0);
    /* USER CODE END 4 */
}

/* USER CODE BEGIN 7 */ 
/**
  * @brief  USBD_U2F_HID_SendReport_FS
  *         Send the report to the Host       
  * @param  report: the report to be sent
  * @param  len: the report length
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
/*  
static int8_t USBD_U2F_HID_SendReport_FS ( uint8_t *report,uint16_t len)
{
  return USBD_U2F_HID_SendReport(hUsbDevice_0, report, len); 
}
*/
/* USER CODE END 7 */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

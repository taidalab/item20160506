/***************************************************************************//**
 * @file spi-host-ui.h
 * @brief Header for SPI Host functions
 * 
 * See @ref spi_util for documentation.
 * 
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
/** @addtogroup spi_util
 *
 * See spi-host.h.
 *
 *@{
 */
 
#ifndef __SPI_HOST_H__
#define __SPI_HOST_H__
#include "../ezsp-host-common.h"

#define SPI_MAX_TIMEOUTS          6   /*!< timeouts before link is judged down */

#define SPI_PORT_LEN              40  /*!< length of serial port name string */

// Bits in traceFlags to enable various host trace outputs
#define TRACE_FRAMES_BASIC        1   /*!< frames sent and received */
#define TRACE_FRAMES_VERBOSE      2   /*!< basic frames + internal variables */
#define TRACE_EVENTS              4   /*!< events */
#define TRACE_EZSP                8   /*!< EZSP commands, responses and callbacks */
#define TRACE_EZSP_VERBOSE        16  /*!< additional EZSP information */


// dummy values to fill default host configuration for compatibility with ASH
// ncpType values
#define SPI_NCP_TYPE_EM2XX_EM3XX  0   /*!< EM2XX or EM3XX */
// resetMethod values
#define SPI_RESET_METHOD_RST      0   /*!< send RST frame */
#define SPI_RESET_METHOD_NONE     3   /*!< no reset - for testing */


#define spiReadConfig(member) \
  (spiHostConfig.member)

#define spiReadConfigOrDefault(member, defval) \
  (spiHostConfig.member)

#define spiWriteConfig(member, value) \
  do { spiHostConfig.member = value; } while (0)

extern EzspHostConfig spiHostConfig;
extern bool ncpSleepEnabled;

#endif //__SPI_HOST_H___

/** @} // END addtogroup
 */

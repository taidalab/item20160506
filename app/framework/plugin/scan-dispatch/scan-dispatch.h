//
// scan-dispatch.h
//
// Dispatching 802.15.4 scan results to interested parties.
//
// April 15, 2015
//
// Copyright 2015 Silicon Laboratories, Inc.                               *80*
//

#ifndef __SCAN_DISPATCH_H__
#define __SCAN_DISPATCH_H__

/** @addtogroup scan-dispatch Scan Dispatch
 *
 * This plugin allows for there to be multiple consumers of stack
 * 802.15.4 scan callbacks.
 *
 * @{
 */

/**
 * @brief The size of the dispatch queue.
 */
#ifndef EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE
  #define EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE 10
#endif

/**
 * @brief Information regarding scan results.
 */
typedef struct {
  /** The status or the rssi of the scan. */
  union {
    EmberStatus status;
    int8_t rssi;
  };

  /** The channel or the lqi of the scan. */
  union {
    uint8_t channel;
    uint8_t lqi;
  };

  /** The ZigBee network found in the scan. */
  EmberZigbeeNetwork *network;

  /** A mask containing information about the scan. */
  uint16_t mask;
} EmberAfPluginScanDispatchScanResults;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  #define EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_SCAN_TYPE (0x00FF)
  #define EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_COMPLETE  (0x0100)
  #define EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_FAILURE   (0x0200)
#endif

/** @brief Get Scan Type
 *
 * Gets the scan type from an ::EmberAfPluginScanDispatchScanResults.
 * This value will either be ::EMBER_ENERGY_SCAN or ::EMBER_ACTIVE_SCAN.
 *
 * @param results The ::EmberAfPluginScanDispatchScanResults for which
 * the scan type will be found.
 *
 * @return The ::EmberNetworkScanType of the scan results.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
  EmberNetworkScanType
  emberAfPluginScanDispatchScanResultsGetScanType(EmberAfPluginScanDispatchScanResults *results);
#else
  #define emberAfPluginScanDispatchScanResultsGetScanType(results) \
    ((EmberNetworkScanType)((results)->mask                        \
                            & EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_SCAN_TYPE))
#endif

/** @brief Results Are Complete
 *
 * Get whether or not the scan that was originally requested by the
 * consumer has completed. This can happen either when the dispatcher
 * asks the stack for a scan (see ::emberStartScan) or after all of
 * the scan results have been delivered to the consumer (see
 * ::emberAfScanCompleteCallback).
 *
 * @param results The ::EmberAfPluginScanDispatchScanResults that belong
 * to a potentially complete scan.
 *
 * @return Whether or not the scan for the scan results is complete.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
  bool
  emberAfPluginScanDispatchScanResultsAreComplete(EmberAfPluginScanDispatchScanResults *results);
#else
  #define emberAfPluginScanDispatchScanResultsAreComplete(results)       \
    HIGH_BYTE(((results)->mask                                           \
               & EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_COMPLETE))
#endif

/** @brief Results Are Failure
 *
 * Get whether or not the scan results are from a failed call
 * to ::emberStartScan.
 *
 * @param results The ::EmberAfPluginScanDispatchScanResults for which
 * the call to ::emberStartScan may have failed.
 *
 * @return Whether or not the call to ::emberStartScan failed for these
 * results.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
  bool
  emberAfPluginScanDispatchScanResultsAreFailure(EmberAfPluginScanDispatchScanResults *results);
#else
  #define emberAfPluginScanDispatchScanResultsAreFailure(results)   \
    HIGH_BYTE(((results)->mask                                      \
           & EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_FAILURE))
#endif

/**
 * @brief A function type that handles scan results.
 */
typedef void (*EmberAfPluginScanDispatchScanResultsHandler)(EmberAfPluginScanDispatchScanResults *results);

/**
 * @brief A structure containing data for scheduling a scan.
 */
typedef struct {
  /** The 802.15.4 scan type to be scheduled. */
  EmberNetworkScanType scanType;
  /** The channel mask to be scanned. */
  uint32_t channelMask;
  /** The duration of the scan period, as an exponent. */
  uint8_t duration;
  /** The handler to be called with the scan results. */
  EmberAfPluginScanDispatchScanResultsHandler handler;
} EmberAfPluginScanDispatchScanData;

/** @brief Schedule Scan
 *
 * This API will schedule an 802.15.4 scan. The results will be delivered to
 * the consumer via a handler in the passed ::EmberAfPluginScanDispatchScanData.
 *
 * @param data An ::EmberAfPluginScanDispatchScanData that holds the scanType,
 * channelMask, duration, and ::EmberAfPluginScanDispatchScanResultsHandler for
 * the scan.
 *
 * @return An ::EmberStatus value describing the result of the scheduling of
 * a scan.
 */
EmberStatus emberAfPluginScanDispatchScheduleScan(EmberAfPluginScanDispatchScanData *data);

/** @brief Clear
 *
 * A call to this function will remove all consumers in the queue for scan
 * results. It will also cancel any 802.15.4 scan that the stack is currently
 * perfoming.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
  void emberAfPluginScanDispatchClear(void);
#else
  #define emberAfPluginScanDispatchClear emberAfPluginScanDispatchInitCallback
#endif

// @} END addtogroup

#endif /* __SCAN_DISPATCH_H__ */

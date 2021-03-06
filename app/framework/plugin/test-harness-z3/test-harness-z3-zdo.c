//
// test-harness-z3-zdo.c
//
// August 3, 2015
// Refactored November 23, 2015
//
// ZigBee 3.0 zdo test harness functionality
//

#include "app/framework/include/af.h"

#include "app/util/zigbee-framework/zigbee-device-host.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

#include "test-harness-z3-core.h"

// -----------------------------------------------------------------------------
// Constants

#define ZDO_NETWORK_UPDATE_REQUEST (0x0038)

// -----------------------------------------------------------------------------
// Globals

EmberEventControl emberAfPluginTestHarnessZ3ZdoSendEventControl;

#define NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER (0x00FF)
static EmberNodeId currentZdoNegativeCommandDestination = EMBER_NULL_NODE_ID;
static uint16_t currentZdoNegativeCommandCluster = NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER;
static uint8_t currentZdoNegativeCommandFrame[32];
static uint8_t currentZdoNegativeCommandFrameLength;

// -----------------------------------------------------------------------------
// ZDO Commands

// Declared in zdo-cli.c. Used here so CLI commands can be reused.
extern uint16_t emAfCliZdoInClusters[];
extern uint16_t emAfCliZdoOutClusters[];
extern uint8_t emAfCliInClCount;
extern uint8_t emAfCliOutClCount;

static uint32_t negativeZdoCommandFlags = 0;
#define NEGATIVE_ZDO_COMMAND_FLAGS_OUR_NWK_ADDRESS        BIT32(0)
#define NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT      BIT32(1)
#define NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE BIT32(2)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR   BIT32(3)
#define NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_KE        BIT32(4)
#define NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_SWAP      BIT32(5)
#define NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_ABSENT    BIT32(6)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_ENDPOINT_FIELDS     BIT32(7)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_NWK_ADDR            BIT32(8)
#define NEGATIVE_ZDO_COMMAND_FLAGS_NO_NEGATIVE_RESPONSE   BIT32(9)

#ifdef EZSP_HOST

static EmberStatus sendZdoCommand(EmberNodeId destination,
                                  uint16_t clusterId,
                                  uint8_t *frame,
                                  uint8_t frameLength)
{
  return EMBER_INVALID_CALL;
}

#else

extern EmberNodeId emCurrentSender;

static EmberStatus sendZdoCommand(EmberNodeId destination,
                                  uint16_t clusterId,
                                  uint8_t *frame,
                                  uint8_t frameLength)
{
  EmberStatus status;
  EmberMessageBuffer message;
  EmberApsFrame apsFrame = {
    EMBER_ZDO_PROFILE_ID,
    clusterId,
    EMBER_ZDO_ENDPOINT,
    EMBER_ZDO_ENDPOINT,
    (EMBER_APS_OPTION_RETRY),
    0, // group id
    0, // sequence
  };

  frame[0] = emberNextZigDevRequestSequence();
  message = emberFillLinkedBuffers(frame, frameLength);

  if (message == EMBER_NULL_MESSAGE_BUFFER) {
    return EMBER_NO_BUFFERS;
  }

  if (emberIsZigbeeBroadcastAddress(destination)) {
    status = emberSendBroadcast(destination,
                                &apsFrame,
                                EMBER_MAX_HOPS,
                                message);
  } else {
    status = emberSendUnicast(EMBER_OUTGOING_DIRECT,
                              destination,
                              &apsFrame,
                              message);
  }

  emberReleaseMessageBuffer(message);

  return status;
}

#endif /* EZSP_HOST */

// plugin test-harness z3 mgmt permit-joining-req <timeS:2> <dstShort:2>
void emAfPluginTestHarnessZ3MgmtPermitJoiningReqCommand(void)
{
  uint16_t permitDurationS = (uint16_t)emberUnsignedCommandArgument(0);
  EmberNodeId destination  = (EmberNodeId)emberUnsignedCommandArgument(1);
  uint32_t options         = emAfPluginTestHarnessZ3GetSignificantBit(2);
  EmberStatus status;

  status = emberPermitJoiningRequest(destination,
                                     permitDurationS,
                                     (options == BIT32(0) ? false : true),
                                     (EMBER_APS_OPTION_RETRY));

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Permit joining request",
                     status);
}

// plugin test-harness z3 mgmt leave <dstShort:2> <removeChildren:1>
// <rejoin:1> <optionBitmask:4>
void emAfPluginTestHarnessZ3MgmtLeaveCommand(void)
{
  EmberNodeId destination = (EmberNodeId)emberUnsignedCommandArgument(0);
  bool removeChildren     = (bool)emberUnsignedCommandArgument(1);
  bool rejoin             = (bool)emberUnsignedCommandArgument(2);
  uint32_t options        = emAfPluginTestHarnessZ3GetSignificantBit(3);
  EmberStatus status = EMBER_INVALID_CALL;
  uint8_t frame[10];
  uint8_t *finger = &frame[1];

  // Long address of destination. Bit 0 means don't put it in payload.
  if (options != BIT(0)) {
    status = emberLookupEui64ByNodeId(destination, finger);
    if (status != EMBER_SUCCESS) {
      goto done;
    }
    finger += EUI64_SIZE;
  }

  // Options.
  if (options != BIT(1)) {
    *finger = 0;
    if (removeChildren) *finger |= BIT(6);
    if (rejoin)         *finger |= BIT(7);
    finger ++;
  }

  status = sendZdoCommand(destination, LEAVE_REQUEST, frame, finger - &frame[0]);
                          
 done:
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Mgmt leave",
                     status);
}

// plugin test-harness z3 mgmt nwk-update-request <scanChannel:2> <scanDuration:2>
// <scanCount:1> <dstShort:2> <option:4>
void emAfPluginTestHarnessZ3MgmtNwkUpdateRequestCommand(void)
{
  uint16_t scanChannel    = (uint16_t)emberUnsignedCommandArgument(0);
  uint16_t scanDuration   = (uint16_t)emberUnsignedCommandArgument(1);
  uint8_t scanCount       = (uint8_t)emberUnsignedCommandArgument(2);
  EmberNodeId destination = (EmberNodeId)emberUnsignedCommandArgument(3);
  uint32_t options        = emAfPluginTestHarnessZ3GetSignificantBit(4);
  EmberStatus status;
  uint8_t frame[9];
  uint8_t *finger = &frame[1];
  EmberNetworkParameters networkParameters;
  EmberNodeType nodeType;

  status = emberAfGetNetworkParameters(&nodeType, &networkParameters);
  if (status != EMBER_SUCCESS) {
    goto done;
  }

  // Scan channel.
  emberAfCopyInt32u(finger, 0, BIT32(scanChannel));
  finger += sizeof(uint32_t);
  // Scan duration.
  *finger++ = (uint8_t)(0xFF & scanDuration); // ANDREW: Q3.
  // Scan count (table 2.90):
  // "This field shall be present only if the ScanDuration is within the
  // range of 0x00 to 0x05."
  if (scanDuration <= 5) {
    *finger++ = scanCount;
  }
  // Network update id (table 2.90):
  // "This field shall only be present of the ScanDuration is 0xfe or 0xff.
  // If the ScanDuration is 0xff, then the value in the nwkUpdateID shall
  // be ignored."
  if (scanDuration == 0xFF || scanDuration == 0xFE) {
    *finger++ = (networkParameters.nwkUpdateId
                 - (options == BIT(0)
                    ? 1
                    : (options == BIT(1)
                       ? -1
                       : 0)));
  }
  // Network manager id (table 2.90):
  // "This field shall be present only if the ScanDuration is set to 0xff,
  // and, where present, indicates the NWK address for the device with the
  // Network Manager bit set in its Node Descriptor."
  if (scanDuration == 0xFF) {
    *finger++ = networkParameters.nwkManagerId;
  }

  status = sendZdoCommand(destination,
                          ZDO_NETWORK_UPDATE_REQUEST,
                          frame,
                          finger - &frame[0]);

 done:
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Network update request",
                     status);
}

// plugin test-harness z3 zdo zdo-reset
void emAfPluginTestHarnessZ3ZdoZdoResetCommand(void)
{
  currentZdoNegativeCommandCluster = NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER;
  negativeZdoCommandFlags = 0;

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "ZDO reset",
                     EMBER_SUCCESS);
}

// plugin test-harness z3 zdo zdo-node-desc-rsp-config <options:4>
void emAfPluginTestHarnessZ3ZdoZdoNodeDescRspConfigCommand(void)
{
  EmberStatus status = EMBER_INVALID_CALL;

#ifndef EZSP_HOST

  uint32_t options = emAfPluginTestHarnessZ3GetSignificantBit(0);

  currentZdoNegativeCommandCluster = NODE_DESCRIPTOR_RESPONSE;

  if (options == BIT(0)) {
    // Shh...
    extern bool emEnableR21StackBehavior;
    emEnableR21StackBehavior = false;
    negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE
                               | NEGATIVE_ZDO_COMMAND_FLAGS_NO_NEGATIVE_RESPONSE);
    status = EMBER_SUCCESS;
  } else {
    status = EMBER_BAD_ARGUMENT;
  }

#endif /* EZSP_HOST */

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Node descriptor response config",
                     status);
}

// plugin test-harness z3 zdo active-endpoint-request <dstShort:2>
// <nwkAddrOfInterest:2> <options:4>
void emAfPluginTestHarnessZ3ZdoActiveEndpointRequestCommand(void)
{
  EmberNodeId destination       = (EmberNodeId)emberUnsignedCommandArgument(0);
  EmberNodeId nwkAddrOfInterest = (EmberNodeId)emberUnsignedCommandArgument(1);
  uint32_t options              = emAfPluginTestHarnessZ3GetSignificantBit(2);
  EmberStatus status;
  uint8_t frame[3], frameLength = 1;

  if (options != BIT(0)) {
    frame[1] = LOW_BYTE(nwkAddrOfInterest);
    frame[2] = HIGH_BYTE(nwkAddrOfInterest);
    frameLength += 2;
  }

  status = sendZdoCommand(destination,
                          ACTIVE_ENDPOINTS_REQUEST,
                          frame,
                          frameLength);

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Active endpoint request",
                     status);
}

// plugin test-harness z3 zdo match-desc-req <dstShort:2>
// <nwkAddrOfInterest:2> <profileId:4> <options:4>
void emAfPluginTestHarnessZ3ZdoMatchDescReqCommand(void)
{
  EmberNodeId destination       = (EmberNodeId)emberUnsignedCommandArgument(0);
  EmberNodeId nwkAddrOfInterest = (EmberNodeId)emberUnsignedCommandArgument(1);
  uint16_t profileId            = (uint16_t)emberUnsignedCommandArgument(2);
  uint32_t options              = emAfPluginTestHarnessZ3GetSignificantBit(3);
  EmberStatus status;
  uint8_t frame[32];
  uint8_t *finger = &frame[1];

  if (options != BIT(0)) {
    // Network address of interest.
    emberAfCopyInt16u(finger, 0, nwkAddrOfInterest);
    finger += sizeof(uint16_t);
  }

  // Profile id.
  emberAfCopyInt16u(finger, 0, profileId);
  finger += sizeof(uint16_t);

  if (options != BIT(1)) {
    uint8_t reallyInClustersCount
      = (options == BIT(2) ? 1 : emAfCliInClCount);
    uint8_t reallyOutClustersCount
      = (options == BIT(3) ? 1 : emAfCliOutClCount);

    // In clusters.
    *finger++ = emAfCliInClCount;
    MEMMOVE(finger,
            emAfCliZdoInClusters,
            reallyInClustersCount * sizeof(uint16_t));
    finger += (reallyInClustersCount * sizeof(uint16_t));

    // Out clusters.
    *finger++ = emAfCliOutClCount;
    MEMMOVE(finger,
            emAfCliZdoOutClusters,
            reallyOutClustersCount * sizeof(uint16_t));
    finger += (reallyOutClustersCount * sizeof(uint16_t));
  } else {
    *finger++ = emAfCliInClCount;  // In clusters.
    *finger++ = emAfCliOutClCount; // Out clusters.
  }

  status = sendZdoCommand(destination,
                          MATCH_DESCRIPTORS_REQUEST,
                          frame,
                          finger - &frame[0]);

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Match descriptor request",
                     status);
}

// plugin test-harness z3 zdo match-desc-rsp-config <nwkAddrOfInterest:2>
// <status:1> <options:4>
void emAfPluginTestHarnessZ3ZdoMatchDescRspConfigCommand(void)
{
  EmberNodeId nwkAddrOfInterest = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t zdoStatus             = (uint8_t)emberUnsignedCommandArgument(1);
  uint32_t options              = (uint32_t)emAfPluginTestHarnessZ3GetSignificantBit(2);
  EmberStatus status = EMBER_SUCCESS;

  // Global state.
  currentZdoNegativeCommandCluster = MATCH_DESCRIPTORS_RESPONSE;
  currentZdoNegativeCommandFrameLength = 5;

  // Positive behavior.
  currentZdoNegativeCommandFrame[1] = zdoStatus;
  currentZdoNegativeCommandFrame[2] = LOW_BYTE(nwkAddrOfInterest);
  currentZdoNegativeCommandFrame[3] = HIGH_BYTE(nwkAddrOfInterest);

  switch (options) {
  case BIT(0):
    // match list absent
    negativeZdoCommandFlags
      = (NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT);
    break;
  case BIT(1):
    // second response device not found
    negativeZdoCommandFlags
      = (NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT
         | NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE);
    currentZdoNegativeCommandFrame[1] = EMBER_ZDP_DEVICE_NOT_FOUND;
    break;
  case BIT(2):
    // second response no descriptor
    negativeZdoCommandFlags
      = (NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT
         | NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE);
    currentZdoNegativeCommandFrame[1] = EMBER_ZDP_NO_DESCRIPTOR;
    break;
  default:
    status = EMBER_BAD_ARGUMENT;
    break;
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Match descriptor response config",
                     status);
}

// plugin test-harness z3 zdo simple-desc-req <dstShort:2> <dstEndpoint:1>
// <nwkAddrOfInterest:2> <options:4>
void emAfPluginTestHarnessZ3ZdoSimpleDescReqCommand(void)
{
  EmberNodeId destination       = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t endpoint              = (uint8_t)emberUnsignedCommandArgument(1);
  EmberNodeId nwkAddrOfInterest = (EmberNodeId)emberUnsignedCommandArgument(2);
  uint32_t options              = emAfPluginTestHarnessZ3GetSignificantBit(3);
  EmberStatus status;
  uint8_t frame[4];
  uint8_t *finger = &frame[1];

  // Network address of interest.
  if (options != BIT(0)) {
    emberAfCopyInt16u(finger, 0, nwkAddrOfInterest);
    finger += sizeof(uint16_t);
  }

  // Endpoint.
  if (options != BIT(1)) {
    *finger++ = endpoint;
  }

  status = sendZdoCommand(destination,
                          SIMPLE_DESCRIPTOR_REQUEST,
                          frame,
                          finger - &frame[0]);

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Simple descriptor request",
                     status);
}

// plugin test-harness z3 simple-desc-rsp-config <nwkAddrOfInterest:2>
// <status:1> <length:1> <options:4>
void emAfPluginTestHarnessZ3ZdoSimpleDescRspConfigCommand(void)
{
  EmberNodeId nwkAddrOfInterest = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t zdoStatus             = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t length                = (uint8_t)emberUnsignedCommandArgument(2);
  uint32_t options              = (uint32_t)emAfPluginTestHarnessZ3GetSignificantBit(3);

  EmberStatus status = EMBER_SUCCESS;

  // Global state.
  currentZdoNegativeCommandCluster = SIMPLE_DESCRIPTOR_RESPONSE;
  currentZdoNegativeCommandFrameLength = 5;

  // Positive behavior.
  currentZdoNegativeCommandFrame[1] = zdoStatus;
  emberAfCopyInt16u(currentZdoNegativeCommandFrame, 2, nwkAddrOfInterest);
  currentZdoNegativeCommandFrame[4] = length;

  // Negative behavior.
  switch (options) {
  case BIT(0):
    // simple descriptor field absent
    negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR);
    break;
  case BIT(1):
    // do not forward to the nwk addr of interest
    break;
  case BIT(2):
    // cluster list key establishment
    negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_KE);
    break;
  case BIT(3):
    // swap support for client/server clusters
    negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_SWAP);
    break;
  case BIT(4):
    // do not send any cluster lists
    negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_ABSENT);
    break;
  case BIT(5):
    // send a positive command alongside a negative command
    negativeZdoCommandFlags
      = (NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE
         | NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR);
    currentZdoNegativeCommandFrame[1] = EMBER_ZDP_DEVICE_NOT_FOUND;
    currentZdoNegativeCommandFrame[4] = 0; // length
    break;
  case BIT(6):
    // do not include endpoints fields in the variable payload
    negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_NO_ENDPOINT_FIELDS);
    break;
  case BIT(7):
    // do not include the network address of interest
    negativeZdoCommandFlags = (NEGATIVE_ZDO_COMMAND_FLAGS_NO_NWK_ADDR);
    currentZdoNegativeCommandFrameLength -= 2;
    break;
  default:
    ; // options of 0x00 is ok
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Simple descriptor response config",
                     status);
}

// plugin test-harness z3 zdo bind-group <shortAddress:2> <srcEndpoint:1>
// <dstEndpoint:1> <dstAddress:2> <cluster:2> <srcIeee:8>
void emAfPluginTestHarnessZ3ZdoBindGroupCommand(void)
{
  EmberNodeId shortAddress = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint      = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint      = (uint8_t)emberUnsignedCommandArgument(2);
  uint16_t groupId         = (uint16_t)emberUnsignedCommandArgument(3);
  EmberAfClusterId cluster = (EmberAfClusterId)emberUnsignedCommandArgument(4);
  EmberStatus status;
  uint8_t frame[16];
  uint8_t *finger = &frame[1];

  // Not currently used.
  (void)dstEndpoint;

  // SrcAddress
  finger += emberCopyBigEndianEui64Argument(5, finger);

  // SrcEndp
  *finger++ = srcEndpoint;

  // ClusterID
  emberAfCopyInt16u(finger, 0, cluster);
  finger += sizeof(cluster);

  // DstAddrMode
  *finger++ = 0x01; // multicast

  // DstAddress
  emberAfCopyInt16u(finger, 0, groupId);
  finger += sizeof(groupId);

  status = sendZdoCommand(shortAddress,
                          BIND_REQUEST,
                          frame,
                          finger - &frame[0]);

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "ZDO bind group",
                     status);
}

// plugin test-harness z3 zdo nwk-addr-req <ieee:8> <requestType:1>
// <startIndex:1> <dstShort:2> <options:4>
void emAfPluginTestHarnessZ3ZdoNwkAddrReqCommand(void)
{
  uint8_t requestType          = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t startIndex           = (uint8_t)emberUnsignedCommandArgument(2);
  EmberNodeId destinationShort = (EmberNodeId)emberUnsignedCommandArgument(3);
  uint32_t options             = emAfPluginTestHarnessZ3GetSignificantBit(4);
  EmberStatus status;
  uint8_t frame[11];
  uint8_t *finger = &frame[1];

  if (options != BIT(0)) {
    // IEEE address.
    emberCopyBigEndianEui64Argument(0, finger);
    finger += EUI64_SIZE;

    // Request type.
    *finger++ = requestType;

    // Start index.
    *finger++ = startIndex;
  }

  status = sendZdoCommand(destinationShort,
                          NETWORK_ADDRESS_REQUEST,
                          frame,
                          finger - &frame[0]);

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Network address request",
                     status);
}

// plugin test-harness z3 zdo ieee-addr-req <nwkAddrOfInterest:2>
// <requestType:1> <startIndex:1> <dstShort:2> <options:4>
void emAfPluginTestHarnessZ3ZdoIeeeAddrReqCommand(void)
{
  EmberNodeId nwkAddrOfInterest = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t requestType           = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t startIndex            = (uint8_t)emberUnsignedCommandArgument(2);
  EmberNodeId destination       = (EmberNodeId)emberUnsignedCommandArgument(3);
  uint32_t options              = emAfPluginTestHarnessZ3GetSignificantBit(4);
  EmberStatus status;
  uint8_t frame[5];
  uint8_t *finger = &frame[1];

  if (options != BIT(0)) {
    // Network address of interest.
    emberAfCopyInt16u(finger, 0, nwkAddrOfInterest);
    finger += sizeof(uint16_t);

    // Request type.
    *finger++ = requestType;

    // Start index.
    *finger++ = startIndex;
  }

  status = sendZdoCommand(destination,
                          IEEE_ADDRESS_REQUEST,
                          frame,
                          finger - &frame[0]);

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "IEEE address request",
                     status);
}

// plugin test-harness z3 nwk ieee-addr-rsp-config reset
// plugin test-harness z3 nwk ieee-addr-rsp-config issuer-nwk-address-remote-dev
// plugin test-harness z3 nwk ieee-addr-rsp-config status-device-not-found
void emAfPluginTestHarnessZ3ZdoIeeeAddrRspConfigCommand(void)
{
  EmberStatus status = EMBER_SUCCESS;
  char firstChar = emberStringCommandArgument(-1, NULL)[0];

  // Global state.
  currentZdoNegativeCommandCluster = IEEE_ADDRESS_RESPONSE;
  currentZdoNegativeCommandFrameLength = 12;

  // Positive behavior.
  currentZdoNegativeCommandFrame[1] = EMBER_ZDP_SUCCESS;

  // Negative behavior.
  switch (firstChar) {
  case 'r':
    // reset
    negativeZdoCommandFlags = 0;
    currentZdoNegativeCommandCluster = NULL_CURRENT_ZDO_NEGATIVE_COMMAND_CLUSTER;
    break;
  case 'i':
    // issuer-nwk-address-remote-dev
    negativeZdoCommandFlags |= NEGATIVE_ZDO_COMMAND_FLAGS_OUR_NWK_ADDRESS;
    break;
  case 's':
    // status-device-not-found
    currentZdoNegativeCommandFrame[1] = EMBER_ZDP_DEVICE_NOT_FOUND;
    break;
  default:
    status = EMBER_BAD_ARGUMENT;
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "IEEE address response config",
                     status);
}

#ifndef EZSP_HOST

EmberStatus emAfPluginTestHarnessZ3ZdoCommandResponseHandler(EmberMessageBuffer requestBuffer,
                                                             uint8_t commandIndex,
                                                             EmberApsFrame *apsFrame)
{
  EmberStatus status = EMBER_SUCCESS;
  uint16_t clusterId = apsFrame->clusterId;

  // Make sure the cluster id matches the negative zdo command's request.
  if (clusterId != (currentZdoNegativeCommandCluster & ~0x8000)) {
    return EMBER_INVALID_CALL;
  }

  // Perform some last second mangling of the frame.
  if (clusterId == IEEE_ADDRESS_REQUEST) {
    EmberNodeId nwkAddressOfInterest
      = HIGH_LOW_TO_INT(emberGetLinkedBuffersByte(requestBuffer,
                                                  commandIndex + 2),
                        emberGetLinkedBuffersByte(requestBuffer,
                                                  commandIndex + 1));
    if (negativeZdoCommandFlags & NEGATIVE_ZDO_COMMAND_FLAGS_OUR_NWK_ADDRESS) {
      currentZdoNegativeCommandFrame[10] = LOW_BYTE(emberAfGetNodeId());
      currentZdoNegativeCommandFrame[11] = HIGH_BYTE(emberAfGetNodeId());
    } else {
      currentZdoNegativeCommandFrame[10] = LOW_BYTE(nwkAddressOfInterest);
      currentZdoNegativeCommandFrame[11] = HIGH_BYTE(nwkAddressOfInterest);
    }
    status = emberLookupEui64ByNodeId(nwkAddressOfInterest,
                                      &currentZdoNegativeCommandFrame[2]);
  } else if (clusterId == MATCH_DESCRIPTORS_REQUEST) {
    if (negativeZdoCommandFlags
        & NEGATIVE_ZDO_COMMAND_FLAGS_MATCH_LIST_ABSENT) {
      // Set the match length to 0.
      currentZdoNegativeCommandFrame[4] = 0;
      currentZdoNegativeCommandFrameLength = 5;
    } else {
      // TODO: fill the match desc response.
    }
  } else if (clusterId == SIMPLE_DESCRIPTOR_REQUEST
             && !(negativeZdoCommandFlags
                  & NEGATIVE_ZDO_COMMAND_FLAGS_NO_SIMPLE_DESCRIPTOR)) {
    // TODO: do we need these to be legit?
    uint8_t inClusterCount = 1, outClusterCount = 2, swapClusters;
    uint16_t inCluster0 = 0x0000, outCluster0 = 0x0100, outCluster1 = 0x0101;
    uint8_t endpoint
      = emberGetLinkedBuffersByte(requestBuffer, commandIndex + 3);
    uint8_t endpointIndex
      = emberAfIndexFromEndpoint(endpoint);

    if (endpointIndex != 0xFF) {
      if (!(negativeZdoCommandFlags
            & NEGATIVE_ZDO_COMMAND_FLAGS_NO_ENDPOINT_FIELDS)) {
        // endpoint
        currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
          = endpoint;
      }

      // profile id
      emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                        currentZdoNegativeCommandFrameLength,
                        emberAfProfileIdFromIndex(endpointIndex));
      currentZdoNegativeCommandFrameLength += sizeof(uint16_t);

      // device id
      emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                        currentZdoNegativeCommandFrameLength,
                        emberAfDeviceIdFromIndex(endpointIndex));
      currentZdoNegativeCommandFrameLength += sizeof(uint16_t);

      // device version
      currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
        = emberAfDeviceVersionFromIndex(endpointIndex);

      swapClusters = (negativeZdoCommandFlags
                      & NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_SWAP);
      if (negativeZdoCommandFlags & NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_KE) {
        inCluster0 = outCluster0 = outCluster1 = ZCL_KEY_ESTABLISHMENT_CLUSTER_ID;
      }

      // input cluster count
      currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
        = (swapClusters ? outClusterCount : inClusterCount);
      if (!(negativeZdoCommandFlags
           & NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_ABSENT)) {
        if (swapClusters) {
          emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                            currentZdoNegativeCommandFrameLength,
                            outCluster0);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
          emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                            currentZdoNegativeCommandFrameLength,
                            outCluster1);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
        } else {
          emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                            currentZdoNegativeCommandFrameLength,
                            inCluster0);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
        }
      }

      // output cluster count
      currentZdoNegativeCommandFrame[currentZdoNegativeCommandFrameLength++]
        = (swapClusters ? inClusterCount : outClusterCount);
      if (!(negativeZdoCommandFlags
            & NEGATIVE_ZDO_COMMAND_FLAGS_CLUSTER_LIST_ABSENT)) {
        if (swapClusters) {
          emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                            currentZdoNegativeCommandFrameLength,
                            inCluster0);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
        } else {
          emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                            currentZdoNegativeCommandFrameLength,
                            outCluster0);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
          emberAfCopyInt16u(currentZdoNegativeCommandFrame,
                            currentZdoNegativeCommandFrameLength,
                            outCluster1);
          currentZdoNegativeCommandFrameLength += sizeof(uint16_t);
        }
      }

      if (negativeZdoCommandFlags & NEGATIVE_ZDO_COMMAND_FLAGS_NO_NWK_ADDR) {
        currentZdoNegativeCommandFrame[2] // simple descriptor length
          = (currentZdoNegativeCommandFrameLength - 3);
      } else {
        currentZdoNegativeCommandFrame[4] // simple descriptor length
          = (currentZdoNegativeCommandFrameLength - 5);
      }
    } else {
      status = EMBER_ERR_FATAL;
    }
  }

  if (status == EMBER_SUCCESS) {
    // We use an event for this command so that the positive stuff will go
    // out of the radio first.
    currentZdoNegativeCommandDestination = emCurrentSender;
    emberEventControlSetActive(emberAfPluginTestHarnessZ3ZdoSendEventControl);
  }

  // Maybe change the cluster id to an invalid cluster so we don't process the
  // ZDO command.
  if (!(negativeZdoCommandFlags
        & NEGATIVE_ZDO_COMMAND_FLAGS_SEND_POSITIVE_RESPONSE)) {
    apsFrame->clusterId = 0xFFFF;
  }

  return status;
}

#endif /* EZSP_HOST */

void emberAfPluginTestHarnessZ3ZdoSendEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginTestHarnessZ3ZdoSendEventControl);

  if (!(negativeZdoCommandFlags
        & NEGATIVE_ZDO_COMMAND_FLAGS_NO_NEGATIVE_RESPONSE)) {
    emberAfCorePrintln("%p: %p: 0x%X",
                       TEST_HARNESS_Z3_PRINT_NAME,
                       "ZDO send event handler",
                       sendZdoCommand(currentZdoNegativeCommandDestination,
                                      currentZdoNegativeCommandCluster,
                                      currentZdoNegativeCommandFrame,
                                      currentZdoNegativeCommandFrameLength));
  }

  // After the command is sent, we reset.
  emAfPluginTestHarnessZ3ZdoZdoResetCommand();

#ifndef EZSP_HOST
  extern bool emEnableR21StackBehavior;
  emEnableR21StackBehavior = true;
#endif /* EZSP_HOST */
}

// *******************************************************************
//  security.h
//
//  functions for manipulating security for Trust Center and 
//  non Trust Center nodes using the commercial security library.
//
//  Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#if !defined emberKeyContents
  #define emberKeyContents(key) (((EmberKeyData*)(key))->contents)
#endif

// Trust Center Functions
bool trustCenterInit(EmberKeyData* preconfiguredKey, 
                        EmberKeyData* networkKey);
void trustCenterPermitJoins(bool allow);
bool trustCenterIsPermittingJoins(void);

// Non Trust Center functions
bool nodeSecurityInit(EmberKeyData* preconfiguredKey);

// Common functions
extern uint8_t addressCacheSize;
#define securityAddressCacheGetSize() (addressCacheSize+0)
void securityAddressCacheInit(uint8_t securityAddressCacheStartIndex,
                              uint8_t securityAddressCacheSize);
void securityAddToAddressCache(EmberNodeId nodeId, EmberEUI64 nodeEui64);


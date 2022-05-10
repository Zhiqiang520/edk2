/** @file
  EDKII SpdmIo Stub

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmStub.h"
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Test/TestConfig.h>

#define SLOT_NUMBER    2

SPDM_MESSAGE_HEADER  *mSpdmIoLastSpdmRequest;
UINTN                mSpdmIoLastSpdmRequestSize;

BOOLEAN mSendReceiveBufferAcquired = FALSE;
UINT8 mSendReceiveBuffer[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
UINTN mSendReceiveBufferSize;
VOID *mScratchBuffer;

EFI_STATUS
EFIAPI
SpdmIoSendMessage (
  IN     SPDM_IO_PROTOCOL                       *This,
  IN     UINTN                                  MessageSize,
  IN CONST VOID                                 *Message,
  IN     UINT64                                 Timeout
  )
{
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;
  VOID                      *SpdmContext;

  SpdmTestContext = SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_IO_PROTOCOL(This);
  SpdmContext = SpdmTestContext->SpdmContext;

  if (Message == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (MessageSize == 0) {
    return EFI_INVALID_PARAMETER;
  }
  if (mSpdmIoLastSpdmRequest != NULL) {
    FreePool (mSpdmIoLastSpdmRequest);
    mSpdmIoLastSpdmRequest = NULL;
  }

  mSpdmIoLastSpdmRequestSize = MessageSize;
  mSpdmIoLastSpdmRequest = AllocateCopyPool (MessageSize, Message);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpdmIoReceiveMessage (
  IN     SPDM_IO_PROTOCOL                       *This,
  IN OUT UINTN                                  *MessageSize,
     OUT VOID                                   **Message,
  IN     UINT64                                 Timeout
  )
{
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;
  VOID                      *SpdmContext;
  UINT32                    *SessionId;
  BOOLEAN                   IsAppMessage;
  EFI_STATUS                Status;
  UINT32                    TmpSessionId;
  UINT32                    *SessionIdPtr;

  SpdmTestContext = SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_IO_PROTOCOL(This);
  SpdmContext = SpdmTestContext->SpdmContext;

  SessionId = NULL;

  Status = SpdmProcessRequest (SpdmContext, &SessionId, &IsAppMessage,
                               mSpdmIoLastSpdmRequestSize, mSpdmIoLastSpdmRequest);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "SpdmProcessRequest - %r\n", Status));
    return Status;
  }

  if(SessionId != NULL) {
    TmpSessionId = *SessionId;
    SessionIdPtr = &TmpSessionId;
  } else {
    SessionIdPtr = NULL;
  }

  ZeroMem (*Message, *MessageSize);
  Status = SpdmBuildResponse (SpdmContext, SessionIdPtr, IsAppMessage, MessageSize, Message);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "SpdmBuildResponse - %r\n", Status));
    return Status;
  }

  return Status;
}

SPDM_TEST_DEVICE_CONTEXT  mSpdmTestDeviceContext = {
  SPDM_TEST_DEVICE_CONTEXT_SIGNATURE,
  NULL,
  {
    SpdmIoSendMessage,
    SpdmIoReceiveMessage,
  },
};

RETURN_STATUS
SpdmDeviceAcquireSenderBuffer (
    VOID *Context, UINTN *MaxMsgSize, VOID **MsgBufPtr)
{
    ASSERT (!mSendReceiveBufferAcquired);
    *MaxMsgSize = sizeof(mSendReceiveBuffer);
    *MsgBufPtr = mSendReceiveBuffer;
    ZeroMem (mSendReceiveBuffer, sizeof(mSendReceiveBuffer));
    mSendReceiveBufferAcquired = TRUE;

    return RETURN_SUCCESS;
}

VOID SpdmDeviceReleaseSenderBuffer (
    VOID *Context, CONST VOID *MsgBufPtr)
{
    ASSERT (mSendReceiveBufferAcquired);
    ASSERT (MsgBufPtr == mSendReceiveBuffer);
    mSendReceiveBufferAcquired = FALSE;

    return;
}

RETURN_STATUS SpdmDeviceAcquireReceiverBuffer (
    VOID *Context, UINTN *MaxMsgSize, VOID **MsgBufPtr)
{
    ASSERT (!mSendReceiveBufferAcquired);
    *MaxMsgSize = sizeof(mSendReceiveBuffer);
    *MsgBufPtr = mSendReceiveBuffer;
    ZeroMem (mSendReceiveBuffer, sizeof(mSendReceiveBuffer));
    mSendReceiveBufferAcquired = TRUE;

    return RETURN_SUCCESS;
}

VOID SpdmDeviceReleaseReceiverBuffer (
    VOID *context, CONST VOID *MsgBufPtr)
{
    ASSERT (mSendReceiveBufferAcquired);
    ASSERT (MsgBufPtr == mSendReceiveBuffer);
    mSendReceiveBufferAcquired = FALSE;

    return ;
}

EFI_STATUS
EFIAPI
MainEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                        Status;
  UINT8                             Index;
  VOID                              *CertChain;
  UINTN                             CertChainSize;
  EFI_SIGNATURE_LIST                *SignatureList;
  UINTN                             SignatureListSize;
  VOID                              *SpdmContext;
  SPDM_DATA_PARAMETER               Parameter;
  UINT8                             Data8;
  UINT16                            Data16;
  UINT32                            Data32;
  BOOLEAN                           HasRspPubCert;
  BOOLEAN                           HasRspPrivKey;
  UINTN                             ScratchBufferSize;
  UINT8                             TestConfig;
  UINTN                             TestConfigSize;

  Status = gRT->GetVariable (
                  L"SpdmTestConfig",
                  &gEfiDeviceSecurityPkgTestConfig,
                  NULL,
                  &TestConfigSize,
                  &TestConfig
                  );

  SpdmContext = AllocateZeroPool (SpdmGetContextSize());
  ASSERT(SpdmContext != NULL);
  SpdmInitContext (SpdmContext);

  ScratchBufferSize = SpdmGetSizeofRequiredScratchBuffer(SpdmContext);
  mScratchBuffer = AllocateZeroPool(ScratchBufferSize);
  ASSERT(mScratchBuffer != NULL);

  mSpdmTestDeviceContext.SpdmContext = SpdmContext;
  SpdmRegisterDeviceIoFunc (SpdmContext, SpdmDeviceSendMessage, SpdmDeviceReceiveMessage);
//  SpdmRegisterTransportLayerFunc (SpdmContext, SpdmTransportMctpEncodeMessage, SpdmTransportMctpDecodeMessage);
  SpdmRegisterTransportLayerFunc (SpdmContext, SpdmTransportPciDoeEncodeMessage,
                                  SpdmTransportPciDoeDecodeMessage, SpdmTransportPciDoeGetHeaderSize);
  SpdmRegisterDeviceBufferFunc (SpdmContext, SpdmDeviceAcquireSenderBuffer, SpdmDeviceReleaseSenderBuffer,
                                SpdmDeviceAcquireReceiverBuffer, SpdmDeviceReleaseReceiverBuffer);
  SpdmSetScratchBuffer (SpdmContext, mScratchBuffer, ScratchBufferSize);

  Status = GetVariable2 (
             EDKII_DEVICE_SECURITY_DATABASE,
             &gEdkiiDeviceSignatureDatabaseGuid,
             &SignatureList,
             &SignatureListSize
             );
  if (!EFI_ERROR(Status)) {
    HasRspPubCert = TRUE;
    // BUGBUG: Assume only 1 SPDM cert.
    ASSERT (CompareGuid (&SignatureList->SignatureType, &gEdkiiCertSpdmCertChainGuid));
    ASSERT (SignatureList->SignatureListSize == SignatureList->SignatureListSize);
    ASSERT (SignatureList->SignatureHeaderSize == 0);
    ASSERT (SignatureList->SignatureSize == SignatureList->SignatureListSize - (sizeof(EFI_SIGNATURE_LIST) + SignatureList->SignatureHeaderSize));
    CertChain = (VOID *)((UINT8 *)SignatureList +
                         sizeof(EFI_SIGNATURE_LIST) +
                         SignatureList->SignatureHeaderSize +
                         sizeof(EFI_GUID));
    CertChainSize = SignatureList->SignatureSize - sizeof(EFI_GUID);

    ZeroMem (&Parameter, sizeof(Parameter));
    Parameter.location = SpdmDataLocationLocal;
    Data8 = SLOT_NUMBER;
    SpdmSetData (SpdmContext, SpdmDataLocalSlotCount, &Parameter, &Data8, sizeof(Data8));

    for (Index = 0; Index < SLOT_NUMBER; Index++) {
      Parameter.additional_data[0] = Index;
      SpdmSetData (SpdmContext, SpdmDataLocalPublicCertChain, &Parameter, CertChain, CertChainSize);
    }
    // do not free it
  } else {
    HasRspPubCert = FALSE;
  }

  HasRspPrivKey = TRUE;

  Data32 = SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_NO_SIG |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_SIG |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_ENCRYPT_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MAC_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MUT_AUTH_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_KEY_EX_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_PSK_CAP_RESPONDER |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_PSK_CAP_RESPONDER_WITH_CONTEXT |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_ENCAP_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_HBEAT_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_KEY_UPD_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_HANDSHAKE_IN_THE_CLEAR_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_PUB_KEY_ID_CAP |
           0;
  if (!HasRspPubCert) {
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
  } else {
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
  }
  if (!HasRspPrivKey) {
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP;
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_SIG;
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_NO_SIG;
  } else {
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP;
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_SIG;
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_NO_SIG;
  }

  if (TestConfig == TEST_CONFIG_NO_CERT_CAP) {
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
  } else if (TestConfig == TEST_CONFIG_NO_CHAL_CAP) {
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP;
  }
  SpdmSetData (SpdmContext, SpdmDataCapabilityFlags, &Parameter, &Data32, sizeof(Data32));

  Data8 = SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;
  SpdmSetData (SpdmContext, SpdmDataMeasurementSpec, &Parameter, &Data8, sizeof(Data8));
  Data32 = SPDM_ALGORITHMS_MEASUREMENT_HASH_ALGO_TPM_ALG_SHA_256;
  SpdmSetData (SpdmContext, SpdmDataMeasurementHashAlgo, &Parameter, &Data32, sizeof(Data32));
  Data32 = SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_2048;
  SpdmSetData (SpdmContext, SpdmDataBaseAsymAlgo, &Parameter, &Data32, sizeof(Data32));
  Data32 = SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_256;
  SpdmSetData (SpdmContext, SpdmDataBaseHashAlgo, &Parameter, &Data32, sizeof(Data32));
  Data16 = SPDM_ALGORITHMS_DHE_NAMED_GROUP_FFDHE_2048;
  SpdmSetData (SpdmContext, SpdmDataDHENamedGroup, &Parameter, &Data16, sizeof(Data16));
  Data16 = SPDM_ALGORITHMS_KEY_SCHEDULE_HMAC_HASH;
  SpdmSetData (SpdmContext, SpdmDataAEADCipherSuite, &Parameter, &Data16, sizeof(Data16));
  Data16 = SPDM_ALGORITHMS_KEY_SCHEDULE_HMAC_HASH;
  SpdmSetData (SpdmContext, SpdmDataKeySchedule, &Parameter, &Data16, sizeof(Data16));
  Data8 = SPDM_ALGORITHMS_OPAQUE_DATA_FORMAT_1;
  SpdmSetData (SpdmContext, SpdmDataOtherParamsSsupport, &Parameter, &Data8, sizeof(Data8));

  Status = gBS->InstallProtocolInterface (
                  &mSpdmTestDeviceContext.SpdmHandle,
                  &gSpdmIoProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSpdmTestDeviceContext.SpdmIoProtocol
                  );

  InitializeSpdmTest (&mSpdmTestDeviceContext);

  return Status;
}

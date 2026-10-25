/** @file
  Android Boot State manipulation tool

  Copyright (c) 2024 rodriguezst (https://github.com/rodriguezst)

  SPDX-License-Identifier: MIT
**/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>

#define MAGIC_ADDRESS  0x17FE00000ULL
#define MAGIC_VALUE    0xC0DEADDEDULL

/**
  Map the page holding the magic address into the MMU as uncached memory.

  Firmwares such as Project Mu only map the DRAM they declare, so a bare
  store to a high physical address faults under the enabled MMU and never
  reaches RAM. Mapping it uncached (best effort) both avoids the fault and
  makes the store land in DRAM directly, so the value survives the warm
  reset. Failures are ignored on purpose: on firmwares that already map the
  region the plain store below keeps working.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
**/
STATIC
VOID
EnsureMagicAddressMapped (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_PHYSICAL_ADDRESS  Base;
  UINT64                Length;

  if (gDS == NULL) {
    return;
  }

  Base   = (EFI_PHYSICAL_ADDRESS)(MAGIC_ADDRESS & ~(UINT64)EFI_PAGE_MASK);
  Length = EFI_PAGE_SIZE;

  gDS->AddMemorySpace (
         EfiGcdMemoryTypeSystemMemory,
         Base,
         Length,
         EFI_MEMORY_UC | EFI_MEMORY_WB
         );

  gDS->AllocateMemorySpace (
         EfiGcdAllocateAddress,
         EfiGcdMemoryTypeSystemMemory,
         0,
         Length,
         &Base,
         ImageHandle,
         NULL
         );

  gDS->SetMemorySpaceAttributes (Base, Length, EFI_MEMORY_UC);
}

/**
  The user Entry Point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point executed successfully.
  @retval other            Some error occurred.
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  // Make sure the magic address is mapped before touching it (see above)
  EnsureMagicAddressMapped (ImageHandle);

  // Write the magic keyword at 0x17FE00000 address
  *(volatile UINT64 *)MAGIC_ADDRESS = MAGIC_VALUE;

  // Stall for 0.5 seconds
  gBS->Stall (500000);

  // Perform a warm reset
  gRT->ResetSystem (
         EfiResetWarm,
         EFI_SUCCESS,
         0,
         NULL
         );

  return EFI_SUCCESS;
}

/** @file
  PStore diagnostic probe (non-interactive).

  Prints the current dual-boot magic value at the pstore region, writes the
  magic, reads it back, then performs a warm reset. Run it again from the boot
  manager after the reboot to see whether the magic survived the reset.

  Copyright (c) 2024 rodriguezst (https://github.com/rodriguezst)

  SPDX-License-Identifier: MIT
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#define MAGIC_ADDRESS  0x17FE00000ULL
#define MAGIC_VALUE    0xC0DEADDEDULL

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT64  Value;
  UINT64  ReadBack;

  Value = *(volatile UINT64 *)MAGIC_ADDRESS;

  Print (L"\r\n===== PStoreProbe =====\r\n");
  Print (L"PStore @ 0x%lx\r\n", (UINT64)MAGIC_ADDRESS);
  Print (L"CURRENT VALUE = 0x%lx\r\n", Value);
  if (Value == MAGIC_VALUE) {
    Print (L">>> MAGIC PRESENT: value SURVIVED the last reset\r\n");
  } else {
    Print (L">>> no magic: value did NOT survive (or first run)\r\n");
  }

  *(volatile UINT64 *)MAGIC_ADDRESS = MAGIC_VALUE;
  ReadBack = *(volatile UINT64 *)MAGIC_ADDRESS;

  Print (L"\r\nWROTE magic 0x%lx\r\n", (UINT64)MAGIC_VALUE);
  Print (L"READBACK      = 0x%lx  %a\r\n", ReadBack,
         (ReadBack == MAGIC_VALUE) ? "(write landed in DRAM)" : "(WRITE FAILED)");

  Print (L"\r\nWarm reset in 8 seconds - read the screen now.\r\n");
  Print (L"After reboot: if Android boots -> magic survived.\r\n");
  Print (L"If UEFI boots -> run PStoreProbe again and read CURRENT VALUE.\r\n");

  gBS->Stall (8 * 1000 * 1000);

  gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);

  return EFI_SUCCESS;
}

/*	$OpenBSD$	*/

/******************************************************************************

  Copyright (c) 2001-2008, Intel Corporation 
  All rights reserved.
  
  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions are met:
  
   1. Redistributions of source code must retain the above copyright notice, 
      this list of conditions and the following disclaimer.
  
   2. Redistributions in binary form must reproduce the above copyright 
      notice, this list of conditions and the following disclaimer in the 
      documentation and/or other materials provided with the distribution.
  
   3. Neither the name of the Intel Corporation nor the names of its 
      contributors may be used to endorse or promote products derived from 
      this software without specific prior written permission.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/
/*$FreeBSD: src/sys/dev/ixgbe/ixgbe_osdep.h,v 1.4 2008/05/16 18:46:30 jfv Exp $*/

#ifndef _IXGBE_H_
#define _IXGBE_H_

#include "bpfilter.h"
#include "vlan.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/socket.h>
#include <sys/timeout.h>
#include <sys/workq.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/bpf.h>
#include <net/if_dl.h>
#include <net/if_media.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#if NBPFILTER > 0
#include <net/bpf.h>
#endif

#if NVLAN > 0
#include <net/if_types.h>
#include <net/if_vlan_var.h>
#endif

#include <uvm/uvm_extern.h>

#include <dev/rndvar.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/ixgbe_type.h>

#define DBG 0 
#define MSGOUT(S, A, B)     printf(S "\n", A, B)
#define DEBUGFUNC(F)        DEBUGOUT(F);
#if DBG
#define DEBUGOUT(S)         printf(S "\n")
#define DEBUGOUT1(S,A)      printf(S "\n",A)
#define DEBUGOUT2(S,A,B)    printf(S "\n",A,B)
#define DEBUGOUT3(S,A,B,C)  printf(S "\n",A,B,C)
#define DEBUGOUT7(S,A,B,C,D,E,F,G)  printf(S "\n",A,B,C,D,E,F,G)
#else
#define DEBUGOUT(S)
#define DEBUGOUT1(S,A)
#define DEBUGOUT2(S,A,B)
#define DEBUGOUT3(S,A,B,C)
#define DEBUGOUT6(S,A,B,C,D,E,F)
#define DEBUGOUT7(S,A,B,C,D,E,F,G)
#endif

#define FALSE               		0
#define TRUE                		1
#define CMD_MEM_WRT_INVALIDATE          0x0010  /* BIT_4 */
#define PCI_COMMAND_REGISTER            PCIR_COMMAND

/* Compat glue */
#define MJUMPAGESIZE	MCLBYTES
#define PCIR_BAR(_x)	(0x10 + (_x) * 4)
#define roundup2(size, unit) (((size) + (unit) - 1) & ~((unit) - 1))
#define usec_delay(x) delay(x)
#define msec_delay(x) delay(1000 * (x))

/* This is needed by the shared code */
struct ixgbe_hw; 

extern uint16_t ixgbe_read_pci_cfg(struct ixgbe_hw *, uint32_t);
#define IXGBE_READ_PCIE_WORD ixgbe_read_pci_cfg

struct ixgbe_osdep {
	bus_dma_tag_t		 os_dmat;
	bus_space_tag_t		 os_memt;
	bus_space_handle_t	 os_memh;

	bus_size_t		 os_memsize;
	bus_addr_t		 os_membase;

	void			*os_sc;
	struct pci_attach_args	*os_pa;
};

#define IXGBE_WRITE_FLUSH(a)						\
	IXGBE_READ_REG(a, IXGBE_STATUS)
#define IXGBE_READ_REG(a, reg)						\
	bus_space_read_4(((struct ixgbe_osdep *)(a)->back)->os_memt,	\
	((struct ixgbe_osdep *)(a)->back)->os_memh, reg)
#define IXGBE_WRITE_REG(a, reg, value)					\
	bus_space_write_4(((struct ixgbe_osdep *)(a)->back)->os_memt,	\
	((struct ixgbe_osdep *)(a)->back)->os_memh, reg, value)
#define IXGBE_READ_REG_ARRAY(a, reg, offset)				\
	bus_space_read_4(((struct ixgbe_osdep *)(a)->back)->os_memt,	\
	((struct ixgbe_osdep *)(a)->back)->os_memh, (reg + ((offset) << 2))))
#define IXGBE_WRITE_REG_ARRAY(a, reg, offset, value)			\
	bus_space_write_4(((struct ixgbe_osdep *)(a)->back)->os_memt,	\
	((struct ixgbe_osdep *)(a)->back)->os_memh, (reg + ((offset) << 2)), value)

int32_t ixgbe_init_ops_generic(struct ixgbe_hw *hw);
int32_t ixgbe_init_hw_generic(struct ixgbe_hw *hw);
int32_t ixgbe_start_hw_generic(struct ixgbe_hw *hw);
int32_t ixgbe_clear_hw_cntrs_generic(struct ixgbe_hw *hw);
int32_t ixgbe_read_pba_num_generic(struct ixgbe_hw *hw, uint32_t *pba_num);
int32_t ixgbe_get_mac_addr_generic(struct ixgbe_hw *hw, uint8_t *mac_addr);
int32_t ixgbe_get_bus_info_generic(struct ixgbe_hw *hw);
int32_t ixgbe_stop_adapter_generic(struct ixgbe_hw *hw);

int32_t ixgbe_led_on_generic(struct ixgbe_hw *hw, uint32_t index);
int32_t ixgbe_led_off_generic(struct ixgbe_hw *hw, uint32_t index);

int32_t ixgbe_init_eeprom_params_generic(struct ixgbe_hw *hw);
int32_t ixgbe_write_eeprom_generic(struct ixgbe_hw *hw, uint16_t offset, uint16_t data);
int32_t ixgbe_read_eeprom_generic(struct ixgbe_hw *hw, uint16_t offset, uint16_t *data);
int32_t ixgbe_read_eeprom_bit_bang_generic(struct ixgbe_hw *hw, uint16_t offset,
                                       uint16_t *data);
int32_t ixgbe_validate_eeprom_checksum_generic(struct ixgbe_hw *hw,
                                           uint16_t *checksum_val);
int32_t ixgbe_update_eeprom_checksum_generic(struct ixgbe_hw *hw);

int32_t ixgbe_set_rar_generic(struct ixgbe_hw *hw, uint32_t index, uint8_t *addr, uint32_t vmdq,
                          uint32_t enable_addr);
int32_t ixgbe_init_rx_addrs_generic(struct ixgbe_hw *hw);
int32_t ixgbe_update_mc_addr_list_generic(struct ixgbe_hw *hw, uint8_t *mc_addr_list,
                                      uint32_t mc_addr_count,
                                      ixgbe_mc_addr_itr func);
int32_t ixgbe_update_uc_addr_list_generic(struct ixgbe_hw *hw, uint8_t *addr_list,
                                      uint32_t addr_count, ixgbe_mc_addr_itr func);
int32_t ixgbe_enable_mc_generic(struct ixgbe_hw *hw);
int32_t ixgbe_disable_mc_generic(struct ixgbe_hw *hw);
int32_t ixgbe_clear_vfta_generic(struct ixgbe_hw *hw);
int32_t ixgbe_set_vfta_generic(struct ixgbe_hw *hw, uint32_t vlan,
                           uint32_t vind, int vlan_on);

int32_t ixgbe_validate_mac_addr(uint8_t *mac_addr);
int32_t ixgbe_acquire_swfw_sync(struct ixgbe_hw *hw, uint16_t mask);
void ixgbe_release_swfw_sync(struct ixgbe_hw *hw, uint16_t mask);
int32_t ixgbe_disable_pcie_master(struct ixgbe_hw *hw);

int32_t ixgbe_read_analog_reg8_generic(struct ixgbe_hw *hw, uint32_t reg, uint8_t *val);
int32_t ixgbe_write_analog_reg8_generic(struct ixgbe_hw *hw, uint32_t reg, uint8_t val);

void ixgbe_set_mta(struct ixgbe_hw *hw, uint8_t *mc_addr);
int32_t ixgbe_init_ops_82598(struct ixgbe_hw *hw);

/* PHY */
int32_t ixgbe_init_phy_ops_generic(struct ixgbe_hw *hw);
int ixgbe_validate_phy_addr(struct ixgbe_hw *hw, uint32_t phy_addr);
enum ixgbe_phy_type ixgbe_get_phy_type_from_id(uint32_t phy_id);
int32_t ixgbe_get_phy_id(struct ixgbe_hw *hw);
int32_t ixgbe_identify_phy_generic(struct ixgbe_hw *hw);
int32_t ixgbe_reset_phy_generic(struct ixgbe_hw *hw);
int32_t ixgbe_read_phy_reg_generic(struct ixgbe_hw *hw, uint32_t reg_addr,
                               uint32_t device_type, uint16_t *phy_data);
int32_t ixgbe_write_phy_reg_generic(struct ixgbe_hw *hw, uint32_t reg_addr,
                                uint32_t device_type, uint16_t phy_data);
int32_t ixgbe_setup_phy_link_generic(struct ixgbe_hw *hw);
int32_t ixgbe_setup_phy_link_speed_generic(struct ixgbe_hw *hw,
                                       ixgbe_link_speed speed,
                                       int autoneg,
                                       int autoneg_wait_to_complete);

int32_t ixgbe_check_phy_link_tnx(struct ixgbe_hw *hw,
                             ixgbe_link_speed *speed,
                             int *link_up);
int32_t ixgbe_get_phy_firmware_version_tnx(struct ixgbe_hw *hw,
                                       uint16_t *firmware_version);

int32_t ixgbe_reset_phy_nl(struct ixgbe_hw *hw);
#endif /* _IXGBE_H_ */

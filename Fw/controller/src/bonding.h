/*
 * bonding.h
 *
 */

#ifndef BONDING_H_
#define BONDING_H_

void peer_manager_init(void);
void delete_bonds(void);
void delete_disconnected_bonds(void);
void bond_management_init(void);
void bms_on_connect(uint16_t m_conn_handle);
void advertising_start(bool b);

#endif /* BONDING_H_ */

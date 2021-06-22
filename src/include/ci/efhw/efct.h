/* SPDX-License-Identifier: GPL-2.0 OR BSD-2-Clause */
/* X-SPDX-Copyright-Text: (c) Copyright 2019-2020 Xilinx, Inc. */

#ifndef CI_EFHW_EFCT_H
#define CI_EFHW_EFCT_H

extern struct efhw_func_ops efct_char_functional_units;

int efct_nic_rxq_bind(struct efhw_nic *nic, int qid,
                      const struct cpumask *mask, bool timestamp_req,
                      size_t n_hugepages);
void efct_nic_rxq_free(struct efhw_nic *nic, int qid, size_t n_hugepages);

#endif /* CI_EFHW_EFCT_H */

/* SPDX-License-Identifier: GPL-2.0 */
/* X-SPDX-Copyright-Text: (c) Solarflare Communications Inc */
#include <ci/efrm/efrm_client.h>
#include "efch.h"
#include <ci/efrm/vi_resource_manager.h>
#include <ci/efrm/efct_rxq.h>
#include <ci/efch/op_types.h>
#include <ci/efhw/mc_driver_pcol.h>
#include "char_internal.h"




/* ************************************************************************ */
/*                            ioctl interface                               */

/* X-SPDX-Source-URL: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git */
/* X-SPDX-Source-Tag: v5.12 */
/* X-SPDX-Source-File: kernel/sched/core.c */
/* X-SPDX-License-Identifier: GPL-2.0-or-later */
static int get_user_cpu_mask(unsigned long __user *user_mask_ptr, unsigned len,
                             struct cpumask *new_mask)
{
  if (len < cpumask_size())
    cpumask_clear(new_mask);
  else if (len > cpumask_size())
    len = cpumask_size();

  return copy_from_user(new_mask, user_mask_ptr, len) ? -EFAULT : 0;
}
/* X-SPDX-Restore: */


static int
rxq_rm_alloc(ci_resource_alloc_t* alloc_, ci_resource_table_t* priv_opt,
             efch_resource_t* rs, int intf_ver_id)
{
  struct efch_efct_rxq_alloc* alloc = &alloc_->u.rxq;
  struct efrm_efct_rxq* rxq;
  struct efrm_pd* pd;
  efch_resource_t* vi_rs;
  cpumask_var_t cpumask;
  int rc;

  rc = efch_resource_id_lookup(alloc->in_vi_rs_id, priv_opt, &vi_rs);
  if (rc < 0) {
    EFCH_ERR("%s: ERROR: id="EFCH_RESOURCE_ID_FMT" (%d)",
             __FUNCTION__,
             EFCH_RESOURCE_ID_PRI_ARG(alloc->in_vi_rs_id), rc);
    return rc;
  }
  if (vi_rs->rs_base->rs_type != EFRM_RESOURCE_VI) {
    EFCH_ERR("%s: ERROR: id="EFCH_RESOURCE_ID_FMT" is not a VI",
             __FUNCTION__,
             EFCH_RESOURCE_ID_PRI_ARG(alloc->in_vi_rs_id));
    return -EINVAL;
  }

  pd = efrm_vi_from_resource(vi_rs->rs_base)->pd;

  if (!alloc_cpumask_var(&cpumask, GFP_KERNEL))
    return -ENOMEM;

  if (alloc->in_cpusetsize == 0) {
    cpumask_setall(cpumask);
  }
  else {
    rc = get_user_cpu_mask((void __user*)alloc->in_cpuset,
                           alloc->in_cpusetsize, cpumask);
    if (rc)
      return rc;
  }

  rc = efrm_rxq_alloc(pd, alloc->in_qid, cpumask, alloc->in_timestamp_req,
                      alloc->in_n_hugepages, &rxq);
  free_cpumask_var(cpumask);
  if (rc < 0) {
    EFCH_ERR("%s: ERROR: rxq_alloc failed (%d)", __FUNCTION__, rc);
    return rc;
  }

  rs->rs_base = efrm_rxq_to_resource(rxq);
  return 0;
}


static void
rxq_rm_free(efch_resource_t* rs)
{
  efrm_rxq_release(efrm_rxq_from_resource(rs->rs_base));
}


static int
rxq_rm_rsops(efch_resource_t* rs, ci_resource_table_t* priv_opt,
             ci_resource_op_t* op, int* copy_out)
{
  switch (op->op) {
  default:
    EFCH_ERR("%s: Invalid op, expected CI_RSOP_RXQ_*", __FUNCTION__);
    return -EINVAL;
  }
}


efch_resource_ops efch_efct_rxq_ops = {
  .rm_alloc  = rxq_rm_alloc,
  .rm_free   = rxq_rm_free,
  .rm_mmap   = NULL,
  .rm_nopage = NULL,
  .rm_dump   = NULL,
  .rm_rsops  = rxq_rm_rsops,
};

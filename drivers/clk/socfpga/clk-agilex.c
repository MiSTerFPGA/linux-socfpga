// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019-2020, Intel Corporation
 */
#include <linux/slab.h>
#include <linux/clk-provider.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include <dt-bindings/clock/agilex-clock.h>

#include "stratix10-clk.h"

static const char * const pll_mux[] = { "osc1", "cb-intosc-hs-div2-clk",
					"f2s-free-clk",};
static const char * const cntr_mux[] = { "main_pll", "periph_pll",
					 "osc1", "cb-intosc-hs-div2-clk",
					 "f2s-free-clk"};
static const char * const boot_mux[] = { "osc1", "cb-intosc-hs-div2-clk",};

static const char * const mpu_free_mux[] = {"main_pll_c0", "peri_pll_c0",
					    "osc1", "cb-intosc-hs-div2-clk",
					    "f2s-free-clk"};

static const char * const noc_free_mux[] = {"main_pll_c1", "peri_pll_c1",
					    "osc1", "cb-intosc-hs-div2-clk",
					    "f2s-free-clk"};

static const char * const emaca_free_mux[] = {"main_pll_c2", "peri_pll_c2",
					      "osc1", "cb-intosc-hs-div2-clk",
					      "f2s-free-clk"};
static const char * const emacb_free_mux[] = {"main_pll_c3", "peri_pll_c3",
					      "osc1", "cb-intosc-hs-div2-clk",
					      "f2s-free-clk"};
static const char * const emac_ptp_free_mux[] = {"main_pll_c3", "peri_pll_c3",
					      "osc1", "cb-intosc-hs-div2-clk",
					      "f2s-free-clk"};
static const char * const gpio_db_free_mux[] = {"main_pll_c3", "peri_pll_c3",
					      "osc1", "cb-intosc-hs-div2-clk",
					      "f2s-free-clk"};
static const char * const psi_ref_free_mux[] = {"main_pll_c2", "peri_pll_c2",
					      "osc1", "cb-intosc-hs-div2-clk",
					      "f2s-free-clk"};
static const char * const sdmmc_free_mux[] = {"main_pll_c3", "peri_pll_c3",
					      "osc1", "cb-intosc-hs-div2-clk",
					      "f2s-free-clk"};
static const char * const s2f_usr0_free_mux[] = {"main_pll_c2", "peri_pll_c2",
						 "osc1", "cb-intosc-hs-div2-clk",
						 "f2s-free-clk"};
static const char * const s2f_usr1_free_mux[] = {"main_pll_c2", "peri_pll_c2",
						 "osc1", "cb-intosc-hs-div2-clk",
						 "f2s-free-clk"};
static const char * const mpu_mux[] = { "mpu_free_clk", "boot_clk",};

static const char * const s2f_usr0_mux[] = {"f2s-free-clk", "boot_clk"};
static const char * const emac_mux[] = {"emaca_free_clk", "emacb_free_clk",
					"boot_clk"};
static const char * const noc_mux[] = {"noc_free_clk", "boot_clk"};
static const char * const sdmmc_mux[] = {"sdmmc_free_clk", "boot_clk"};
static const char * const s2f_user0_mux[] = {"s2f_user0_free_clk", "boot_clk"};
static const char * const s2f_user1_mux[] = {"s2f_user1_free_clk", "boot_clk"};
static const char * const psi_mux[] = {"psi_ref_free_clk", "boot_clk"};
static const char * const gpio_db_mux[] = {"gpio_db_free_clk", "boot_clk"};
static const char * const emac_ptp_mux[] = {"emac_ptp_free_clk", "boot_clk"};

/* clocks in AO (always on) controller */
static const struct stratix10_pll_clock agilex_pll_clks[] = {
	{ AGILEX_BOOT_CLK, "boot_clk", boot_mux, ARRAY_SIZE(boot_mux), 0,
	  0x0},
	{ AGILEX_MAIN_PLL_CLK, "main_pll", pll_mux, ARRAY_SIZE(pll_mux),
	  0, 0x48},
	{ AGILEX_PERIPH_PLL_CLK, "periph_pll", pll_mux, ARRAY_SIZE(pll_mux),
	  0, 0x9c},
};

static const struct n5x_perip_c_clock n5x_main_perip_c_clks[] = {
	{ AGILEX_MAIN_PLL_C0_CLK, "main_pll_c0", "main_pll", NULL, 1, 0, 0x54, 0},
	{ AGILEX_MAIN_PLL_C1_CLK, "main_pll_c1", "main_pll", NULL, 1, 0, 0x54, 8},
	{ AGILEX_MAIN_PLL_C2_CLK, "main_pll_c2", "main_pll", NULL, 1, 0, 0x54, 16},
	{ AGILEX_MAIN_PLL_C3_CLK, "main_pll_c3", "main_pll", NULL, 1, 0, 0x54, 24},
	{ AGILEX_PERIPH_PLL_C0_CLK, "peri_pll_c0", "periph_pll", NULL, 1, 0, 0xA8, 0},
	{ AGILEX_PERIPH_PLL_C1_CLK, "peri_pll_c1", "periph_pll", NULL, 1, 0, 0xA8, 8},
	{ AGILEX_PERIPH_PLL_C2_CLK, "peri_pll_c2", "periph_pll", NULL, 1, 0, 0xA8, 16},
	{ AGILEX_PERIPH_PLL_C3_CLK, "peri_pll_c3", "periph_pll", NULL, 1, 0, 0xA8, 24},
};

static const struct stratix10_perip_c_clock agilex_main_perip_c_clks[] = {
	{ AGILEX_MAIN_PLL_C0_CLK, "main_pll_c0", "main_pll", NULL, 1, 0, 0x58},
	{ AGILEX_MAIN_PLL_C1_CLK, "main_pll_c1", "main_pll", NULL, 1, 0, 0x5C},
	{ AGILEX_MAIN_PLL_C2_CLK, "main_pll_c2", "main_pll", NULL, 1, 0, 0x64},
	{ AGILEX_MAIN_PLL_C3_CLK, "main_pll_c3", "main_pll", NULL, 1, 0, 0x68},
	{ AGILEX_PERIPH_PLL_C0_CLK, "peri_pll_c0", "periph_pll", NULL, 1, 0, 0xAC},
	{ AGILEX_PERIPH_PLL_C1_CLK, "peri_pll_c1", "periph_pll", NULL, 1, 0, 0xB0},
	{ AGILEX_PERIPH_PLL_C2_CLK, "peri_pll_c2", "periph_pll", NULL, 1, 0, 0xB8},
	{ AGILEX_PERIPH_PLL_C3_CLK, "peri_pll_c3", "periph_pll", NULL, 1, 0, 0xBC},
};

static const struct stratix10_perip_cnt_clock agilex_main_perip_cnt_clks[] = {
	{ AGILEX_MPU_FREE_CLK, "mpu_free_clk", NULL, mpu_free_mux, ARRAY_SIZE(mpu_free_mux),
	   0, 0x3C, 0, 0, 0},
	{ AGILEX_NOC_FREE_CLK, "noc_free_clk", NULL, noc_free_mux, ARRAY_SIZE(noc_free_mux),
	  0, 0x40, 0, 0, 0},
	{ AGILEX_L4_SYS_FREE_CLK, "l4_sys_free_clk", NULL, noc_mux, ARRAY_SIZE(noc_mux), 0,
	  0, 4, 0x30, 1},
	{ AGILEX_EMAC_A_FREE_CLK, "emaca_free_clk", NULL, emaca_free_mux, ARRAY_SIZE(emaca_free_mux),
	  0, 0xD4, 0, 0x88, 0},
	{ AGILEX_EMAC_B_FREE_CLK, "emacb_free_clk", NULL, emacb_free_mux, ARRAY_SIZE(emacb_free_mux),
	  0, 0xD8, 0, 0x88, 1},
	{ AGILEX_EMAC_PTP_FREE_CLK, "emac_ptp_free_clk", NULL, emac_ptp_free_mux,
	  ARRAY_SIZE(emac_ptp_free_mux), 0, 0xDC, 0, 0x88, 2},
	{ AGILEX_GPIO_DB_FREE_CLK, "gpio_db_free_clk", NULL, gpio_db_free_mux,
	  ARRAY_SIZE(gpio_db_free_mux), 0, 0xE0, 0, 0x88, 3},
	{ AGILEX_SDMMC_FREE_CLK, "sdmmc_free_clk", NULL, sdmmc_free_mux,
	  ARRAY_SIZE(sdmmc_free_mux), 0, 0xE4, 0, 0, 0},
	{ AGILEX_S2F_USER0_FREE_CLK, "s2f_user0_free_clk", NULL, s2f_usr0_free_mux,
	  ARRAY_SIZE(s2f_usr0_free_mux), 0, 0xE8, 0, 0x30, 2},
	{ AGILEX_S2F_USER1_FREE_CLK, "s2f_user1_free_clk", NULL, s2f_usr1_free_mux,
	  ARRAY_SIZE(s2f_usr1_free_mux), 0, 0xEC, 0, 0x88, 5},
	{ AGILEX_PSI_REF_FREE_CLK, "psi_ref_free_clk", NULL, psi_ref_free_mux,
	  ARRAY_SIZE(psi_ref_free_mux), 0, 0xF0, 0, 0x88, 6},
};

static const struct stratix10_gate_clock agilex_gate_clks[] = {
	{ AGILEX_MPU_CLK, "mpu_clk", NULL, mpu_mux, ARRAY_SIZE(mpu_mux), 0, 0x24,
	  0, 0, 0, 0, 0x30, 0, 0},
	{ AGILEX_MPU_PERIPH_CLK, "mpu_periph_clk", "mpu_clk", NULL, 1, 0, 0x24,
	  0, 0, 0, 0, 0, 0, 4},
	{ AGILEX_MPU_CCU_CLK, "mpu_ccu_clk", "mpu_clk", NULL, 1, 0, 0x24,
	  0, 0, 0, 0, 0, 0, 2},
	{ AGILEX_L4_MAIN_CLK, "l4_main_clk", NULL, noc_mux, ARRAY_SIZE(noc_mux), 0, 0x24,
	  1, 0x44, 0, 2, 0x30, 1, 0},
	{ AGILEX_L4_MP_CLK, "l4_mp_clk", NULL, noc_mux, ARRAY_SIZE(noc_mux), 0, 0x24,
	  2, 0x44, 8, 2, 0x30, 1, 0},
	/*
	 * The l4_sp_clk feeds a 100 MHz clock to various peripherals, one of them
	 * being the SP timers, thus cannot get gated.
	 */
	{ AGILEX_L4_SP_CLK, "l4_sp_clk", NULL, noc_mux, ARRAY_SIZE(noc_mux), CLK_IS_CRITICAL, 0x24,
	  3, 0x44, 16, 2, 0x30, 1, 0},
	{ AGILEX_CS_AT_CLK, "cs_at_clk", NULL, noc_mux, ARRAY_SIZE(noc_mux), 0, 0x24,
	  4, 0x44, 24, 2, 0x30, 1, 0},
	{ AGILEX_CS_TRACE_CLK, "cs_trace_clk", NULL, noc_mux, ARRAY_SIZE(noc_mux), 0, 0x24,
	  4, 0x44, 26, 2, 0x30, 1, 0},
	{ AGILEX_CS_PDBG_CLK, "cs_pdbg_clk", "cs_at_clk", NULL, 1, 0, 0x24,
	  4, 0x44, 28, 1, 0, 0, 0},
	{ AGILEX_CS_TIMER_CLK, "cs_timer_clk", NULL, noc_mux, ARRAY_SIZE(noc_mux), 0, 0x24,
	  5, 0, 0, 0, 0x30, 1, 0},
	{ AGILEX_S2F_USER0_CLK, "s2f_user0_clk", NULL, s2f_usr0_mux, ARRAY_SIZE(s2f_usr0_mux), 0, 0x24,
	  6, 0, 0, 0, 0, 0, 0},
	{ AGILEX_EMAC0_CLK, "emac0_clk", NULL, emac_mux, ARRAY_SIZE(emac_mux), 0, 0x7C,
	  0, 0, 0, 0, 0x94, 26, 0},
	{ AGILEX_EMAC1_CLK, "emac1_clk", NULL, emac_mux, ARRAY_SIZE(emac_mux), 0, 0x7C,
	  1, 0, 0, 0, 0x94, 27, 0},
	{ AGILEX_EMAC2_CLK, "emac2_clk", NULL, emac_mux, ARRAY_SIZE(emac_mux), 0, 0x7C,
	  2, 0, 0, 0, 0x94, 28, 0},
	{ AGILEX_EMAC_PTP_CLK, "emac_ptp_clk", NULL, emac_ptp_mux, ARRAY_SIZE(emac_ptp_mux), 0, 0x7C,
	  3, 0, 0, 0, 0x88, 2, 0},
	{ AGILEX_GPIO_DB_CLK, "gpio_db_clk", NULL, gpio_db_mux, ARRAY_SIZE(gpio_db_mux), 0, 0x7C,
	  4, 0x98, 0, 16, 0x88, 3, 0},
	{ AGILEX_SDMMC_CLK, "sdmmc_clk", NULL, sdmmc_mux, ARRAY_SIZE(sdmmc_mux), 0, 0x7C,
	  5, 0, 0, 0, 0x88, 4, 4},
	{ AGILEX_S2F_USER0_CLK, "s2f_user1_clk", NULL, s2f_user0_mux, ARRAY_SIZE(s2f_user0_mux), 0, 0x24,
	  6, 0, 0, 0, 0x30, 2, 0},
	{ AGILEX_S2F_USER1_CLK, "s2f_user1_clk", NULL, s2f_user1_mux, ARRAY_SIZE(s2f_user1_mux), 0, 0x7C,
	  6, 0, 0, 0, 0x88, 5, 0},
	{ AGILEX_PSI_REF_CLK, "psi_ref_clk", NULL, psi_mux, ARRAY_SIZE(psi_mux), 0, 0x7C,
	  7, 0, 0, 0, 0x88, 6, 0},
	{ AGILEX_USB_CLK, "usb_clk", "l4_mp_clk", NULL, 1, 0, 0x7C,
	  8, 0, 0, 0, 0, 0, 0},
	{ AGILEX_SPI_M_CLK, "spi_m_clk", "l4_mp_clk", NULL, 1, 0, 0x7C,
	  9, 0, 0, 0, 0, 0, 0},
	{ AGILEX_NAND_X_CLK, "nand_x_clk", "l4_mp_clk", NULL, 1, 0, 0x7C,
	  10, 0, 0, 0, 0, 0, 0},
	{ AGILEX_NAND_CLK, "nand_clk", "nand_x_clk", NULL, 1, 0, 0x7C,
	  10, 0, 0, 0, 0, 0, 4},
	{ AGILEX_NAND_ECC_CLK, "nand_ecc_clk", "nand_x_clk", NULL, 1, 0, 0x7C,
	  10, 0, 0, 0, 0, 0, 4},
};

static int n5x_clk_register_c_perip(const struct n5x_perip_c_clock *clks,
				       int nums, struct stratix10_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		clk = n5x_register_periph(clks[i].name, clks[i].parent_name,
					  clks[i].parent_names, clks[i].num_parents,
					  clks[i].flags, base, clks[i].offset,
					  clks[i].shift);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}
		data->clk_data.clks[clks[i].id] = clk;
	}
	return 0;
}

static int agilex_clk_register_c_perip(const struct stratix10_perip_c_clock *clks,
				       int nums, struct stratix10_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		clk = s10_register_periph(clks[i].name, clks[i].parent_name,
					  clks[i].parent_names, clks[i].num_parents,
					  clks[i].flags, base, clks[i].offset);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}
		data->clk_data.clks[clks[i].id] = clk;
	}
	return 0;
}

static int agilex_clk_register_cnt_perip(const struct stratix10_perip_cnt_clock *clks,
					 int nums, struct stratix10_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		clk = s10_register_cnt_periph(clks[i].name, clks[i].parent_name,
					      clks[i].parent_names,
					      clks[i].num_parents,
					      clks[i].flags, base,
					      clks[i].offset,
					      clks[i].fixed_divider,
					      clks[i].bypass_reg,
					      clks[i].bypass_shift);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}
		data->clk_data.clks[clks[i].id] = clk;
	}

	return 0;
}

static int agilex_clk_register_gate(const struct stratix10_gate_clock *clks,					    int nums, struct stratix10_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		clk = agilex_register_gate(clks[i].name, clks[i].parent_name,
					clks[i].parent_names,
					clks[i].num_parents,
					clks[i].flags, base,
					clks[i].gate_reg,
					clks[i].gate_idx, clks[i].div_reg,
					clks[i].div_offset, clks[i].div_width,
					clks[i].bypass_reg,
					clks[i].bypass_shift,
					clks[i].fixed_div);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}
		data->clk_data.clks[clks[i].id] = clk;
	}

	return 0;
}

static int agilex_clk_register_pll(const struct stratix10_pll_clock *clks,
				 int nums, struct stratix10_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		clk = agilex_register_pll(clks[i].name, clks[i].parent_names,
				    clks[i].num_parents,
				    clks[i].flags, base,
				    clks[i].offset);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}
		data->clk_data.clks[clks[i].id] = clk;
	}

	return 0;
}

static int n5x_clk_register_pll(const struct stratix10_pll_clock *clks,
				 int nums, struct stratix10_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		clk = n5x_register_pll(clks[i].name, clks[i].parent_names,
				    clks[i].num_parents,
				    clks[i].flags, base,
				    clks[i].offset);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}
		data->clk_data.clks[clks[i].id] = clk;
	}

	return 0;
}

static struct stratix10_clock_data *__socfpga_agilex_clk_init(struct platform_device *pdev,
						    int nr_clks)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct stratix10_clock_data *clk_data;
	struct clk **clk_table;
	struct resource *res;
	void __iomem *base;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(dev, res);
	if (IS_ERR(base)) {
		pr_err("%s: failed to map clock registers\n", __func__);
		return ERR_CAST(base);
	}

	clk_data = devm_kzalloc(dev, sizeof(*clk_data), GFP_KERNEL);
	if (!clk_data)
		return ERR_PTR(-ENOMEM);

	clk_data->base = base;
	clk_table = devm_kcalloc(dev, nr_clks, sizeof(*clk_table), GFP_KERNEL);
	if (!clk_table)
		return ERR_PTR(-ENOMEM);

	clk_data->clk_data.clks = clk_table;
	clk_data->clk_data.clk_num = nr_clks;
	ret = of_clk_add_provider(np, of_clk_src_onecell_get, &clk_data->clk_data);
	if (ret)
		return ERR_PTR(ret);

	return clk_data;
}

static int agilex_clkmgr_init(struct platform_device *pdev)
{
	struct stratix10_clock_data *clk_data;

	clk_data = __socfpga_agilex_clk_init(pdev, AGILEX_NUM_CLKS);
	if (IS_ERR(clk_data))
		return PTR_ERR(clk_data);

	agilex_clk_register_pll(agilex_pll_clks, ARRAY_SIZE(agilex_pll_clks), clk_data);

	agilex_clk_register_c_perip(agilex_main_perip_c_clks,
				 ARRAY_SIZE(agilex_main_perip_c_clks), clk_data);

	agilex_clk_register_cnt_perip(agilex_main_perip_cnt_clks,
				   ARRAY_SIZE(agilex_main_perip_cnt_clks),
				   clk_data);

	agilex_clk_register_gate(agilex_gate_clks, ARRAY_SIZE(agilex_gate_clks),
			      clk_data);
	return 0;
}

static int n5x_clkmgr_init(struct platform_device *pdev)
{
	struct stratix10_clock_data *clk_data;

	clk_data = __socfpga_agilex_clk_init(pdev, AGILEX_NUM_CLKS);
	if (IS_ERR(clk_data))
		return PTR_ERR(clk_data);

	n5x_clk_register_pll(agilex_pll_clks, ARRAY_SIZE(agilex_pll_clks), clk_data);

	n5x_clk_register_c_perip(n5x_main_perip_c_clks,
				 ARRAY_SIZE(n5x_main_perip_c_clks), clk_data);

	agilex_clk_register_cnt_perip(agilex_main_perip_cnt_clks,
				   ARRAY_SIZE(agilex_main_perip_cnt_clks),
				   clk_data);

	agilex_clk_register_gate(agilex_gate_clks, ARRAY_SIZE(agilex_gate_clks),
			      clk_data);
	return 0;
}

static int agilex_clkmgr_probe(struct platform_device *pdev)
{
	int (*probe_func)(struct platform_device *);

	probe_func = of_device_get_match_data(&pdev->dev);
	if (!probe_func)
		return -ENODEV;
	return	probe_func(pdev);;
}

static const struct of_device_id agilex_clkmgr_match_table[] = {
	{ .compatible = "intel,agilex-clkmgr",
	  .data = agilex_clkmgr_init },
	{ .compatible = "intel,easic-n5x-clkmgr",
	  .data = n5x_clkmgr_init },
	{ }
};

static struct platform_driver agilex_clkmgr_driver = {
	.probe		= agilex_clkmgr_probe,
	.driver		= {
		.name	= "agilex-clkmgr",
		.suppress_bind_attrs = true,
		.of_match_table = agilex_clkmgr_match_table,
	},
};

static int __init agilex_clk_init(void)
{
	return platform_driver_register(&agilex_clkmgr_driver);
}
core_initcall(agilex_clk_init);

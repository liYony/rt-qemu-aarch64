/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-12-20     GuEe-GUI     first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <gtimer.h>
#include <cpuport.h>
#include <drivers/platform.h>
#include <drivers/pic.h>
#include <drivers/ofw_irq.h>
#ifdef RT_USING_KTIME
#include <ktime.h>
#endif

#define EL1_PHY_TIMER_IRQ_NUM 30

static int irq_gtimer = EL1_PHY_TIMER_IRQ_NUM;
static volatile rt_uint64_t timer_step;

static void rt_hw_timer_isr(int vector, void *parameter)
{
    rt_hw_set_gtimer_val(timer_step);
    rt_tick_increase();
}

void rt_hw_gtimer_init(void)
{
    rt_hw_interrupt_install(irq_gtimer, rt_hw_timer_isr, RT_NULL, "tick");
    rt_hw_isb();
    timer_step = rt_hw_get_gtimer_frq();
    rt_hw_dsb();
    timer_step /= RT_TICK_PER_SECOND;
    rt_hw_gtimer_local_enable();
}

void rt_hw_gtimer_local_enable(void)
{
    rt_hw_gtimer_disable();
    rt_hw_set_gtimer_val(timer_step);
    rt_hw_interrupt_umask(irq_gtimer);
#ifdef RT_USING_KTIME
    rt_ktime_cputimer_init();
#endif
    rt_hw_gtimer_enable();
}

void rt_hw_gtimer_local_disable(void)
{
    rt_hw_gtimer_disable();
    rt_hw_interrupt_mask(irq_gtimer);
}

static rt_err_t gtimer_probe(struct rt_platform_device *pdev)
{
    irq_gtimer = rt_ofw_get_irq(pdev->parent.ofw_node, 1);
    rt_hw_gtimer_init();

    return RT_EOK;
}

static const struct rt_ofw_node_id gtimer_ofw_ids[] =
{
    { .compatible = "arm,armv8-timer" },
    { /* sentinel */ }
};

static struct rt_platform_driver gtimer_driver =
{
    .name = "arm-armv8-timer",
    .ids = gtimer_ofw_ids,

    .probe = gtimer_probe,
};

static int gtimer_drv_register(void)
{
    rt_platform_driver_register(&gtimer_driver);

    return 0;
}
INIT_FRAMEWORK_EXPORT(gtimer_drv_register);
